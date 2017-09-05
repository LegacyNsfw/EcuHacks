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

// Test rev matching.
void RevMatchUnitTests() __attribute__ ((section ("Misc")));
void RevMatchUnitTests()
{
	// Makes the debugger watch window easier to use.
	RamVariables *pRV = pRamVariables;
	
	// Need sane values to avoid div/0, etc.
	*pVehicleSpeed = 50.0f;
	*pRPM = 2500.0f;
	*pCoolantTemperature = 80;
	*pCurrentGear = 3;
	*pTargetThrottlePlatePosition_In = 10.0f;
	*pTargetThrottlePlatePosition_Out = 0.0f;
	*pCruiseFlagsA = 0;
	
	RevMatchResetAndEnable();
	
	// Confirm no throttle change by default.
	Assert(pRamVariables->RevMatchState == RevMatchEnabled, "Mode should be 'enabled'");
	Assert(*pTargetThrottlePlatePosition_Out == 10.0f, "no throttle change by default");
	Assert(pRamVariables->UpshiftRpm < 2000, "Upshift RPM is sane 1");
	Assert(pRamVariables->UpshiftRpm > 1500, "Upshift RPM is sane 2");
	Assert(pRamVariables->DownshiftRpm > 3500, "Downshift RPM is sane 1");
	Assert(pRamVariables->DownshiftRpm < 4000, "Downshift RPM is sane 2");
	
	// Match revs with a braking downshift.
	*pCruiseFlagsA = CruiseFlagsALightBrake | CruiseFlagsAClutch;
	RevMatchCode();
	Assert(pRamVariables->RevMatchState == RevMatchDecelerationDownshift, "Mode should be braking downshift");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 13.32f), "Throttle changed - deceleration downshift.");
	
	// Confirm no throttle change after brake is released.
	*pCruiseFlagsA = CruiseFlagsAClutch;
	RevMatchCode();
	Assert(pRamVariables->RevMatchState == RevMatchEnabled, "");
	Assert(*pTargetThrottlePlatePosition_Out == 10.0f, "no throttle change after brake released");
	
	// Match revs with a braking downshift again.
	*pCruiseFlagsA = CruiseFlagsALightBrake | CruiseFlagsAClutch;
	RevMatchCode();	
	Assert(pRamVariables->RevMatchState == RevMatchDecelerationDownshift, "Mode should be braking downshift");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 13.32f), "Throttle changed - deceleration downshift again.");
	
	// Confirm no throttle change after clutch is released.
	*pCruiseFlagsA = CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchState == RevMatchEnabled, "");
	Assert(*pTargetThrottlePlatePosition_Out == 10.0f, "no throttle change after clutch released");
	
	// Match revs with a braking downshift again.
	*pCruiseFlagsA = CruiseFlagsALightBrake | CruiseFlagsAClutch;
	RevMatchCode();	
	Assert(pRamVariables->RevMatchState == RevMatchDecelerationDownshift, "Mode should be braking downshift");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 13.32f), "Throttle changed - deceleration downshift yet again.");
		
	// Confirm no throttle change after countdown timer runs out.
	int i;
	for (i = 0; i < 500; i++)
	{
		RevMatchCode();	
	}
	
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 13.32f), "Throttle changed - waiting for timeout.");
	
	for (i = 0; i < 515; i++)
	{
		RevMatchCode();	
	}
	
	RevMatchCode();	
	
	Assert(*pTargetThrottlePlatePosition_Out == 10.0f, "no throttle change after countdown timer expires.");
	Assert(pRamVariables->RevMatchState == RevMatchExpired, "Mode should be 'expired'");
	
	// Re-enabled after both pedals are released.
	*pCruiseFlagsA = 0;
	RevMatchCode();		
	Assert(pRamVariables->RevMatchState == RevMatchEnabled, "Mode should be 'enabled'");
	
	// Switch to acceleration-downshift mode.
	*pCruiseFlagsA = CruiseFlagsACancel;
	RevMatchCode();
	RevMatchCode();
	Assert(pRamVariables->RevMatchState == RevMatchReadyForAccelerationDownshift, "Mode should be 'ready for acceleration downshift'");
	
	// Match revs with a non-braking downshift.
	*pCruiseFlagsA = CruiseFlagsAClutch;
	RevMatchCode();	
	Assert(pRamVariables->RevMatchState == RevMatchAccelerationDownshift, "Mode should be 'acceleration downshift'");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 13.32f), "Throttle changed - acceleration downshift.");
	
	// Confirm no throttle change after clutch is released.
	*pCruiseFlagsA = 0;
	RevMatchCode();	
	Assert(pRamVariables->RevMatchState == RevMatchEnabled, "Mode should be 'enabled'");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 10.0f), "Throttle NOT changed - acceleration downshift terminated by clutch release.");

	// Match revs with a non-braking downshift again.
	*pCruiseFlagsA = CruiseFlagsACancel;
	RevMatchCode();
	RevMatchCode();
	Assert(pRamVariables->RevMatchState == RevMatchReadyForAccelerationDownshift, "Mode should be 'ready for acceleration downshift'");

	*pCruiseFlagsA = CruiseFlagsAClutch;
	RevMatchCode();	
	Assert(pRamVariables->RevMatchState == RevMatchAccelerationDownshift, "Mode should be 'acceleration downshift'");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 13.32f), "Throttle changed - acceleration downshift again.");
	
	// Confirm no throttle change after countdown timer runs out.
	for (i = 0; i < 500; i++)
	{
		RevMatchCode();	
	}
	
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 13.32f), "Throttle changed - waiting for timeout.");
	
	for (i = 0; i < 605; i++)
	{
		RevMatchCode();	
	}
	
	RevMatchCode();	
	
	Assert(*pTargetThrottlePlatePosition_Out == 10.0f, "no throttle change after countdown timer expires.");
	Assert(pRamVariables->RevMatchState == RevMatchExpired, "Mode should be 'expired'");
}

// Test the mode-switch feature for the rev match hack.
void RevMatchStateUnitTests() __attribute__ ((section ("Misc")));
void RevMatchStateUnitTests()
{
	// Makes the debugger watch window easier to use.
	RamVariables *pRV = pRamVariables;

	// Need sane values to avoid div/0, etc.
	*pVehicleSpeed = 50.0f;
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
	Assert(pRamVariables->RevMatchState == RevMatchDisabled, "Rev match should be disabled when RPM < 500");
	// Assert(pRamVariables->Counter == 0, "Counter should be reset when RPM < 500");
	Assert(*pTargetThrottlePlatePosition_Out == 12.5f, "Throttle setting should be updated.");
	
	// Enable flag stays off when engine speed exceeds the threshold
	*pRPM = 750;
	RevMatchCode();
	Assert(pRamVariables->RevMatchState == RevMatchDisabled, "Rev match should not automatically enable when RPM > 500");

	// Briefly tapping the cruise-cancel switch doesn't enable rev match.
	*pRPM = 750;
	*pCruiseFlagsA = CruiseFlagsACancel;
	RevMatchCode();
	RevMatchCode();
	RevMatchCode();
	RevMatchCode();
	RevMatchCode();
	Assert(pRamVariables->RevMatchState == RevMatchDisabled, "Rev match should not automatically enable when RPM > 500 and cruise-cancel touched.");
	
	// Holding cancel does enable rev-match
	//
	// TODO: adjust iteration count to give ~1 second delay
	*pCruiseFlagsA = CruiseFlagsACancel;
	int count;
	for(count = 0; count < 1005; count++)
	{
		RevMatchCode();
	}
		
	Assert(pRamVariables->RevMatchState == RevMatchAlmostEnabled, "Rev match should automatically enable appropriate delay.");
	
	// Release the cruise-cancel switch after a long press
	*pCruiseFlagsA = 0;
	RevMatchCode();
	
	Assert(pRamVariables->RevMatchState == RevMatchEnabled, "Rev match remains enabled.");
	
	// When rev match is enabled, tapping cruise-cancel switches to the acceleration-downshift mode
	*pCruiseFlagsA = CruiseFlagsACancel;
	RevMatchCode(); // start counting
	
	Assert(pRamVariables->RevMatchState == RevMatchReadyForAccelerationDownshift, "Acceleration downshift enabled after tapping cancel.");
	
	// Acceleration-downshift remains engaged for a couple seconds, then reverts to enabled.
	*pCruiseFlagsA = 0;
	RevMatchCode();
	
	Assert(pRamVariables->RevMatchState == RevMatchReadyForAccelerationDownshift, "Acceleration downshift remains enabled after releasing cancel.");
	
	*pCruiseFlagsA = 0;
	for(count = 0; count < 500; count++)
	{
		RevMatchCode();
	}

	Assert(pRamVariables->RevMatchState == RevMatchReadyForAccelerationDownshift, "Acceleration downshift remains enabled ~1s after releasing cancel.");
	
	for(count = 0; count < 505; count++)
	{
		RevMatchCode();
	}

	Assert(pRamVariables->RevMatchState == RevMatchEnabled, "Acceleration downshift reverts to 'enabled' after another second.");
	
	// Hold the set/coast switch plus clutch and brake for 5 seconds to enter calibration mode.
	*pSpeed = 0;
	*pCruiseFlagsA = CruiseFlagsACancel | CruiseFlagsALightBrake | CruiseFlagsAClutch;
	pRamVariables->RevMatchCalibrationIndex = 123;
	for(count = 0; count < 5005; count++)
	{
		RevMatchCode();
	}
	
	Assert(pRamVariables->RevMatchState == RevMatchCalibration, "Holding 5 seconds while enabled switches to calibration mode.");
	Assert(pRamVariables->RevMatchCalibrationIndex == 0, "Rev match calibration index is initialized to zero.");
	
	// Exit calibration mode by releasing the clutch.
	*pCruiseFlagsA = 0;
	RevMatchCode(); 	
	Assert(pRamVariables->RevMatchState == RevMatchEnabled, "From calibration mode, release clutch to switch to enabled.");

	// Hold the cruise-cancel switch WITHOUT the clutch for 5 seconds just prepares for a downshift.
	// Note that we'll toggle between Enabled and ReadyForAccelDownshift due to timeouts.
	// Could add ConditionReleaseCancel to the timeout, but... not worth the trouble.
	*pCruiseFlagsA = CruiseFlagsACancel;
	for(count = 0; count < 5005; count++)
	{
		RevMatchCode();
	}
	
	Assert(pRamVariables->RevMatchState == RevMatchReadyForAccelerationDownshift, "Holding 5 seconds while enabled switches to calibration mode.");
}

void RevMatchCalibrationUnitTests() __attribute__ ((section ("Misc")));
void RevMatchCalibrationUnitTests()
{
	// Makes the debugger watch window easier to use.
	RamVariables *pRV = pRamVariables;

	RevMatchResetAndEnable();
	
	*pSpeed = 0;
	*pCruiseFlagsA = CruiseFlagsACancel | CruiseFlagsALightBrake | CruiseFlagsAClutch;
	RevMatchCode();
	
	pRamVariables->Counter += 5005;
	pRamVariables->RevMatchCalibrationIndex = -1;
	RevMatchCode();
	
	Assert(pRamVariables->RevMatchState == RevMatchCalibration, "Switched to calibration mode.");
	Assert(pRamVariables->RevMatchCalibrationIndex == 0, "Negative calibration index gets fixed.");
	Assert(pRamVariables->DownshiftRpm == 2000, "RPM for index 0.");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 8.6f), "Throttle position for index 0.");
	
	pRamVariables->RevMatchCalibrationIndex = 0;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 0, "Calibration index remains zero.");
	Assert(pRamVariables->DownshiftRpm == 2000, "RPM for index 0.");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 8.6f), "Throttle position for index 0.");

	pRamVariables->RevMatchCalibrationIndex = 4;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 3, "Large index gets fixed.");
	Assert(pRamVariables->DownshiftRpm == 6500, "RPM for index 3.");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 21.75f), "Throttle position for index 3.");

	*pCruiseFlagsA = CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 3, "Can't increase index too far.");
	Assert(pRamVariables->DownshiftRpm == 6500, "RPM for index 3.");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 21.75f), "Throttle position for index 3.");

	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 3, "index does not change.");
	
	*pCruiseFlagsA = CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 2, "Decrease index.");
	Assert(pRamVariables->DownshiftRpm == 5000, "RPM for index 2.");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 16.5f), "Throttle position for index 2.");

	*pCruiseFlagsA = CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 2, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 2, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 1, "Decrease index.");
	Assert(pRamVariables->DownshiftRpm == 3500, "RPM for index 1.");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 12.75f), "Throttle position for index 1.");

	*pCruiseFlagsA = CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 1, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 1, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 0, "Decrease index.");
	Assert(pRamVariables->DownshiftRpm == 2000, "RPM for index 0.");
	Assert(AreCloseEnough(*pTargetThrottlePlatePosition_Out, 8.6f), "Throttle position for index 0.");

	*pCruiseFlagsA = CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 0, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 0, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsASetCoast | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 0, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 0, "index does not change.");

	*pCruiseFlagsA = CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 1, "Increase index.");

	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 1, "No change.");

	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 1, "No change.");

	*pCruiseFlagsA = CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 2, "Increase index.");

	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 2, "No change.");

	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 2, "No change.");

	*pCruiseFlagsA = CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 3, "Increase index.");

	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 3, "No change.");

	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 3, "No change.");

	*pCruiseFlagsA = CruiseFlagsAResumeAccel | CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 3, "No change.");

	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 3, "No change.");

	*pCruiseFlagsA = CruiseFlagsAClutch | CruiseFlagsALightBrake;
	RevMatchCode();
	Assert(pRamVariables->RevMatchCalibrationIndex == 3, "No change.");

	*pCruiseFlagsA = 0;
	RevMatchCode();
	Assert(pRamVariables->RevMatchState == RevMatchEnabled, "Exit calibration mode.");
}
