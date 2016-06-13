## Storing the wiper positions

The DS1882 has an internal EEPROM which can store the wiper positions. However, it will write to the EEPROM on all volume changes, which will quickly wear out the memory.

We therefore store the wiper positions in the Atmega328p EEPROM instead. This EEPROM is said to survive 100 000 writes. To avoid writing to the EEPROM on all volume changes, we wait until the user has let the rotary encoder untouched for 3 seconds.
At an estimate of 10 volume changes per day in average, this lets the EEPROM survive for at least 27 years.

## DS1882 address

A2, A1 and A0 on the DS1882 are tied to ground.
The address of the device therefore becomes 40 (00101000 in binary).

## BOM

All passive SMD parts are of package 1206

Electrolytic capacitors: 2x0.1uF + 1uF + 10uF. Designed for 5mm diameter (Nichicon KW/FG).
SMD capacitors: 3x0.1uF ceramic
SMD resistors: 560R, 120R, 240R, 1k1, 2x4k7, 3x10k, 3x470R
Voltage regulators: LM337 and LM317, SOT-223 packages
Diodes: 1N4002 through hole
ICs: DS1882 SOIC16 and ATmega328p TQFP-32
Male headers: 1pcs 1x5 pins, 2pcs 1x4 pins, 4pcs 1x3 pins, 1pcs 1x2 pins, 1pcs 2x3 pins

