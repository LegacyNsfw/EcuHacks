#include "EcuHacks.h"

// All test functions must be explicitly put into the Misc section, otherwise
// the compiler/linker will put them in an address range that conflicts with
// ECU code.

// Sets or clears the 0x80 bit of the 'cruise flags A' ECU variable.
void SetClutch(int value) __attribute__ ((section ("Misc")));
void SetClutch(int value)
{
	if (value == 0)
	{
		*pCruiseFlagsA = 0x7F;
	}
	else
	{
		*pCruiseFlagsA = 0xFF;
	}
}

// Gets the 0x80 bit of the 'rev limit flags' ECU variable.
int GetFuelCutFlag() __attribute__ ((section ("Misc")));
int GetFuelCutFlag()
{
	// The SH2E compiler implements this correctly.
	char result = *pFlagsRevLimit_0x80 & (char)0x80;
	return result;
	
#if SH4_Compiler_Bug	
	// All of these will run afoul of an SH4 compiler bug, which corrupts the stack.
	// When the caller returns, PC is given a random value.
	//
	// char result = *pFlagsRevLimit_0x80 & (char)0x80;
	// char result = *((char*)pFlagsRevLimit_0x80) & (char)0x80;
	// char flags = *pFlagsRevLimit_0x80;
	// flags = flags & (char)0x80;

	// This works around the compiler bug.  The fact that this works at all is
	// probably another bug.  See the final two test cases for NON-buggy code 
	// that verifies proper setting of the rev limit flag.

	int flags = *((unsigned*)pFlagsRevLimit_0x80);
	flags >>= 24;
	
	if ((flags & 0x80) == 0x80)
	{		
		return 1;
	}
	else
	{
		return 0;
	}
#endif	
}

// Test the rev limiter hack.
void RevLimiterUnitTests() __attribute__ ((section ("Misc")));
void RevLimiterUnitTests()
{
	pRamVariables->UpshiftRpm = 6500;
	pRamVariables->RevMatchState = RevMatchDisabled;
	
	const float FfsCut = 6550;
	const float FfsResume = 6450;
	
	*pThrottlePedal = 95.0f;
	*pRPM = LaunchControlCut + 1000;
	*pSpeed = 0.0f;
	SetClutch(0);
	RevLimitPatch();
	Assert(!GetFuelCutFlag(), "Normal stopped: Allow fuel at LaunchControlCut + 1000 RPM, stopped, no clutch");
	
	*pRPM = FfsCut + 10;
	*pSpeed = 0.0f;
	SetClutch(0);
	RevLimitPatch();
	Assert(!GetFuelCutFlag(), "Normal stopped: Allow fuel at FlatFootShiftCut + 1000 RPM, stopped, no clutch");

	*pRPM = LaunchControlCut + 1000;
	*pSpeed = 20.0f;
	SetClutch(0);
	RevLimitPatch();
	Assert(!GetFuelCutFlag(), "Normal moving: Allow fuel at LaunchControlCut + 1000 RPM, moving, no clutch");
	
	*pRPM = FfsCut + 10;
	*pSpeed = 20.0f;
	SetClutch(0);
	RevLimitPatch();
	Assert(!GetFuelCutFlag(), "Normal moving: Allow fuel at FlatFootShiftCut + 1000 RPM, moving, no clutch");
	
	*pRPM = LaunchControlCut - 1;
	*pSpeed = 0.0f;
	SetClutch(1);
	RevLimitPatch();
	Assert(!GetFuelCutFlag(), "Launch Control: Allow fuel at LaunchControlCut - 1 RPM, standstill, clutch pressed");
	
	*pRPM = LaunchControlCut + 1;
	*pSpeed = 0.0f;
	SetClutch(1);
	RevLimitPatch();
	Assert(GetFuelCutFlag(), "Launch Control: Cut fuel at LaunchControlCut + 1 RPM, standstill, clutch pressed");
	
	*pRPM = LaunchControlResume - 1;
	*pSpeed = 0.0f;
	SetClutch(1);
	RevLimitPatch();
	Assert(!GetFuelCutFlag(), "Launch Control: Resume fuel at LaunchControlResume - 1 RPM, standstill, clutch pressed");
	
	*pRPM = FfsCut - 1;
	*pSpeed = 20.0f;
	SetClutch(1);
	RevLimitPatch();
	Assert(!GetFuelCutFlag(), "Flat Foot Shifting: Allow fuel at FlatFootShiftCut - 1 RPM, moving, clutch pressed");
	
	*pRPM = FfsCut + 1;
	*pSpeed = 20.0f;
	SetClutch(1);
	RevLimitPatch();
	Assert(GetFuelCutFlag(), "Flat Foot Shifting: Cut fuel at FlatFootShiftCut + 1 RPM, moving, clutch pressed");	

	*pRPM = FfsResume - 1;
	*pSpeed = 20.0f;
	SetClutch(1);
	RevLimitPatch();
	Assert(!GetFuelCutFlag(), "Flat Foot Shifting: Resume fuel at FlatFootShiftResume - 1 RPM, moving, clutch pressed");

	*pRPM = RedlineCut - 1;
	*pSpeed = 20.0f;
	SetClutch(0);
	RevLimitPatch();
	Assert(!GetFuelCutFlag(), "Redline: Allow fuel at RedlineCut - 1 RPM, moving, clutch not pressed");
	
	*pRPM = RedlineCut + 1;
	*pSpeed = 20.0f;
	SetClutch(0);
	RevLimitPatch();
	Assert(GetFuelCutFlag(), "Redline: Cut fuel at RedlineCut + 1 RPM, moving, clutch not pressed");	

	*pRPM = RedlineResume - 1;
	*pSpeed = 20.0f;
	SetClutch(0);
	RevLimitPatch();
	Assert(!GetFuelCutFlag(), "Redline: Resume fuel at RedlineResume - 1 RPM, moving, clutch not pressed");
	
	// Verify the upper-limit sanity check
	pRamVariables->UpshiftRpm = 9000;

	*pRPM = RedlineCut + 1;
	*pSpeed = 20.0f;
	SetClutch(1);
	RevLimitPatch();
	Assert(GetFuelCutFlag(), "Flat Foot Shifting: Cut fuel at RedlineCut + 1 RPM, when rev match cut is too high");	
	
	// Verify the other bits in the rev limiter flag are not modified.
	*pRPM = 6000.0f;
	*pSpeed = 0.0f;
	SetClutch(1);
	*pFlagsRevLimit_0x80 = 0x00;
	RevLimitPatch();
	char flags = *pFlagsRevLimit_0x80;
	int comparison = flags == (char) 0x80;
	Assert(comparison, "When rev limit flag is set, no other bits are set.");

	// Verify the other bits in the rev limiter flag are not modified.
	*pRPM = 1000.0f;
	*pSpeed = 0.0f;
	SetClutch(0);
	*pFlagsRevLimit_0x80 = 0xFF;
	RevLimitPatch();
	flags = *pFlagsRevLimit_0x80;
	comparison = flags == (char) 0x7F;
	Assert(comparison, "When rev limit flag is cleared, no other bits are cleared.");	
	
	// Sanity check the defaults
	Assert(RedlineCut > RedlineResume, "Redline cut/resume sanity.");
	Assert(LaunchControlCut > LaunchControlResume, "LaunchControl cut/resume sanity.");
	Assert(RevMatchFfsFuelCutDelta > 0, "FlatFootShift cut sanity.");
	Assert(RevMatchFfsFuelResumeDelta < 0, "FlatFootShift resume sanity.");
}

