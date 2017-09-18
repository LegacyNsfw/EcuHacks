
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Rev limiter tables start here
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

.section RomHole_RevLimiterTables,"ax"
.align 4
.global _StartOfRevLimiterTables
_StartOfRevLimiterTables:

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Cut/resume thresholds
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
.global _RedlineCut, _RedlineResume
.global _LaunchControlCut, _LaunchControlResume, 
.global _RevMatchFfsFuelCutDelta, _RevMatchFfsFuelResumeDelta
.global _FlatFootShiftSpeedThreshold

_RedlineCut:					.float 7200
_RedlineResume:					.float 7100
_RevMatchFfsFuelCutDelta:       .float 50  !! RPM offset from rev match
_RevMatchFfsFuelResumeDelta:    .float -50 !! RPM offset from rev match
_FlatFootShiftSpeedThreshold:	.float 5
_LaunchControlCut:				.float 4000
_LaunchControlResume:			.float 3999


.end



