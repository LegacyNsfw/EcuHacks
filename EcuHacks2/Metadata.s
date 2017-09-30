!! This file contains metadata for the ROM patching tool

		.section 	Metadata,"ax"

		.equ	RequireVersion, 0x12340000
		.equ    CalibrationId,  0x12340001
		.equ	Patch,          0x12340002
		.equ    Replace4Bytes,  0x12340003
		
		.align  4

RomPatchVersion:
		.long	RequireVersion
		.long	5

CalibrationIdentifierChange:
		.long   CalibrationId 
		.long	0x2000         !! location to find original ID and write new ID
		.long   8              !! number of characters
		.string "A2WC522N"     !! original calibration ID
		.string "\0\0\0\0\0\0" !! padding
		.string "A2WC540F"     !! new calibration ID
		.string "\0\0\0\0\0\0" !! padding
					
RevLimitHookPatch:
		.long	Patch
		.long	0x318DC        !! start address
		.long	0x31911        !! end address

RevLimitCodePatch:
		.long	Patch
		.long	_StartOfRevLimiterTables !! start address
		.long	_EndOfRevLimiterCode - 1 !! end address
	
SpeedDensityHookPatch:
		.long	Replace4Bytes
		.long   0x00007A04          !! address
		.long	_Pull2dHooked       !! old value
		.long	_ComputeMassAirFlow !! new value
		
SpeedDensityCodePatch:
		.long	Patch
		.long	_StartOfSpeedDensityTables !! start address
		.long	_EndOfSpeedDensityCode - 1 !! end address

RevMatchHookPatch:
		.long	Replace4Bytes
		.long	0x00011860			!! address
		.long	0x0003E60E			!! old value 
		.long	_RevMatchCode		!! new value

RevMatchAcceleratorHookPatch:
		.long	Replace4Bytes

!! Coordinate changes:
!! 1) old-value changes in RomAddresses.s - ReadAcceleratorPedal
!! 2) RAM address change in EcuHacks.h - pAcceleratorPedalPositionRaw

!!		.long	0x00006240          !! address (Alternate)
!!		.long   0x0000622C          !! address (Primary)
		.long   0x000113F4          !! address ProcessAcceleratorPedal		
		.long	_DefaultProcessAcceleratorPedal !! old value
		.long	_RevMatchProcessAcceleratorPedal !! new value
		
RevMatchCodePatch:
		.long	Patch
		.long	_StartOfRevMatchTables
		.long	_EndOfRevMatchCode - 1
		.end
				
		
	
		