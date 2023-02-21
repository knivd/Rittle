#ifndef SSD196_H
#define SSD196_H

// SSD196x controllers on 8-bit bus

// NOTE: some parts in this code have been borrowed and adapted from the Micromite

#ifdef __cplusplus
extern "C" {
#endif

#include "..\platform.h"

#define LCD_SSD_CS		LATEbits.LATE4		// pin 64
#define LCD_SSD_DC		LATEbits.LATE1		// pin 61
#define LCD_SSD_RES		LATEbits.LATE0		// pin 58

void lcdSSD196_Attach(void);

#ifdef __cplusplus
}
#endif

#endif // SSD196_H
