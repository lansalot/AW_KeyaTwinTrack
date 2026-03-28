CAN commands for motor control:
Enable: 23 0D 20 01 00 00 00 00
Returned ID: 0x0580000+ motor ID (hexadecimal)
Data: 60 0D 20 00 00 00 00 00

Disable: 23 0C 20 01 00 00 00 00
Returned ID: 0x0580000 + motor ID (hexadecimal)
Data: 60 0C 20 00 00 00 00 00

Speed control: 23 00 20 01 DATA_L(H) DATA_L(L) DATA_H(H) DATA_H(L)
Speed control: 23 00 20 01 03 E8 00 00 Speed 100RPM (0x03E8=1000)
Returned ID: 0x0580000 + motor ID (hexadecimal)
Data: 60 00 20 00 00 00 00 00

Position control: 23 02 20 01 DATA_L(H) DATA_L(L) DATA_H(H) DATA_H(L)
Position control: 23 02 20 01 27 10 00 00 Rotate 360°counterclockwise
Returned ID: 0x0580000+ motor ID (hexadecimal)
Data: 60 02 20 00 00 00 00 00


If the given speed +50 (rated speed 100)
Control command ID: 0x06000001 (extended ID)
Enable: 23 0D 20 01 00 00 00 00
Given speed: 23 00 20 01 01 F4 00 00 (0x01F4 = 500)
 If the given speed -50 (rated speed 100)
Control command ID: 0x06000001 (extended ID)
Enable: 23 0D 20 01 00 00 00 00
Given speed: 23 00 20 01 FE 0C FF FF


Position control:
The given position value - 50000~ + 50000 means 5 circles clockwise ~ 5 circles counterclockwise
(0x3CB0 FFFF) (0XC350 0000)
The software setting control mode is CAN control (0019 is set to 2)
The software setting control mode is absolute position control (0020 is set to 3)
Or the software setting control mode is set to relative position control (0020 is set to 4)


We are using absolute position control