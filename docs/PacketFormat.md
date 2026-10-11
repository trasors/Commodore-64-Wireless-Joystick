# Packet format

The packet format is borrowed heavily from the Digispark Joystick library. This makes it easy to implement Atmel based USB receivers for use with the joystick.

## Packer byte structure

|Byte             |Byte 0|Byte 1|Byte 2|Byte 3|Byte 4|Byte 5|Byte 6 |Byte 7            |
|-----------------|------|------|------|------|------|------|------|-------------------|
|Left             |0     |      |      |      |      |      |      |                   |
|Centre (default) |127   |      |      |      |      |      |      |                   |
|Right            |255   |      |      |      |      |      |      |                   |
|Up               |      |0     |      |      |      |      |      |                   |
|Centre (default) |      |127   |      |      |      |      |      |                   |
|Down             |      |255   |      |      |      |      |      |                   |
|Analog 1         |      |      |0-255 |      |      |      |      |                   |
|Analog 2         |      |      |      |0-255 |      |      |      |                   |
|Unused           |      |      |      |      |127   |      |      |                   |
|Unused           |      |      |      |      |      |127   |      |                   |
|Unused           |      |      |      |      |      |      |0     |                   |
|Buttons 1 to 8   |      |      |      |      |      |      |      |See button register|

## Button register

|Bit              |7   |6   |5   |4   |3   |2   |1   |0   |Decimal|
|-----------------|----|----|----|----|----|----|----|----|-------|
|Initial values   |0   |0   |0   |0   |0   |0   |0   |0   |0      |
|Button 1         |    |    |    |    |    |    |    |1   |1      |
|Button 2         |    |    |    |    |    |    |1   |    |2      |
|Button 3         |    |    |    |    |    |1   |    |    |4      |
|Button 4         |    |    |    |    |1   |    |    |    |8      |
|Button 5         |    |    |    |1   |    |    |    |    |16     |
|Button 6         |    |    |1   |    |    |    |    |    |32     |
|Button 7         |    |1   |    |    |    |    |    |    |64     |
|Button 8         |1   |    |    |    |    |    |    |    |128    |

The code allows multiple button presses, for example pressing Button 1 & 2 result in 0b00000011 (decimal 3) being set.
