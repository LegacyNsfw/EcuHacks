
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
.global _RevMatchMinimumSpeed
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

.global _RevMatchEnableFeedback
.global _RevMatchEnableCalibrationFeedback

.global _RevMatchProportionalGain
.global _RevMatchIntegralGain

_Gear1Multiplier:  .float 8.2 
_Gear2Multiplier:  .float 13.4
_Gear3Multiplier:  .float 19.7
_Gear4Multiplier:  .float 26.4
_Gear5Multiplier:  .float 33.8
_Gear6Multiplier:  .float 42.6

_MinTargetRpm:     .float 1000
_MaxTargetRpm:     .float 7000

_MinCoolantTemperature:    .float 71 !! 160F
_RevMatchMinimumSpeed:     .float 40 !! 40 kph / 25 mph
_RevMatchBuildVersion:     .float 231
_RevMatchProportionalGain: .float 0.00200 !! 0.002 = .2% throttle adjustment per 100 RPM difference
_RevMatchIntegralGain:     .float 0.00005 !! Additional throttle adjustment per 100 RPM difference per 125th/second.

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

_RevMatchEnableFeedback:            .byte 01
_RevMatchEnableCalibrationFeedback: .byte 01
_RevMatchUnusedByte1:               .byte 00
_RevMatchUnusedByte2:               .byte 00
! _RevMatchTargetOffset               .float -100.0
.end



