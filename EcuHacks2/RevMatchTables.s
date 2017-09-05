
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Rev match tables start here
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

.section RomHole_RevMatchTables,"ax"
.align 4
.global _StartOfRevMatchTables
_StartOfRevMatchTables:

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Cut/resume thresholds
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

.global _Gear1Multiplier
.global _Gear2Multiplier
.global _Gear3Multiplier
.global _Gear4Multiplier
.global _Gear5Multiplier
.global _Gear6Multiplier
.global _MinTargetRpm
.global _MaxTargetRpm
.global _MinCoolantTemperature
.global _RevMatchDuration
.global _RevMatchAccelerationDownshiftReadyDuration
.global _RevMatchEnableDelay
.global _RevMatchCalibrationDelay

.global _RevMatchTable

_Gear1Multiplier:  .float 8.2 
_Gear2Multiplier:  .float 13.4
_Gear3Multiplier:  .float 19.7
_Gear4Multiplier:  .float 26.4
_Gear5Multiplier:  .float 33.8
_Gear6Multiplier:  .float 42.6

_MinTargetRpm:     .float 1000
_MaxTargetRpm:     .float 7000

_MinCoolantTemperature: .float 71 !! 160F

_RevMatchDuration:          .int 1000 !! How many counter ticks per second?
_RevMatchAccelerationDownshiftReadyDuration: .int 2000
_RevMatchEnableDelay:       .int 1000
_RevMatchCalibrationDelay:  .int 5000

_RevMatchTable: 
.byte 00
.byte 04
.byte 00
.byte 00
.long inputValues
.long outputValues

inputValues:
.float 2000
.float 3500
.float 5000
.float 6500

outputValues:
.float 8.6
.float 12.75
.float 16.5
.float 21.75

.end



