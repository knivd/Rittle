
Welcome!
========

Thank you for giving Rittle a try!
Please note, Rittle is still in development, so the current version is likely not a final release. 
For any bugs or other feedback please feel free to email me at knivd@me.com


Requirements
============

The "RITTLE Board" or similar with the PIC32MZ2048EFH064-250 microcontroller in TQPF64 or QFN64 package.

Other PIC32MZ2048EF*064 chips (where other letter replaces the star) will most provably work fine too. The "-250" suffix only applies to those chips that will support the rittle's "Turbo" mode with 256 MHz clock. The other variations that don't have the "-250" suffix will work fine but without support of 256 MHz clock mode.


Programming the firmware
========================

Use PickIt3, ICD3, or any other similar programmer. The wiring of a 64-pin chip (TQFP or QFN) is listed below:

--+ 1                  +----------------------+
  |--- MCLR -----------| MCLR (pin 9)       P |
I |--- Vdd ------------| Vdd supply pins    I |
C |--- GND ------------| GND supply pins    C |
D |--- Data -----------| PGED2 (pin 18)     3 |
  |--- Clock ----------| PGEC2 (pin 17)     2 |
--+ 5                  +----------------------+

For minimal working configuration of the PIC32MZ microcontroller, please refer to page 40 the datasheet: https://www.microchip.com/wwwproducts/en/PIC32MZ2048EFH064


Getting the Rittle console running
==================================

A serial terminal software will be required. Ideally, one that supports V100 escape codes. During development Rittle has been tested with Putty: https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html

Serial console is available on the USB of the PIC32 microcontroller, or on pins 17 (data receive) and 18 (data transmit) with protocol 115200bps, 8 bits data word, No parity, 1 stop bit.

