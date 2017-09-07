#include "EcuHacks.h"

// To test, load a patched ROM and set PC to 00011696.
// You should JSR to the code that looks up throttle plate angle from tables,
// then return to stock ECU code, then step into RevMatchCode(), then step 
// through stock ECU code that applies a low-pass filter to the throttle 
// plate angle.

extern float Gear1Multiplier;
extern float Gear2Multiplier;
extern float Gear3Multiplier;
extern float Gear4Multiplier;
extern float Gear5Multiplier;
extern float Gear6Multiplier;
extern float MinTargetRpm;
extern float MaxTargetRpm;
extern float MinCoolantTemperature;

extern int RevMatchDuration;
extern int RevMatchAccelerationDownshiftReadyDuration;
extern int RevMatchEnableDelay;
extern int RevMatchCalibrationDelay;

extern TwoDimensionalTable RevMatchTable;

float Window(float rpm) __attribute__ ((section ("RomHole_RevMatchCode")));
float Window(float rpm)
{
	if (rpm < MinTargetRpm)
	{
		return MinTargetRpm;
	}
	
	if (rpm > MaxTargetRpm)
	{
		return MaxTargetRpm;
	}
	
	return rpm;
}

void SetTargetRpm() __attribute__ ((section ("RomHole_RevMatchCode")));
void SetTargetRpm()
{
	float upshift = 0;
	float downshift = 0;
	
	switch(*pCurrentGear)
	{
		case 1:
			upshift = Gear2Multiplier;
			downshift = Gear1Multiplier;
			break;
			
		case 2:
			upshift = Gear3Multiplier;
			downshift = Gear1Multiplier;
			break;
			
		case 3:
			upshift = Gear4Multiplier;
			downshift = Gear2Multiplier;
			break;
			
		case 4:
			upshift = Gear5Multiplier;
			downshift = Gear3Multiplier;
			break;
			
		case 5:
			upshift = Gear6Multiplier;
			downshift = Gear4Multiplier;
			break;
			
		case 6:
			upshift = Gear6Multiplier;
			downshift = Gear5Multiplier;
			break;
			
		default:
			upshift = Gear6Multiplier;
			downshift = Gear6Multiplier;
			break;
	}
	
	pRamVariables->UpshiftRpm = Window((*pVehicleSpeed * 1000.0f) / upshift);
	pRamVariables->DownshiftRpm = Window((*pVehicleSpeed * 1000.0f) / downshift);
}

void UpdateCounter() __attribute__ ((section ("RomHole_RevMatchCode")));
void UpdateCounter()
{
	if (pRamVariables->Counter < MAX_COUNTER)
	{
		pRamVariables->Counter = pRamVariables->Counter + 1;
	}
	else
	{
		pRamVariables->Counter = 0;
	}
}

unsigned int GetElapsed() __attribute__ ((section ("RomHole_RevMatchCode")));
unsigned int GetElapsed(int start)
{
	if (pRamVariables->Counter >= start)
	{
		return pRamVariables->Counter - start;
	}
	else
	{
		unsigned int initial = (MAX_COUNTER + 1) - start;
		return initial + pRamVariables->Counter;
	}
}

int IntervalElapsed(int elapsed, int duration) __attribute__ ((section ("RomHole_RevMatchCode")));
int IntervalElapsed(int elapsed, int duration)
{
	int threshold = duration * 100;
	if (elapsed > threshold)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int ConditionWithDelay(int condition, int duration) __attribute__ ((section ("RomHole_RevMatchCode")));
int ConditionWithDelay(int condition, int duration)
{
	if (condition)
	{
		// Run the condition timer.
		if (pRamVariables->RevMatchConditionStart == 0)
		{
			pRamVariables->RevMatchConditionStart = pRamVariables->Counter;
		}

		int elapsed = GetElapsed(pRamVariables->RevMatchConditionStart);
		return IntervalElapsed(elapsed, duration);
	}
	
	// Reset condition timer
	pRamVariables->RevMatchConditionStart = 0;
	
	return 0;
}

int StateTimeout(int duration) __attribute__ ((section ("RomHole_RevMatchCode")));
int StateTimeout(int duration)
{
	// Run the timer.
	if (pRamVariables->RevMatchTimeoutStart == 0)
	{
		pRamVariables->RevMatchTimeoutStart = pRamVariables->Counter;
	}

	int elapsed = GetElapsed(pRamVariables->RevMatchTimeoutStart);
	return IntervalElapsed(elapsed, duration);
}

// returning zero means 'no change'
// which implies that there is no way for a user to go from enabled to disabled, other than stopping the engine.
enum RevMatchStates EvaluateTransitionDisabled() __attribute__ ((section ("RomHole_RevMatchCode")));
enum RevMatchStates EvaluateTransitionDisabled()
{
	int condition = *pCruiseFlagsA & CruiseFlagsACancel;
	if (ConditionWithDelay(condition, 10))
	{
		return RevMatchAlmostEnabled;
	}
	
	return 0;	
}

enum RevMatchStates EvaluateTransitionAlmostEnabled() __attribute__ ((section ("RomHole_RevMatchCode")));
enum RevMatchStates EvaluateTransitionAlmostEnabled()
{
	if ((*pCruiseFlagsA & CruiseFlagsACancel) == 0)
	{
		return RevMatchEnabled;
	}
	
	return 0;
}

enum RevMatchStates EvaluateTransitionEnabled() __attribute__ ((section ("RomHole_RevMatchCode")));
enum RevMatchStates EvaluateTransitionEnabled()
{
	if ((*pCruiseFlagsA & CruiseFlagsALightBrake) &&
		(*pCruiseFlagsA & CruiseFlagsAClutch) &&
		!(*pCruiseFlagsA & CruiseFlagsACancel))
	{
		return RevMatchDecelerationDownshift;
	}
	
	if ((*pCruiseFlagsA & CruiseFlagsACancel) &&
		!(*pCruiseFlagsA & CruiseFlagsALightBrake))
	{
		return RevMatchReadyForAccelerationDownshift;
	}
	
	int condition = 
		((*pCruiseFlagsA & CruiseFlagsACancel) &&
		(*pCruiseFlagsA & CruiseFlagsAClutch) &&
		(*pSpeed < 1));
	
	if (ConditionWithDelay(condition, 50))
	{
		return RevMatchCalibration;
	}
	
	return 0;
}

enum RevMatchStates EvaluateTransitionReadyForAccelerationDownshift() __attribute__ ((section ("RomHole_RevMatchCode")));
enum RevMatchStates EvaluateTransitionReadyForAccelerationDownshift()
{
	if (*pCruiseFlagsA & CruiseFlagsAClutch)
	{
		return RevMatchAccelerationDownshift;
	}
	
	int cancelReleased = !(*pCruiseFlagsA & CruiseFlagsACancel);
	if (cancelReleased && StateTimeout(10))
	{
		return RevMatchEnabled;
	}
	
	return 0;
}


enum RevMatchStates EvaluateTransitionDecelerationDownshift() __attribute__ ((section ("RomHole_RevMatchCode")));
enum RevMatchStates EvaluateTransitionDecelerationDownshift()
{
	if (StateTimeout(10))
	{
		return RevMatchExpired;
	}
	
	if (!(*pCruiseFlagsA & CruiseFlagsALightBrake))
	{
		return RevMatchEnabled;
	}

	if (!(*pCruiseFlagsA & CruiseFlagsAClutch))
	{
		return RevMatchEnabled;
	}
	
	return 0;	
}

enum RevMatchStates EvaluateTransitionAccelerationDownshift() __attribute__ ((section ("RomHole_RevMatchCode")));
enum RevMatchStates EvaluateTransitionAccelerationDownshift()
{
	if (StateTimeout(10))
	{
		return RevMatchExpired;
	}
	
	if (!(*pCruiseFlagsA & CruiseFlagsAClutch))
	{
		return RevMatchEnabled;
	}
	
	return 0;	
}


enum RevMatchStates EvaluateTransitionCalibration() __attribute__ ((section ("RomHole_RevMatchCode")));
enum RevMatchStates EvaluateTransitionCalibration()
{
	if (!(*pCruiseFlagsA & CruiseFlagsALightBrake))
	{
		return RevMatchEnabled;
	}

	if (!(*pCruiseFlagsA & CruiseFlagsAClutch))
	{
		return RevMatchEnabled;
	}
	
	if (*pSpeed > 1)
	{
		return RevMatchEnabled;
	}
	
	return 0;
}

enum RevMatchStates EvaluateTransitionExpired() __attribute__ ((section ("RomHole_RevMatchCode")));
enum RevMatchStates EvaluateTransitionExpired()
{
	if (!(*pCruiseFlagsA & CruiseFlagsAClutch))
	{
		return RevMatchEnabled;
	}
	
	return 0;	
}

void UpdateState() __attribute__ ((section ("RomHole_RevMatchCode")));
void UpdateState()
{
	// Reset when shutting down or starting up.
	if (*pRPM < 500)
	{
		pRamVariables->RevMatchState = RevMatchDisabled;
		pRamVariables->Counter = 0;
		pRamVariables->RevMatchConditionStart = 0;
		pRamVariables->RevMatchTimeoutStart = 0;
		pRamVariables->RevMatchTransitionEvaluator = EvaluateTransitionDisabled;
		return;
	}

	RamVariables *pRV = pRamVariables;
	
	if (pRamVariables->RevMatchTransitionEvaluator == 0)
	{
		// Should never happen - set breakpoint here.
		return;
	}
	
	enum RevMatchStates nextState = (*(pRamVariables->RevMatchTransitionEvaluator))();
	if (nextState == 0)
	{
		return;
	}
	
	pRamVariables->RevMatchState = nextState;
	pRamVariables->RevMatchTimeoutStart = 0;
	pRamVariables->RevMatchConditionStart = 0;
	
	switch(nextState)
	{
		case RevMatchAlmostEnabled:
			pRamVariables->RevMatchTransitionEvaluator = EvaluateTransitionAlmostEnabled;
			break;

		case RevMatchEnabled:
			pRamVariables->RevMatchTransitionEvaluator = EvaluateTransitionEnabled;
			break;
			
		case RevMatchReadyForAccelerationDownshift:
			pRamVariables->RevMatchTransitionEvaluator = EvaluateTransitionReadyForAccelerationDownshift;
			break;
			
		case RevMatchDecelerationDownshift:
			pRamVariables->RevMatchTransitionEvaluator = EvaluateTransitionDecelerationDownshift;
			break;

		case RevMatchAccelerationDownshift:
			pRamVariables->RevMatchTransitionEvaluator = EvaluateTransitionAccelerationDownshift;
			break;
			
		case RevMatchCalibration:
			pRamVariables->RevMatchTransitionEvaluator = EvaluateTransitionCalibration;
			pRamVariables->RevMatchCalibrationIndex = 0;
			break;
			
		case RevMatchExpired:
			pRamVariables->RevMatchTransitionEvaluator = EvaluateTransitionExpired;
			break;
	}
}

void RevMatchCode(void) __attribute__ ((section ("RomHole_RevMatchCode")));
void RevMatchCode()
{
	// These nopcodes are just here to make it easier to distinguish between code
	// generated by the compiler, and code I actually wrote.
	asm("nop");
	asm("nop");
	
	// The code below replaces a function that literally did 
	// nothing more than what these two lines do.
	float defaultThrottlePlateAngle = *pTargetThrottlePlatePosition_In;
	*pTargetThrottlePlatePosition_Out = defaultThrottlePlateAngle;
	
	UpdateCounter();
	UpdateState();
	SetTargetRpm();
	
	if (*pCoolantTemperature < MinCoolantTemperature)
	{
		return;
	}
	
	RamVariables *pRV = pRamVariables;
	
	// Match revs for a downshift?
	if ((pRamVariables->RevMatchState == RevMatchDecelerationDownshift) ||
		(pRamVariables->RevMatchState == RevMatchAccelerationDownshift))
	{
		// Check the clutch again, just to be 100% certain that this
		// code never opens the throttle without the clutch pressed.
		if (*pCruiseFlagsA & CruiseFlagsAClutch)
		{
			*pTargetThrottlePlatePosition_Out = Pull2d(&RevMatchTable, pRamVariables->DownshiftRpm);
		}
		else
		{
			// This should never happen!
			// Set a breakpoint here, so if it does happen, it'll be obvious.
			pRamVariables->RevMatchState = RevMatchEnabled;
		}
		
		return;
	}
		
	if (pRamVariables->RevMatchState == RevMatchCalibration)
	{
		// Check the clutch again, just to be 100% certain that this
		// code never opens the throttle without the clutch pressed.
		if (!(*pCruiseFlagsA & CruiseFlagsAClutch) || (*pSpeed > 1))
		{
			pRamVariables->RevMatchState = RevMatchEnabled;
			return;
		}
		
		if ((*pCruiseFlagsA & CruiseFlagsAResumeAccel) && (pRamVariables->RevMatchCalibrationIndexChanged == 0))
		{
			pRamVariables->RevMatchCalibrationIndex++;
			pRamVariables->RevMatchCalibrationIndexChanged = 1;
		}
		else if ((*pCruiseFlagsA & CruiseFlagsASetCoast) && (pRamVariables->RevMatchCalibrationIndexChanged == 0))
		{
			pRamVariables->RevMatchCalibrationIndex--;
			pRamVariables->RevMatchCalibrationIndexChanged = 1;
		}
		else
		{
			pRamVariables->RevMatchCalibrationIndexChanged = 0;
		}

		if (pRamVariables->RevMatchCalibrationIndex < 0)
		{
			pRamVariables->RevMatchCalibrationIndex = 0;
		}

		if (pRamVariables->RevMatchCalibrationIndex >= RevMatchTable.elementCount)
		{
			pRamVariables->RevMatchCalibrationIndex = RevMatchTable.elementCount - 1;
		}
		
		pRamVariables->DownshiftRpm = RevMatchTable.inputArray[pRamVariables->RevMatchCalibrationIndex];
		*pTargetThrottlePlatePosition_Out = Pull2d(&RevMatchTable, pRamVariables->DownshiftRpm);
		
		return;
	}

	// More nopcodes to distinguish between my code and the compiler's code.	
	asm("nop");
	asm("nop");
}


void GetRevMatchTableInfo() __attribute__ ((section ("Misc")));
void GetRevMatchTableInfo()
{
}