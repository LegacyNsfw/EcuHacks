!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!! Markers for end of various code segments.  Metadata.s refers to these.
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

.section RomHole_SpeedDensityEndMarker,"ax"
.align 4
.global _EndOfSpeedDensityCode
_EndOfSpeedDensityCode: .long 0x12341234
						.long 0x01010101

.section RomHole_RevLimiterEndMarker,"ax"
.align 4
.global _EndOfRevLimiterCode
_EndOfRevLimiterCode:	.long 0x12341234
						.long 0x02020202

.section RomHole_RevMatchEndMarker,"ax"
.align 4
.global _EndOfRevMatchCode
_EndOfRevMatchCode:     .long 0x12341234
						.long 0x03030303

.end



