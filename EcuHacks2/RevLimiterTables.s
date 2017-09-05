
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
.global _FlatFootShiftCut, _FlatFootShiftResume,
.global _FlatFootShiftSpeedThreshold

_RedlineCut:					.float 7200
_RedlineResume:					.float 7100
_FlatFootShiftCut:				.float 5000
_FlatFootShiftResume:			.float 4500
_FlatFootShiftSpeedThreshold:	.float 5
_LaunchControlCut:				.float 4000
_LaunchControlResume:			.float 3999


.end



