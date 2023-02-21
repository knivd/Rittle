/*

RITTLE programming language and operating environment

(C) 2017 - 2020, Konstantin Dimitrov, knivd@me.com

Redistribution and use in source and binary forms, with or without modification, are permitted provided
that the following conditions are met:
    1. Redistributions of source code must retain the above copyright notice, this list of conditions and
		the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
		and the following disclaimer in the documentation and/or other materials provided with the distribution.
    3. Neither the name of Konstantin Dimitrov nor the names of other contributors may be used to endorse
		or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.

IN NO EVENT SHALL KONSTANTIN DIMITROV BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef RITTLE_H
#define RITTLE_H

#ifdef __cplusplus
extern "C" {
#endif

// =================================================================================================


#if defined(CODEBLOCKS)
	#include "..\codeblocks\platform.h"

#elif defined(PIC32MZEF)
    #include "..\pic32mzef\platform.h"

#else
	#warning "missing HW_PLATFORM (using generic Rittle)"
	#define HW_PLATFORM "generic"		// this is the default platform when there is no other defined

#endif

#ifndef SW_VERSION
	#define SW_VERSION	":undefined"	// must always start with ':'
#endif


// =================================================================================================

// some important constants (for RSC and RVM)
#define MAX_STACK 50		// maximum VM stack depth
#define MAX_NESTED 50		// defines the maximum number of nested features (loops, calls, ifs)
#define MAX_LEVEL 5			// maximum nesting level for compilation
#define MAX_TKEXPR 150		// defines the maximum number of tokens in an expression
#define MAX_ID_LENGTH 50	// maximum length of an identifier (name of variable or function)
#define MAX_FN_LENGTH 250	// maximum file name length for an include file
#define MAX_DIMENSIONS 10	// maximum number of array dimensions
#define MAX_PARAMS 20		// maximum input or output parameters in a function
#define MAX_FILES 5         // maximum number of open files/devices
#define MAX_THREADS 5		// maximum extra parallel threads

// Definitions of physical drive number for each drive
#define DEV_NUL 0			// NULL device
#define DEV_IFS 1			// internal flash disk
#define DEV_RAM 2			// small internal RAM disk for data exchange between files
#define DEV_SD1 3			// external SD card 1
#define DEV_SD2 4			// external SD card 2

// bit function macro
#define BIT(x) (1ULL<<(x))

// console prompt
#define PROMPT "_"

// space representation of a single tab in the RIDE
#define TAB "   "

// use the standard fgets() function instead of the Rittle one
//#define USE_FGETS

// basic keyboard codes
#define KEY_BREAK	0x03	// Ctrl-C used for terminating execution
#define KEY_ENTER   '\r'
#define KEY_TAB     '\t'
#define KEY_BACKSPC	'\b'
#define KEY_NLINE	'\n'
#define KEY_BEEP	'\a'
#define KEY_ESC     '\e'	// 0x1b
#define KEY_FFEED	0x0b
#define KEY_CLREOL	0x1a

// VT100-compatible key codes
#define ESC_SEQ		((KEY_ESC<<8)+'[')	// begin of an Escape sequence
#define KEY_LEFT	((ESC_SEQ<<8)+'D')
#define KEY_RIGHT	((ESC_SEQ<<8)+'C')
#define KEY_UP		((ESC_SEQ<<8)+'A')
#define KEY_DOWN	((ESC_SEQ<<8)+'B')
#define KEY_HOME	((ESC_SEQ<<16)+('1'<<8)+'~')
#define KEY_END		((ESC_SEQ<<16)+('4'<<8)+'~')
#define KEY_DEL		((ESC_SEQ<<16)+('3'<<8)+'~')
#define KEY_INS		((ESC_SEQ<<16)+('2'<<8)+'~')
#define KEY_PGUP	((ESC_SEQ<<16)+('5'<<8)+'~')
#define KEY_PGDN	((ESC_SEQ<<16)+('6'<<8)+'~')
#define KEY_F1		((ESC_SEQ<<16)+('O'<<8)+'P')
#define KEY_F2		((ESC_SEQ<<16)+('O'<<8)+'Q')
#define KEY_F3		((ESC_SEQ<<16)+('O'<<8)+'R')
#define KEY_F4		((ESC_SEQ<<16)+('O'<<8)+'S')

// data type definitions
#define rINVALID	0x00
#define rUNIT		0x01
#define _rTEXT		0x02
// IMPORTANT the following types MUST correspond to their relevant token codes
#define rANY		0x10
#define rSINT8		0x12
#define rSINT16		0x14
#define rSINT32		0x16
#define rSINT64		0x18
#define rREAL		0x1c
#define rTEXT		0x1e
#define rFUNC		0x1f

#define rtype_t 	unsigned char

// interface roles; SOURCE applies only to text from the input source
#define rrNONE		0
#define rrSOURCE	1
#define rrINPUT		2
#define rrOUTPUT	3
#define rrREFER		4

#define rrole_t 	unsigned char

// universal data structure
typedef struct {
	union {
		signed long long sint64;
		double real;
		struct {
			char *text;			// points to pre-allocated data
			unsigned long tlen;	// length of text
		};
	} data;
    rtype_t type;	// data type
    rrole_t role;	// interface role (reserved field in the RVM)
} rdata_t;

// language token structure
typedef struct {
	const char *name;		// name in text form
	unsigned char level;	// precedence level - greater number is higher level
	signed char inc;		// additional address increment after execution (depends on the length of the data)
							// -1 is a special case for tokens with variable length of the following data
	signed char params;		// number of required parameters
							// value -1 indicates that the number can be variable; everything until the end of statement
							// or closing bracket is taken in that case
							// NOTE: operators have this value set as 0
							// NOTE: tokens with -1 params should have the highest level (12)
	void (*exec)(void);		// handler function for execution
							// functions return error codes by modifying the global variable (xv)
} rtoken_t;

extern const rtoken_t tokens[256];	// main token table
char conbuf[256];                   // console input buffer; this size also determines the size of several other buffers
unsigned char fake[8];              // a tiny buffer used for speculative compilation

// Code Blocks uses different name for LONG_LONG and ULONG_LONG constants
#ifndef LONG_LONG
	#define LONG_LONG 		LLONG
#endif
#ifndef LONG_LONG_MIN
	#define LONG_LONG_MIN 	LLONG_MIN
#endif
#ifndef LONG_LONG_MAX
	#define LONG_LONG_MAX 	LLONG_MAX
#endif
#ifndef ULONG_LONG
	#define ULONG_LONG 		ULLONG
#endif
#ifndef ULONG_LONG_MIN
	#define ULONG_LONG_MIN 	ULLONG_MIN
#endif
#ifndef ULONG_LONG_MAX
	#define ULONG_LONG_MAX 	ULLONG_MAX
#endif

// 'exit value': global compilation/execution code
// must be cleared before calling compilation or VM execution functions
// refer to xvline() in the file "rittle.c" for return codes and their explanation
signed long xv;

// number of currently compiled source line
unsigned long rscline;

// the start of the currently compiles source line in RSC
char *rscsrc;

// RAM disk data entry point
unsigned char *ramdisk;

#include <conio.h>	// kbhit() and getch()
#include <limits.h>
#include "xmem.h"
#include "fatfs/source/ff.h"
#include "rsc.h"
#include "ride.h"
#include "rvm.h"

FATFS FatFs;	// work area for FatFs

// main entry to the Rittle console and user interface
// this is the only function that the external environment need to call to invoke the Rittle
void rittle(void);

// compile and return error code (negative number) or total compiled code length (positive number or 0)
// (level) must be 0 when the function is called from outside
// (scf) is a "speculative compilation flag" - performs compilation without writing into the destination
//				the destination however still needs to point to a valid memory address
//				this can be used to estimate the size of a compiled code before performing an actual compilation
// (dbginfo) is a flag specifying whether debug commentaries should be included in the output code
signed long compile(char **source, unsigned char **destination, unsigned short level, char scf, char dbginfo);

// initialise the RVM
// *debugfunc is a callback function used for debugging; its result may update the value of (xv)
void initRVM(unsigned char *code, unsigned long length, void *debug(void));

// execute and return error code
// NOTE: initRVM() must be executed before first run
// 			negative number: error code
// 			0: no error
//			1: execution terminated normally
//			2: execution terminated by a break instruction
// (mode) defines the way of execution
//			0: execute with no breakpoints (normal executable)
//			1: execute with breakpoints
//			2: single step of all threads
//			3: single step of single thread
signed long execute(unsigned char *code, unsigned long length, char mode);

// list compiled code
// (addr) is a virtual address from where to start then counting
void list(unsigned long addr, unsigned char *code, unsigned long length);

// show error line and error code text
void xvline(char *source);

// initialise storage drive and return error code (negative) or size in bytes (positive)
// (*dev) device identifier
signed long long ffinit(char *dev);

// *** (internal function)
// skip whitespace characters in source
// (preinc) specifies the minimum number of characters that definitely need to be skipped
void skip(char **source, signed long preinc);

// *** (internal function)
// return token code by given its text name, or return 0 if not found
unsigned char token(char *s);

// *** (internal function)
// determine the best integer data type based on the value of (v)
void cast(void *v, rdata_t *c);

// *** (internal function)
// a backup plan since the function round() is not available in the XC32 compiler
double xround(double v);

// text line editor
// (*buf) text buffer
// (bufsz) maximum size of the text buffer including the terminating 0 character
// (x) initial position of the cursor (starting from 0)
// output
// (*buf) text after editing
void line_edit(char *prompt, char *buf, unsigned int bufsz, int x);

#ifdef __cplusplus
}
#endif

#endif // RITTLE_H
