! This tells the linker where to find functions that exist in the stock ROM.
! The ".global" line is needed to allow code in other source files to refer to
! the names in this file.

.global _Pull3d, _Pull2d, _Pull2dHooked, _ReadAcceleratorPedal, _FuelInjectorScaling

.equ _Pull3d,                     0x2150 
.equ _Pull2d,                     0x209C 
.equ _Pull2dHooked,               0x209C 
.equ _ReadAcceleratorPedal,       0x785C
.equ _FuelInjectorScaling,        0xc6e48
.end
