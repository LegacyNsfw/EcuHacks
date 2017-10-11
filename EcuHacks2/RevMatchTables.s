
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
.global _RevMatchFakeAccelerator
.global _RevMatchUnusedFloat1 !! Reserved for future use
.global _RevMatchUnusedFloat2
.global _RevMatchUnusedFloat3

.global _RevMatchDuration
.global _RevMatchAccelerationDownshiftReadyDuration
.global _RevMatchEnableDelay
.global _RevMatchCalibrationDelay

.global _RevMatchTable
.global _RevMatchInputValues
.global _RevMatchOutputValues

_Gear1Multiplier:  .float 8.2 
_Gear2Multiplier:  .float 13.4
_Gear3Multiplier:  .float 19.7
_Gear4Multiplier:  .float 26.4
_Gear5Multiplier:  .float 33.8
_Gear6Multiplier:  .float 42.6

_MinTargetRpm:     .float 1000
_MaxTargetRpm:     .float 7000

_MinCoolantTemperature: .float 71 !! 160F
_RevMatchFakeAccelerator: .float 10 !! About 20% pedal, prevents the ECU from cutting fuel
!! _RevMatchFakeAccelerator: .float 35
_RevMatchBuildVersion:    .float 101
_RevMatchUnusedFloat2:    .float 0 !! Reserved for future use
_RevMatchUnusedFloat3:    .float 0


_RevMatchDuration:           .int 250 !! 125 iterations/second, so this is 2 seconds
_RevMatchAccelerationDownshiftReadyDuration: .int 250 !! 2 seconds
_RevMatchEnableDelay:        .int 125 !! 1 second
_RevMatchCalibrationDelay:   .int 625 !! 5 seconds

_RevMatchTable: 
.byte 00
.byte 04
.byte 00
.byte 00
.long _RevMatchInputValues
.long _RevMatchOutputValues

_RevMatchInputValues:
.float 1000
.float 3000
.float 5000
.float 6500

_RevMatchOutputValues:
.float 4.5
.float 10
.float 14
.float 18

.end



