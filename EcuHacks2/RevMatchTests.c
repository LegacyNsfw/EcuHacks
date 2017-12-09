#include "EcuHacks.h"

void RevMatchResetAndEnable() __attribute__ ((section ("Misc")));
void RevMatchResetAndEnable()
{
	// Disable.
	*pRPM = 100;
	RevMatchCode();
	
	// Press cancel.
	*pRPM = 2500.0f;
	*pCruiseFlagsA = CruiseFlagsACancel;
	RevMatchCode();
	
	// Hold cancel.
	pRamVariables->Counter = 1005;
	RevMatchCode();
	
	// Release cancel.
	*pCruiseFlagsA = 0;
	RevMatchCode();
}

// All test functions must be explicitly put into the Misc section, otherwise
// the compiler/linker will put them in an address range that conflicts with
// ECU code.

void UpdateCounter();
unsigned int GetElapsed(int start);

void RevMatchCounterTests() __attribute__ ((section ("Misc")));
void RevMatchCounterTests()
{
	int start = MAX_COUNTER - 2;
	pRamVariables->Counter = start;
	UpdateCounter();
	UpdateCounter();
	UpdateCounter();
	UpdateCounter();
	unsigned int result = GetElapsed(start);
	AssertEqualInts(result, 4, "Four ticks");
}

// Test rev matching in downshifts.
void RevMatchDownshiftTests() __attribute__ ((section ("Misc")));
void RevMatchDownshiftTests()
{
	// Makes the debugger watch window easier to use.
	RamVariables *pRV = pRamVariables;
	
	// Need sane values to avoid div/0, etc.
	*pSpeed = 50.0f;
	*pRPM = 2500.0f;
	*pCoolantTemperature = 80;
	*pCurrentGear = 3;
	*pTargetThrottlePlatePosition_In = 8.0f;
	*pTargetThrottlePlatePosition_Out = 0.0f;
	*pCruiseFlagsA = 0;
	
	RevMatchResetAndEnable();
	pRV->RevMatchFeedbackEnabled = 0;
	
	// Confirm no throttle change by default.
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchEnabled, "Mode should be 'enabled'");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, 8.0f, "no throttle change by default");
	AssertTrue(pRamVariables->UpshiftRpm < 2000, "Upshift RPM is sane 1");
	AssertTrue(pRamVariables->UpshiftRpm > 1500, "Upshift RPM is sane 2");
	AssertTrue(pRamVariables->DownshiftRpm > 3500, "Downshift RPM is sane 1");
	AssertTrue(pRamVariables->DownshiftRpm < 4000, "Downshift RPM is sane 2");
	
	float expectedThrottle = 11.06268;
	
	// Match revs with a braking downshift.
	*pCruiseFlagsA = CruiseFlagsALightBrake | CruiseFlagsAClutch; 
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchDecelerationDownshift, "Mode should be braking downshift");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, expectedThrottle, "Throttle changed - deceleration downshift.");
	
	// Confirm no throttle change after brake is released.
	*pCruiseFlagsA = CruiseFlagsAClutch;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchEnabled, "");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, 8.0f, "no throttle change after brake released");
	
	// Match revs with a braking downshift again.
	*pCruiseFlagsA = CruiseFlagsALightBrake | CruiseFlagsAClutch;
	RevMatchCode();	
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchDecelerationDownshift, "Mode should be braking downshift");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, expectedThrottle, "Throttle changed - deceleration downshift again.");
	
	// Confirm no throttle change after clutch is released.
	*pCruiseFlagsA = CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchEnabled, "");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, 8.0f, "no throttle change after clutch released");
	
	// Match revs with a braking downshift again.
	*pCruiseFlagsA = CruiseFlagsALightBrake | CruiseFlagsAClutch;
	RevMatchCode();	
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchDecelerationDownshift, "Mode should be braking downshift");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, expectedThrottle, "Throttle changed - deceleration downshift yet again.");

	// Confirm no throttle change after countdown timer runs out.
	int i;
	for (i = 0; i < 125; i++)
	{
		RevMatchCode();	
	}
	
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, expectedThrottle, "Throttle changed - waiting for timeout.");
	
	for (i = 0; i < 130; i++)
	{
		RevMatchCode();	
	}
	
	RevMatchCode();	
	
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, 8.0f, "no throttle change after countdown timer expires.");
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchExpired, "Mode should be 'expired'");
	
	// Re-enabled after both pedals are released.
	*pCruiseFlagsA = 0;
	RevMatchCode();		
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchEnabled, "Mode should be 'enabled'");
	
	// Switch to acceleration-downshift mode.
	*pCruiseFlagsA = CruiseFlagsACancel;
	RevMatchCode();
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchReadyForAccelerationDownshift, "Mode should be 'ready for acceleration downshift'");
	
	// Match revs with a non-braking downshift.
	*pCruiseFlagsA = CruiseFlagsAClutch;
	RevMatchCode();	
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchAccelerationDownshift, "Mode should be 'acceleration downshift'");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, expectedThrottle, "Throttle changed - acceleration downshift.");
	
	// Confirm no throttle change after clutch is released.
	*pCruiseFlagsA = 0;
	RevMatchCode();	
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchEnabled, "Mode should be 'enabled'");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, 8.0f, "Throttle NOT changed - acceleration downshift terminated by clutch release.");

	// Match revs with a non-braking downshift again.
	*pCruiseFlagsA = CruiseFlagsACancel;
	RevMatchCode();
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchReadyForAccelerationDownshift, "Mode should be 'ready for acceleration downshift'");

	*pCruiseFlagsA = CruiseFlagsAClutch;
	RevMatchCode();	
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchAccelerationDownshift, "Mode should be 'acceleration downshift'");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, expectedThrottle, "Throttle changed - acceleration downshift again.");
	
	// Confirm no throttle change after countdown timer runs out.
	for (i = 0; i < 125; i++)
	{
		RevMatchCode();	
	}
	
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, expectedThrottle, "Throttle changed - waiting for timeout.");
	
	for (i = 0; i < 130; i++)
	{
		RevMatchCode();	
	}
	
	RevMatchCode();	
	
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, 8.0f, "no throttle change after countdown timer expires.");
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchExpired, "Mode should be 'expired'");
}

// Test the mode-switch feature for the rev match hack.
void RevMatchStateTests() __attribute__ ((section ("Misc")));
void RevMatchStateTests()
{
	// Makes the debugger watch window easier to use.
	RamVariables *pRV = pRamVariables;

	// Need sane values to avoid div/0, etc.
	*pSpeed = 50.0f;
	*pRPM = 2500.0f;
	*pCoolantTemperature = 80;
	*pCurrentGear = 3;
	
	// Show that the 'enable' flag is turned off when starting/stopping the engine.	
	*pCruiseFlagsA = 0;
	pRamVariables->RevMatchState = RevMatchEnabled;
	*pRPM = 400;
	*pTargetThrottlePlatePosition_Out = 0;
	*pTargetThrottlePlatePosition_In = 12.5f;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchDisabled, "Rev match should be disabled when RPM < 500");
	// Assert(pRamVariables->Counter == 0, "Counter should be reset when RPM < 500");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, 12.5f, "Throttle setting should be updated.");
	
	// Enable flag stays off when engine speed exceeds the threshold
	*pRPM = 750;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchDisabled, "Rev match should not automatically enable when RPM > 500");

	// Briefly tapping the cruise-cancel switch doesn't enable rev match.
	*pRPM = 750;
	*pCruiseFlagsA = CruiseFlagsACancel;
	RevMatchCode();
	RevMatchCode(); 
	RevMatchCode();
	RevMatchCode();
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchDisabled, "Rev match should not automatically enable when RPM > 500 and cruise-cancel touched.");
	
	// Holding cancel does enable rev-match
	//
	// TODO: adjust iteration count to give ~1 second delay
	*pCruiseFlagsA = CruiseFlagsACancel;
	int count;
	for(count = 0; count < 255; count++)
	{
		RevMatchCode();
	}
		
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchAlmostEnabled, "Rev match should automatically enable appropriate delay.");
	
	// Release the cruise-cancel switch after a long press
	*pCruiseFlagsA = 0;
	RevMatchCode();
	
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchEnabled, "Rev match remains enabled.");
	
	// When rev match is enabled, tapping cruise-cancel switches to the acceleration-downshift mode
	*pCruiseFlagsA = CruiseFlagsACancel;
	RevMatchCode(); // start counting
	
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchReadyForAccelerationDownshift, "Acceleration downshift enabled after tapping cancel.");
	
	// Acceleration-downshift remains engaged for a couple seconds, then reverts to enabled.
	*pCruiseFlagsA = 0;
	RevMatchCode();
	
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchReadyForAccelerationDownshift, "Acceleration downshift remains enabled after releasing cancel.");
	
	*pCruiseFlagsA = 0;
	for(count = 0; count < 125; count++)
	{
		RevMatchCode();
	}

	AssertEqualInts(pRamVariables->RevMatchState, RevMatchReadyForAccelerationDownshift, "Acceleration downshift remains enabled ~1s after releasing cancel.");
	
	for(count = 0; count < 130; count++)
	{
		RevMatchCode();
	}

	AssertEqualInts(pRamVariables->RevMatchState, RevMatchEnabled, "Acceleration downshift reverts to 'enabled' after another second.");
	
	// Hold the set/coast switch plus clutch and brake for 5 seconds to enter calibration mode.
	*pSpeed = 0;
	*pCruiseFlagsA = CruiseFlagsACancel | CruiseFlagsALightBrake | CruiseFlagsAClutch;
	pRamVariables->RevMatchCalibrationIndex = 123;
	for(count = 0; count < 630; count++)
	{
		RevMatchCode();
	}
	
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchCalibration, "Holding 5 seconds while enabled switches to calibration mode.");
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "Rev match calibration index is initialized to zero.");
	AssertEqualFloats(pRamVariables->RevMatchCalibrationThrottle, 4.5, "Rev match throttle.");
	
	// Exit calibration mode by releasing the clutch.
	*pCruiseFlagsA = 0;
	RevMatchCode(); 	
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchEnabled, "From calibration mode, release clutch to switch to enabled.");

	// Hold the cruise-cancel switch WITHOUT the clutch for 5 seconds just prepares for a downshift.
	// Note that we'll toggle between Enabled and ReadyForAccelDownshift due to timeouts.
	// Could add ConditionReleaseCancel to the timeout, but... not worth the trouble.
	*pCruiseFlagsA = CruiseFlagsACancel;
	*pSpeed = 50;
	for(count = 0; count < 650; count++)
	{
		RevMatchCode();
	}
	
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchReadyForAccelerationDownshift, "Holding 5 seconds while enabled switches to calibration mode.");
}

void RevMatchCalibrationIndexTests() __attribute__ ((section ("Misc")));
void RevMatchCalibrationIndexTests()
{
	// Makes the debugger watch window easier to use.
	RamVariables *pRV = pRamVariables;

	RevMatchResetAndEnable();
	pRV->RevMatchFeedbackEnabled = 0;
	
	*pSpeed = 0;
	*pCruiseFlagsA = CruiseFlagsACancel | CruiseFlagsALightBrake | CruiseFlagsAClutch;
	RevMatchCode();
	
	pRamVariables->Counter += 650;
	pRamVariables->RevMatchCalibrationIndex = -1;
	RevMatchCode();
	
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchCalibration, "Switched to calibration mode.");
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "Negative calibration index gets fixed.");
	AssertEqualFloats(pRamVariables->DownshiftRpm, 1000.0f, "RPM for index 0.");
	AssertEqualFloats(pRamVariables->RevMatchCalibrationThrottle, 4.5f, "Throttle for index 0.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, 4.5f, "Throttle position for index 0.");
	
	// Prove the user can't bring the index below zero.
	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");

	// Increase index step by step. 
	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 1, "Increase index.");

	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 1, "No change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 1, "No change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 2, "Increase index.");

	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 2, "No change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 2, "No change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 3, "Increase index.");

	// Prove the user can't increase the index past 3.
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 3, "No change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 3, "No change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 3, "No change.");

	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 3, "No change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 3, "No change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 3, "Calibration index remains 3.");
	AssertEqualFloats(pRamVariables->DownshiftRpm, 6500.0f, "RPM for index 3.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, 18.0f, "Throttle position for index 3.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 3, "index does not change.");

	// Decrease index step by step.
	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 2, "Decrease index.");
	AssertEqualFloats(pRamVariables->DownshiftRpm, 5000.0, "RPM for index 2.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, 14.0f, "Throttle position for index 2.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 2, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 2, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 1, "Decrease index.");
	AssertEqualFloats(pRamVariables->DownshiftRpm, 3000.0f, "RPM for index 1.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, 10.0f, "Throttle position for index 1.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 1, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 1, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "Decrease index.");
	AssertEqualFloats(pRamVariables->DownshiftRpm, 1000.0f, "RPM for index 0.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, 4.5f, "Throttle position for index 0.");

	*pCruiseFlagsA = 0;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchState, RevMatchEnabled, "Exit calibration mode.");
}

void RevMatchCalibrationThrottleTests() __attribute__((section("Misc")));
void RevMatchCalibrationThrottleTests()
{
	const float rpmIndex0 = 1000.0f;
	const float throttleIndex0 = 4.5f;
	const float throttleIndex1 = 10.0f;
	
	// Makes the debugger watch window easier to use.
	RamVariables *pRV = pRamVariables;

	RevMatchResetAndEnable();

	// Temporarily disable calibration feedback to enable calibration throttle adjustment.
	pRamVariables->RevMatchCalibrationFeedbackEnabled = 0;
	*pSpeed = 0;
	*pCruiseFlagsA = CruiseFlagsACancel | CruiseFlagsALightBrake | CruiseFlagsAClutch;
	RevMatchCode();

	pRamVariables->Counter += 650;
	RevMatchCode();

	AssertEqualInts(pRamVariables->RevMatchState, RevMatchCalibration, "Switched to calibration mode.");
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "Default calibration index.");
	AssertEqualFloats(pRamVariables->DownshiftRpm, rpmIndex0, "RPM for index 0.");
	AssertEqualFloats(pRamVariables->RevMatchCalibrationThrottle, throttleIndex0, "Throttle for index 0.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, throttleIndex0, "Throttle position for index 0.");

	// Increase throttle.
	*pCruiseFlagsA = CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, throttleIndex0 + 0.5f, "Throttle position after increment.");

	// Do not increase throttle again, until set/coast button is released.
	*pCruiseFlagsA = CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, throttleIndex0 + 0.5f, "Throttle does not change.");

	// Reset switch state
	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, throttleIndex0 + 0.5f, "Throttle does not change.");

	// Increase again
	*pCruiseFlagsA = CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, throttleIndex0 + 1.0f, "Throttle increases.");

	// Reset switch state
	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, throttleIndex0 + 1.0f, "Throttle does not change.");

	// Decrease throttle
	*pCruiseFlagsA = CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, throttleIndex0 + 0.5f, "Throttle decreases.");

	// No change, switch must be reset first
	*pCruiseFlagsA = CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, throttleIndex0 + 0.5f, "Throttle not changed.");

	// Reset switch state
	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, throttleIndex0 + 0.5f, "Throttle does not change.");

	// Decrease throttle
	*pCruiseFlagsA = CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, throttleIndex0, "Throttle decreases.");

	// Reset switch state
	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 0, "index does not change.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, throttleIndex0, "Throttle does not change.");

	// Increase index
	*pCruiseFlagsA = CruiseFlagsAEnableButton | CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	AssertEqualInts(pRamVariables->RevMatchCalibrationIndex, 1, "Increase index.");
	AssertEqualFloats(*pTargetThrottlePlatePosition_Out, throttleIndex1, "Throttle for index 1.");
}

void RevMatchFeedbackTests() __attribute__ ((section ("Misc")));
void RevMatchFeedbackTests()
{
	RevMatchResetFeedback();
	pRamVariables->RevMatchFeedbackEnabled = 0;
	
	*pRPM = 2800;

	float result = RevMatchGetThrottle(3000);
	AssertEqualFloats(result, 10.0, "Raw throttle at 3000");
	
	pRamVariables->RevMatchFeedbackEnabled = 1;
	result = RevMatchGetThrottle(3000);
	AssertEqualFloats(result, 10.41, "Throttle + proportional");
	
	result = RevMatchGetThrottle(3000);
	AssertEqualFloats(result, 10.42, "Throttle + proportional + integral");

	result = RevMatchGetThrottle(3000);
	AssertEqualFloats(result, 10.43, "Throttle + proportional + integral again");

	AssertEqualFloats(pRamVariables->RevMatchDerivativeFeedback, 0.0f, "Zero derivative feedback");
	
	*pRPM = 2900;
	
	result = RevMatchGetThrottle(3000);
	
	float d = pRamVariables->RevMatchDerivativeFeedback;
	
	AssertEqualFloats(d, -0.0002, "Derivative term should be negative as error has gotten smaller.");
}