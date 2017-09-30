! This tells the linker where to find functions that exist in the stock ROM.
! The ".global" line is needed to allow code in other source files to refer to
! the names in this file.

.global _Pull3d, _Pull2d, _Pull2dHooked, _DefaultProcessAcceleratorPedal, _FuelInjectorScaling

.equ _Pull3d,                     0x2150 
.equ _Pull2d,                     0x209C 
.equ _Pull2dHooked,               0x209C 
!! .equ _DefaultProcessAcceleratorPedal,       0xA734 !! Original ADC read (range 6-30)
!! .equ _DefaultProcessAcceleratorPedal,       0x785C !! Alternate ADC read (range 28-98)
.equ _DefaultProcessAcceleratorPedal,         0x21934 !! AssignAcceleratorPedal_4Back_1_And_2
.equ _FuelInjectorScaling,        0xc6e48
.end
