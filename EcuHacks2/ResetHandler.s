!! This is rougly equivalent to "main" but with fewer instructions to step
!! through in the simulator when testing the code.

		.section	Zero,"ax"
!! Reset handler and stack pointer
		.long _ResetHandler
		.long 0xffff2000
!! Manual reset handler and stack pointer
		.long _ResetHandler
		.long 0xffff2000
!! Various exception handlers - this isn't all of them but it should suffice.
		.long IllegalInstructionHandler
		.long Reserved5Handler
		.long SlotIllegalInstructionHandler
		.long Reserved7Handler
		.long Reserved8Handler
		.long CpuAddressErrorHandler
		.long DmacAddressErrorHandler
		.long NmiHandler
		.long BreakHandler
		.long FpuExceptionHandler
		.long HudiHandler

		.section 	RSTHandler,"ax"
_ResetHandler:
				mov.l	Stack,r15
				mov.l	SetValues,r0
				jsr		@r0
				nop
				
				mov.l	DemonstrateAssertionFailure, r0
				jsr		@r0
				nop
				
				mov.l	RevLimiterUnitTests,r0			
				jsr		@r0			
				nop
				
! Only useful when testing with a patched ROM				
				mov.l	SpeedDensityUnitTests,r0
				jsr		@r0			
				nop

				mov.l	RevMatchStateUnitTests,r0
				jsr		@r0
				nop

				mov.l	RevMatchUnitTests,r0
				jsr		@r0
				nop

				mov.l	RevMatchCalibrationUnitTests,r0
				jsr		@r0
				nop
				
				mov.l	RevMatchCounterTests,r0
				jsr		@r0
				nop

!! Only useful when testing with a patched ROM.
!! This jumps into ECU code, which will call the function whose Pull2D method
!! was hijacked for the speed density hack.  You should see it step into the
!! ComputeMassAirFlow function, which returns a MAF value in fr0, then step 
!! out of the method whose Pull2d call was hijacked, then step into a series 
!! of other subroutines in the stock ECU code.		
 				mov.l	SpeedDensityIntegrationTest,r0
				jsr		@r0			
				nop

!! Only useful when testing with a patched ROM.
!! This jumps into a series of JSR instructions in the ECU code.
!! The first will look up the throttle plate angle based on the
!! DBW tables. The next is the hijacked jump into the rev match
!! code. The next is a functiont that applies a low-pass filter
!! to the throttle plate angle.
 				mov.l	RevMatchIntegrationTest,r0
				jsr		@r0			
				nop

				mov.l	GenericTests,r0
				jsr		@r0							
				nop

				mov.l	GetRevLimiterTableInfo,r0
				jsr		@r0							
				nop
				
				mov.l	GetSpeedDensityTableInfo,r0
				jsr		@r0							
				nop
				
				mov.l	GetRevMatchTableInfo,r0
				jsr		@r0
				nop
				
.stop:
				bra		.stop
				nop								
		.align 4

Stack:
		.long	0xFFFF1000
		
!! You can't just "mov.l _SetValues,r0" (or any other function) since the
!! functions all live in other segments.  You get "pcrel too far" errors.
SetValues:
		.long	_SetValues
		
DemonstrateAssertionFailure:
		.long	_DemonstrateAssertionFailure
		
RevLimiterUnitTests:
		.long	_RevLimiterUnitTests

SpeedDensityUnitTests:
		.long   _SpeedDensityUnitTests

RevMatchStateUnitTests:
		.long	_RevMatchStateUnitTests

RevMatchUnitTests:
		.long	_RevMatchUnitTests

RevMatchCalibrationUnitTests:
		.long	_RevMatchCalibrationUnitTests

RevMatchCounterTests:
		.long	_RevMatchCounterTests
								
SpeedDensityIntegrationTest:
		.long   0x6144 

RevMatchIntegrationTest:
		.long   0x11696 

GetSpeedDensityTableInfo:
		.long	_GetSpeedDensityTableInfo	

GetRevLimiterTableInfo:
		.long	_GetRevLimiterTableInfo	

GetRevMatchTableInfo:
		.long	_GetRevMatchTableInfo	

GenericTests:
		.long	_GenericTests	

!! Handlers for various exceptions and interrupts.  They do nothing, but 
!! at least you can know which exception or interrupt was triggered.
IllegalInstructionHandler:
		rte
		nop
		
Reserved5Handler:
		rte
		nop
		
SlotIllegalInstructionHandler:
		rte
		nop
		
Reserved7Handler:
		rte
		nop
		
Reserved8Handler:
		rte
		nop
		
CpuAddressErrorHandler:
		rte
		nop
		
DmacAddressErrorHandler:
		rte
		nop
		
NmiHandler:
		rte
		nop
		
BreakHandler:
		rte
		nop
		
FpuExceptionHandler:
		rte
		nop

HudiHandler:
		rte
		nop

		.end
		
		