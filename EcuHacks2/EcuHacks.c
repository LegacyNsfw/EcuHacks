/******************************************************************************

Preparing to use HEW to simulate execution of ECU hacks:

1) Under the Setup menu: Simulator, System, set "Step Unit" to Instruction.
	Otherwise you don't see effects of instructions (like changes to register
	values) until one or two instructions later, which makes life harder.

2) Under the Build menu, KPIT GNUSH Toolchain, C/C++ tab:
	Pick "Object" from the dropdown, and pick Assembly.
	Pick "Optimize" from the dropdown, make sure the box is unchecked.
	(This isn't strictly required, but it makes life easier.)

3) Under the Build menu, KPIT GNUSH Toolchain, CPU tab:
	Pick CPU = SH4 Single Precision Only

4) Under Setup, Simulator, Memory, add a read/write memory resource:
	0xFFFF0000 to 0xFFFFBFFF

5) In the 'download modules' section of the project, add EcuHacks.x (the 
	project output file) to location 0.
	
6) After resetting the CPU, you'll have PC set to 0xA000:0000.  That's where
	the code in ResetHandler.s gets located.  Think of it as 'main' for testing.

*******************************************************************************

Validating hacks prior to ROM patching:

1)	Make sure all tests pass (except for tests that were designed to fail).

2)	Make sure that all patch code is correctly aligned at the desired address:
	2.1) The first instruction in RevLimitPatch must be at 0x318DC.
	2.2) The revLimitExit label in RevLimitPatch must be at 0x31912.
	2.3) TBD...

******************************************************************************/

#include "EcuHacks.h"

void SetValues() __attribute__ ((section ("Misc")));

// These are only intended for use in the debugger's watch window.
// If you use them in the actual code, you'll find that the compiler starts
// making references to global variables starting at address 0x7000:0000.

const float *_pRPM = pRPM;
const float *_pSpeed = pSpeed;
const char *_pFlagsRevLimit_0x80 = pFlagsRevLimit_0x80;
const char *_pCruiseFlagsA  = pCruiseFlagsA;

char* currentTestName;

void SetValues() 
{
	// These are just here to clarify the boundary between the prologue and the
	// 'real' code when stepping through in a debugger.
	asm("nop");
	asm("nop");

	// If you change compiler settings, inspect the way the variables get set.
	// Ideally it should take three instructions per assignment, with no 
	// references to other memory.  If this does reference other memory, patches
	// will be difficult to apply.
	
	// LC/FFS test settings
	*pRPM = 1000.0f;
	*pSpeed = 12.0f;
	*pCruiseFlagsA = 128;	
	*pFlagsRevLimit_0x80 = 0;
	
	// Speed-density test settings
	*pManifoldAbsolutePressure = 268.917649; // Corresponds to -9.5psiG, which is normal for idle.
	*pAtmosphericPressure = 700;
	
	*pMafSensorVoltage = 1.5 * MafVoltageToInternalUnits;
}

