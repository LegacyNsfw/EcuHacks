.section RomHole_SpeedDensity,"ax"

!! To test, set PC to 0x6144.
!! This will step into AssignMafSensorViaScaling in the patched ROM code.
!! It should step into this code rather than Pull2d.
!! This should then step back out into the series of JSR instructions in the stock ROM code.
_ComputeMassAirFlow:

	sts.l   pr, @-r15          ! store return addy to stack
	mov.l   (pull2d), r3        ! Finish maf routine
	jsr     @r3                ! Pull MAF g/s value from scaling
	nop
	mov.l   (rpm), r0          ! Move rpm addy -> r0
	fmov    fr0, fr9           ! MAF g/s value -> fr9
	mov.l   (comp), r1         ! Move compensation variable addy -> r1
	fmov.s  @r0, fr15          ! RPM Value -> fr15
	mov.l   (map), r2          ! Move MAP addy -> r2
	fmov.s  @r1, fr14          ! compensation input value -> fr14
	mova    (engdisp), r0      ! Displacement addy -> r0
	fmov.s  @r2, fr13          ! MAP value -> fr13
	mov.l   (airtemp), r1      ! IAT addy -> r1
	fmov.s  @r0, fr12          ! Displacement value -> fr12
	mova    (c2k), r0          ! C2K addy -> r0
	fmov.s  @r1, fr11          ! IAT value -> r11
	fmov.s  @r0, fr10          ! C2K value -> fr10
	mova    (const_sd), r0     ! SD Constant addy -> r0
	fadd    fr10, fr11         ! IAT in KELVIN value -> fr11
	fmov.s  @r0, fr10          ! SD Constant value -> fr10
	fmul    fr13, fr12         ! map * eng displacement -> fr12
	mov.l   (rawflow_out), r12  ! rawflow addy -> r12
	fmul    fr15, fr12         ! map * eng displacement * rpm -> fr12
	mova    (comp_def),r0      ! Compensation table definition addy -> r0
	fmul    fr10, fr12         ! map * eng displacement * rpm * constant -> fr12
	mov.l   (pull2d), r3
	mov     r0, r4
	fdiv    fr11, fr12         ! map * eng displacement * rpm * constant -> fr12 (aka "raw airflow calc")
	jsr     @r3                ! Pull compensation table
	fmov    fr14, fr4          ! Compensation input -> fr4 for lookup
	mova    (ve_def), r0       ! VE Map LUT addy -> r4
	fmov    fr0, fr11          ! Compensation result -> fr11
	mov     r0, r4
	mov.l   (pull3d), r9       ! Pull3D Subroutine addy -> r9
	fmov    fr13, fr4          ! MAP value -> fr4 for lookup
	jsr     @r9                ! Pull VE Map!!
	fmov    fr15, fr5          ! RPM -> fr5 for lookup
	mov.l   (ve_out), r0       ! VE_out RAM addy -> r0 = RAM+0x00
	fmov.s  fr12, @r12         ! store raw airflow @ RAM+4
	fmov.s  fr0, @r0           ! store VE @ RAM+0x00
	mov.l   (comp_out), r1     ! MAP_comp RAM addy -> r1 = RAM+0x10
	fmul    fr0, fr12          ! calculate in VE to final SD VE flow
	fmov.s  fr11, @r1          ! Store Compensation @ RAM+0x10
	mov.l   (maf_out), r0      ! MAF_out RAM addy -> r0 = RAM+0x0C
	fmov.s  fr9, @r0           ! Store MAF output @ RAM+0x0C
	fmul    fr11, fr12         ! apply the compensation to sd result
	mov.l   (sd_out), r1       ! sd_out addy -> r1 = RAM+0x08
	mova    (switch), r0       ! Switch addy -> r0
	fmov.s  fr12, @r1          ! store final SD calculated airflow
                               ! Check output switch
	mov.b   @r0, r2            ! Move Switch Value to r2
	extu.b  r2, r0
	cmp/eq  #1, r0             ! Check switch
	bt/s    sd_output          ! IF TRUE (switch == 1), delayed branch to sd_output
	nop                        ! ELSE fall through to maf_output

maf_output:
	bra     end
	fmov    fr9, fr0           ! Stores MAF value to output

sd_output:
	fmov    fr12, fr0          ! Stores SD Flow to output

end:

	lds.l   @r15+, pr
	rts
	nop

	.align 4
rpm:
	.long 0xFFFF51F8           ! LGT engine RPM, 32bit float

comp:
    .long 0xFFFF2E34             ! LGT atmospheric pressure
	!!.long 0xFFFF4F34           ! LGT delta MAP.  (Need to double-check this w/ logging.)

map:
	.long 0xFFFF4F40           ! LGT manifold absolute pressure, 32bit float, mmHG - Impreza ROM contains filtered and unfiltered, no sure if that's true about A2WC522N

engdisp:
	.float 2.457               ! engine displacement (2.457 liters)

airtemp:
	.long 0xFFFF2DB8           ! LGT airtemp (should be manifold airtemp, but IAT will work for now)

c2k:
	.float 273.15              ! number to add to celsius to get kelvin (required for ideal gas law)

const_sd:      
	.float 0.003871098   ! constant needed to wrap up other conversion constants for Ideal Gas Law to ECU units of measure
         ! saves a lot of cycles, see spreadsheet for proof
         ! g/s = (Liter*mmHG*RPM / (Celsius+273) * k * VE
         ! then multiply by any other compensations desired

pull3d:
	.long 0x2150               ! LGT pull3d subroutine location (same as Impreza, go figure)

pull2d:
	.long 0x209C               ! LGT location of pull_2d subroutine (same as Impreza again)

ct:
	.long 0xFFFF503c           ! LGT coolant temp, 32bit float, degrees celsius

ve_out:
	.long 0xFFFFA000           ! LGT free memory spot to store ve (32bit float)

rawflow_out:
	.long 0xFFFFA004           ! LGT free memory spot to store intermediate raw airflow (32bit float)

sd_out:
	.long 0xFFFFA008           ! LGT free memory spot to store SD calculated airflow (32bit float)

maf_out:
	.long 0xFFFFA00C           ! LGT free memory spot to store MAF

comp_out:
	.long 0xFFFFA010           ! LGT free memory spot to store current compensation output
		
switch:
	.byte 0x00                 ! LGT free ROM space for output switch (0 = MAF, 1 = SD)

.align 4

ve_def:
	.long	ve_def_table
comp_def:
	.long	comp_def_table

! ******************** MAPS BELOW ********************

EOF: .word 0x1234

