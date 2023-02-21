#ifndef LCD4_H
#define LCD4_H

// HD44780-compatible LCD panels

#ifdef __cplusplus
extern "C" {
#endif

#include "..\platform.h"

#define LCD4_RS		LATFbits.LATF0
#define LCD4_EN		LATFbits.LATF1
#define LCD4_D4		LATEbits.LATE0
#define LCD4_D5		LATEbits.LATE1
#define LCD4_D6		LATEbits.LATE2
#define LCD4_D7		LATEbits.LATE3
#define LCD4_RW		LATEbits.LATE4	// not used for now

extern unsigned char lcd4_lockf;	// scroll lock flag

void lcd4_Attach(void);

void __LCD4(void);

#ifdef __cplusplus
}
#endif

#endif // LCD4_H
