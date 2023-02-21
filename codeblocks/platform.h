#ifndef PLATFORM_H
#define PLATFORM_H

#define HW_PLATFORM "PC"
#define SW_VERSION ":J9.0-alpha"	// must always start with ':'

#define IFS_SIZE 8192		// reserved IFS: storage size in kB (MUST BE AT LEAST 256)
#define RAM_DISK_SIZE 1024 	// RAM disk size in kB (this can be 0 if RAM disk is not required)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <time.h>
#include "..\core\rittle.h"

// alternative key codes (for platform-dependent consoles)
#define KEY_BACKSPC_ALT	0x7f
#define KEY_LEFT_ALT	((KEY_ESC<<8)+'K')
#define KEY_RIGHT_ALT	((KEY_ESC<<8)+'M')
#define KEY_UP_ALT		((KEY_ESC<<8)+'H')
#define KEY_DOWN_ALT	((KEY_ESC<<8)+'P')
#define KEY_HOME_ALT	((KEY_ESC<<8)+'G')
#define KEY_END_ALT		((KEY_ESC<<8)+'O')
#define KEY_DEL_ALT		((KEY_ESC<<8)+'S')
#define KEY_INS_ALT		((KEY_ESC<<8)+'R')
#define KEY_PGUP_ALT	((KEY_ESC<<8)+'I')
#define KEY_PGDN_ALT	((KEY_ESC<<8)+'Q')
#define KEY_F1_ALT		((KEY_ESC<<8)+';')
#define KEY_F2_ALT		((KEY_ESC<<8)+'<')
#define KEY_F3_ALT		((KEY_ESC<<8)+'=')
#define KEY_F4_ALT		((KEY_ESC<<8)+'>')

// use the standard fgets() function
// #define USE_FGETS

// internal flash disk data entry point
FILE *ifs;

// not used in this platform but required by Rittle
#define SYSCLK 1
void platform_link(void);
void init_warm(void);

// load default system options after reset
void init_cold(void);

// random number equivalent to rand()
int rnd(void);

// system reset
void sysreset(void);

// delay for specified number of microseconds
void dlyus(unsigned long us);

// read the core clock counter
// the core clock counter is a free-running counter clocked at precisely 1MHz
unsigned long cclk(void);

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_H
