#include "EcuHacks.h"

// All test functions must be explicitly put into the Misc section, otherwise
// the compiler/linker will put them in an address range that conflicts with
// ECU code.

float CallSpeedDensityHook() __attribute__ ((section ("Misc")));
float CallSpeedDensityHook()
{
	void (*func)() = (void(*)()) 0x79A8;
	(*func)(*pMafSensorVoltage);
	
	return *pMassAirFlow;
}

// Unit tests for the speed density hack.
void SpeedDensityUnitTests() __attribute__ ((section ("Misc")));
void SpeedDensityUnitTests()
{
	float expectedMafFromSensor = 3.3727;
	float expectedMafFromSpeedDensity = 3.0557;
		
	// Pretend the ECU has just booted, so MafMode is not initialized.
	pRamVariables->MafMode = 0;
	*pManifoldAbsolutePressure = 268.917649; // Corresponds to -9.5psiG, which is normal for idle.	
	*pAtmosphericPressure = 700;
	*pIntakeAirTemperature = 15;
	*pMafSensorVoltage = (short) (1.25f * MafVoltageToInternalUnits);
	
	float result = CallSpeedDensityHook();	
	Assert(AreCloseEnough(result, expectedMafFromSensor), "First execution should return MAF from sensor.");
	Assert(pRamVariables->MafMode == MafModeSensor, "First execution should set MafMode to MafModeSensor.");
	
	// Verify that we get the same result again.
	result = CallSpeedDensityHook();	
	Assert(AreCloseEnough(result, expectedMafFromSensor), "Second execution should return MAF from sensor.");
	Assert(pRamVariables->MafMode == MafModeSensor, "MafMode should remain set to MafModeSensor.");

	pRamVariables->MafMode = MafModeSpeedDensity;
	result = CallSpeedDensityHook();	
	Assert(AreCloseEnough(result, expectedMafFromSpeedDensity), "Second execution should return MAF from speed-density.");
	Assert(pRamVariables->MafMode == MafModeSpeedDensity, "MafMode should remain set to MafModeSpeedDensity.");
}

