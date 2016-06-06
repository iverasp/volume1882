## Storing the wiper positions

The DS1882 has an internal EEPROM which can store the wiper positions. However, it will write to the EEPROM on all volume changes, which will quickly wear out the memory.

We therefore store the wiper positions in the Atmega328p EEPROM instead. This EEPROM is said to survive 100 000 writes. To avoid writing to the EEPROM on all volume changes, we wait until the user has let the rotary encoder untouched for 3 seconds.
At an estimate of 10 volume changes per day in average, this lets the EEPROM survive for at least 27 years.

## DS1882 address

A2, A1 and A0 on the DS1882 are tied to ground.
The address of the device therefore becomes 40 (00101000 in binary).

