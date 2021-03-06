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
extern float RevMatchMinimumSpeed;
extern float RevMatchMaximumThrottle;

extern int RevMatchDuration;
extern int RevMatchAccelerationDownshiftReadyDuration;
extern int RevMatchEnableDelay;
extern int RevMatchCalibrationDelay;

extern float RevMatchProportionalGain;

extern TwoDimensionalTable RevMatchTable, RevMatchDownshiftAdjustmentTable;
extern float RevMatchInputValues;
extern float RevMatchOutputValues;

extern float RevMatchDownshiftAdjustmentInputValues;
extern float RevMatchDownshiftAdjustmentOutputValues;
	
extern char RevMatchEnableFeedback, RevMatchEnableCalibrationFeedback;


float RpmWindow(float rpm) __attribute__ ((section ("RomHole_RevMatchCode")));
float RpmWindow(float rpm)
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
	
	switch(pRamVariables->RevMatchFromGear)
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
			pRamVariables->UpshiftRpm = MinTargetRpm;
			pRamVariables->DownshiftRpm = MinTargetRpm;
			return;
	}
	
	pRamVariables->UpshiftRpm = RpmWindow((*pSpeed * 1000.0f) / upshift);
	pRamVariables->DownshiftRpm = RpmWindow((*pSpeed * 1000.0f) / downshift);
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
	
	// Since the logger assumes that 4-byte values are floats...
	pRamVariables->CounterAsFloat = pRamVariables->Counter;
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
	int threshold = duration;
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
	if (ConditionWithDelay(condition, RevMatchEnableDelay))
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
	// The speed condition here and below ensures that the downshift conditions 
	// aren't met at a standstill, which enables the transition to calibration.
	if ((*pCruiseFlagsA & CruiseFlagsALightBrake) &&
		(*pCruiseFlagsA & CruiseFlagsAClutch) &&
		!(*pCruiseFlagsA & CruiseFlagsACancel) &&
		*pSpeed > RevMatchMinimumSpeed && 
		*pCurrentGear > 1)
	{
		return RevMatchDecelerationDownshift;
	}
	
	if ((*pCruiseFlagsA & CruiseFlagsACancel) &&
		!(*pCruiseFlagsA & CruiseFlagsALightBrake) &&
		*pSpeed > RevMatchMinimumSpeed &&
		*pCurrentGear > 1)
	{
		return RevMatchReadyForAccelerationDownshift;
	}
	
	int condition = 
		((*pCruiseFlagsA & CruiseFlagsACancel) &&
		(*pCruiseFlagsA & (CruiseFlagsAHardBrake | CruiseFlagsALightBrake)) &&
		(*pNeutralAndOtherFlags & NeutralSwitchBit) &&
		(*pSpeed < 1));
	
	if (ConditionWithDelay(condition, RevMatchCalibrationDelay))
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
	if (cancelReleased && StateTimeout(RevMatchAccelerationDownshiftReadyDuration))
	{
		return RevMatchEnabled;
	}
	
	// If the driver decides to accelerate without downshifting
	if (*pThrottlePedal > RevMatchMaximumThrottle)
	{
		return RevMatchEnabled;
	}
	
	return 0;
}


enum RevMatchStates EvaluateTransitionDecelerationDownshift() __attribute__ ((section ("RomHole_RevMatchCode")));
enum RevMatchStates EvaluateTransitionDecelerationDownshift()
{
	if (StateTimeout(RevMatchDuration))
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
	if (StateTimeout(RevMatchDuration))
	{
		return RevMatchExpired;
	}

	if (!(*pCruiseFlagsA & CruiseFlagsAClutch))
	{
		return RevMatchReadyForAccelerationDownshift;
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

	if (*pNeutralAndOtherFlags & CruiseFlagsAClutch)
	{
		return RevMatchEnabled;
	}

	if (!(*pNeutralAndOtherFlags & NeutralSwitchBit))
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

void SetCalibrationThrottle(void) __attribute__((section("RomHole_RevMatchCode")));
void SetCalibrationThrottle()
{
	if (pRamVariables->RevMatchCalibrationIndex < 0)
	{
		pRamVariables->RevMatchCalibrationIndex = 0;
	}

	if (pRamVariables->RevMatchCalibrationIndex >= RevMatchTable.elementCount)
	{
		pRamVariables->RevMatchCalibrationIndex = RevMatchTable.elementCount - 1;
	}

	float target = RevMatchTable.inputArray[pRamVariables->RevMatchCalibrationIndex];

	// Setting these variables isn't strictly necessary but it gives the unit tests
	// an additional way to verify that we've taken the expected code path.
	pRamVariables->UpshiftRpm = pRamVariables->DownshiftRpm = RpmWindow(target);
	
	pRamVariables->RevMatchCalibrationThrottle = Pull2d(&RevMatchTable, pRamVariables->DownshiftRpm);
	
	RevMatchResetFeedback();
}

void UpdateState() __attribute__((section("RomHole_RevMatchCode")));
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
		pRamVariables->RevMatchFeedbackEnabled = RevMatchEnableFeedback;
		pRamVariables->RevMatchCalibrationFeedbackEnabled = RevMatchEnableCalibrationFeedback;
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

	switch (nextState)
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
		RevMatchResetFeedback();
		pRamVariables->RevMatchTransitionEvaluator = EvaluateTransitionDecelerationDownshift;
		break;

	case RevMatchAccelerationDownshift:
		RevMatchResetFeedback();
		pRamVariables->RevMatchTransitionEvaluator = EvaluateTransitionAccelerationDownshift;
		break;

	case RevMatchCalibration:
		pRamVariables->RevMatchTransitionEvaluator = EvaluateTransitionCalibration;
		pRamVariables->RevMatchCalibrationIndex = 0;
		pRamVariables->RevMatchCalibrationThrottleChanged = 0;
		pRamVariables->RevMatchCalibrationIndexChanged = 0;
		SetCalibrationThrottle();
		break;

	case RevMatchExpired:
		pRamVariables->RevMatchTransitionEvaluator = EvaluateTransitionExpired;
		break;
	}
}

void AdjustCalibrationThrottle(void) __attribute__((section("RomHole_RevMatchCode")));
void AdjustCalibrationThrottle()
{
	if (*pCruiseFlagsA & CruiseFlagsAResumeAccel)
	{
		if (pRamVariables->RevMatchCalibrationThrottleChanged == 0)
		{
			pRamVariables->RevMatchCalibrationThrottle += 0.5;
			pRamVariables->RevMatchCalibrationThrottleChanged = 1;
		}
	}
	else if (*pCruiseFlagsA & CruiseFlagsASetCoast)
	{
		if (pRamVariables->RevMatchCalibrationThrottleChanged == 0)
		{
			pRamVariables->RevMatchCalibrationThrottle -= 0.5;
			pRamVariables->RevMatchCalibrationThrottleChanged = 1;
		}
	}
	else
	{
		pRamVariables->RevMatchCalibrationThrottleChanged = 0;
	}
}

void AdjustCalibrationIndex(void) __attribute__((section("RomHole_RevMatchCode")));
void AdjustCalibrationIndex()
{
	if (*pCruiseFlagsA & CruiseFlagsAResumeAccel)
	{
		if (pRamVariables->RevMatchCalibrationIndexChanged == 0)
		{
			pRamVariables->RevMatchCalibrationIndex++;
			pRamVariables->RevMatchCalibrationIndexChanged = 1;
		}
	}
	else if (*pCruiseFlagsA & CruiseFlagsASetCoast)
	{
		if (pRamVariables->RevMatchCalibrationIndexChanged == 0)
		{
			pRamVariables->RevMatchCalibrationIndex--;
			pRamVariables->RevMatchCalibrationIndexChanged = 1;
		}
	}
	else
	{
		pRamVariables->RevMatchCalibrationIndexChanged = 0;
	}

	if (pRamVariables->RevMatchCalibrationIndexChanged == 1)
	{		 
		SetCalibrationThrottle();
	}
}

// The original ECU code will cut fuel and add more throttle, and those
// behaviors need to be defeated for rev matching to work.
void FixDefaultBehaviors(void) __attribute__((section("RomHole_RevMatchCode")));
void FixDefaultBehaviors()
{
	// All 3 together are needed defeat fuel cut.
	*pOverrunFuelCutFlags_1 = OverrunFuelCutFlags_1_Defeat;	
	*pOverrunFuelCutFlags_2 = 0; 
	*pOverrunFuelCutFlags_3 = 0; 

	// The original ECU code adds this value to the throttle plate target, 
	// which will cause the engine to rev much faster than the desired RPM.
	// Forcing this to zero fixes that problem.
	*pThrottleCompensation = 0;
}

void RevMatchCode() __attribute__ ((section ("RomHole_RevMatchCode")));
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

	if (pRamVariables->RevMatchState != RevMatchCalibration)
	{
		SetTargetRpm();
	}

	if (!(*pCruiseFlagsA & CruiseFlagsAClutch))
	{
		// We only update our copy of this variable while the clutch is not pressed.
		// The ECU will keep guessing at the current gear even while the clutch IS
		// pressed, which causes to guess wrong at times.
		pRamVariables->RevMatchFromGear = *pCurrentGear;
	}

	if (*pCoolantTemperature < MinCoolantTemperature)
	{
		return;
	}
	
	RamVariables *pRV = pRamVariables;
	
	// Match revs for a downshift?
	if ((pRamVariables->RevMatchState == RevMatchDecelerationDownshift) ||
		(pRamVariables->RevMatchState == RevMatchAccelerationDownshift))
	{
		// Not sure if this should move into state transiitons or should
		// just stay here. State transition is more elegant but this is 
		// simpler and doesn't need to be duplicated across two states.
		if (*pSpeed < RevMatchMinimumSpeed)
		{
			// Rev matching is annoying when you're coming to a stop.
			return;
		}
				
		// Target RPM can be adjusted for braking downshifts.
		float targetRpm = pRamVariables->DownshiftRpm;
		if (pRamVariables->RevMatchState == RevMatchDecelerationDownshift)
		{
			targetRpm = Pull2d(&RevMatchDownshiftAdjustmentTable, targetRpm);
			targetRpm = RpmWindow(targetRpm);
		}

		// Check the clutch again, just to be 100% certain that this
		// code never opens the throttle without the clutch pressed.
		if (*pCruiseFlagsA & CruiseFlagsAClutch)
		{			
			*pTargetThrottlePlatePosition_Out = RevMatchGetThrottle(targetRpm);
			FixDefaultBehaviors();
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
		if (*pCruiseFlagsA & CruiseFlagsAEnableButton)
		{
			AdjustCalibrationIndex();
		}
		else
		{
			AdjustCalibrationThrottle();
		}

		// Check the neutral switch, to be 100% certain that this
		// code never opens the throttle with the car in gear.
		// The speed < 1 condition should mean the car isn't moving, 
		// with a small allowance for possible noise from the sensor.
		if ((*pNeutralAndOtherFlags & NeutralSwitchBit) && (*pSpeed < 1))
		{
			if (pRamVariables->RevMatchCalibrationFeedbackEnabled)
			{
				// Integral windup can close the throttle and stall the engine.
				float minimumThrottle = RevMatchOutputValues;
				float targetThrottle = RevMatchGetThrottle(pRamVariables->DownshiftRpm);
				if (targetThrottle > minimumThrottle)
				{
					*pTargetThrottlePlatePosition_Out = targetThrottle;
				}
				else
				{
					*pTargetThrottlePlatePosition_Out = minimumThrottle;
				}
			}
			else
			{
				*pTargetThrottlePlatePosition_Out = pRamVariables->RevMatchCalibrationThrottle;
			}

			FixDefaultBehaviors();
		}
		else
		{
			pRamVariables->RevMatchState = RevMatchEnabled;
		}
					
		return;
	}

	// More nopcodes to distinguish between my code and the compiler's code.	
	asm("nop");
	asm("nop");
}


void GetRevMatchTableInfo() __attribute__ ((section ("Misc")));
void GetRevMatchTableInfo()
{
	void* address = 0;
	address = &Gear1Multiplier; // 6 elements, gear multipliers
	address = &MinTargetRpm; // 2 elements, min and max target RPM
	address = &MinCoolantTemperature;
	address = &RevMatchDuration; // Delays: rev match active duration, accel downshift ready timeout, feature enable delay, calibration entry delay
	address = &RevMatchInputValues;
	address = &RevMatchOutputValues;
	address = &RevMatchEnableFeedback;
	address = &RevMatchEnableCalibrationFeedback;
	address = &RevMatchProportionalGain;
	address = &RevMatchEnableFeedback;
	address = &RevMatchDownshiftAdjustmentInputValues;
	address = &RevMatchDownshiftAdjustmentOutputValues;
	
	address = &(pRamVariables->RevMatchState); // single byte
	address = &(pRamVariables->Counter); // 4 bytes
	address = &(pRamVariables->UpshiftRpm); // 4 bytes
	address = &(pRamVariables->DownshiftRpm); // 4 bytes
	address = &(pRamVariables->RevMatchCalibrationIndex); // single byte
	address = &(pRamVariables->CounterAsFloat); // 4 bytes
	address = &(pRamVariables->RevMatchProportionalFeedback); // 4 bytes
	address = &(pRamVariables->RevMatchIntegralFeedback); // 4 bytes
}