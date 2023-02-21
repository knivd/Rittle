#ifndef RVM_H
#define RVM_H

// RITTLE VIRTUAL MACHINE

#ifdef __cplusplus
extern "C" {
#endif

#include "rittle.h"

#define E  2.7182818284590452353602874713527
#define PI 3.1415926535897932384626433832795

// variable flag definitions
#define vfCONST		0b00000001	// the data is constant in code memory
#define vfMORPH		0b00000010	// can morph into any data type - 0:no, 1:yes
#define vfREFER		0b00000100	// indicates that the field (addr) contains address to another variable

// single variable structure for the virtual machine
typedef struct {
	void *next;					// next record in the chain or NULL if last
	unsigned short vid;			// variable id
	rtype_t type;				// data type
	unsigned char flags;		// additional flags
	unsigned long len;			// byte length of a single data element (for TEXT this is the maximum length of a string)
	void *addr;					// pointer to physical data; normally NULL unless data is at fixed address or a reference
    unsigned long dim[MAX_DIMENSIONS];	// number of elements in arrays (0 for discrete variables)
} vmvar_t;						// (actual data follows immediately after the structure)

// runtime process structures
typedef struct {
	unsigned long entry;		// entry point for execution
	unsigned long pc;			// program address counter
	unsigned short dsp;	    	// data stack pointer
	rdata_t stack[MAX_STACK];	// data stack
	unsigned short plp;	    	// parameter list pointer
	rdata_t params[MAX_STACK];	// function parameter FIFO
	unsigned short csp;	    	// call stack pointer
	unsigned long callst[MAX_NESTED];	// call return address stack
	unsigned long ix;			// calculated current flat index for arrays
	vmvar_t *vmv;				// run-time variables
	vmvar_t *local[MAX_NESTED];	// local variables
	unsigned short lvidx;		// index for the local variable stack
	signed long dlycntr;		// non-blocking delay counter (microseconds)
	signed long long tickval;	// tick value
	signed long long tickcntr;	// current tick counter (taken from the uptime counter)
	unsigned long tickcall;		// function address for the tick call
	DIR dir;					// directory information used by ffirst/fnext
	FILINFO finfo;				// file information used by ffirst/fnext
} rvm_t;

rdata_t xres;                   // the top of the stack value is always returned here after RVM
                                // finished execution
                                // if the entry value of (xres) is
                                // .role==rrINPUT
                                // .type==rFUNC
                                // .data.sint64==-1
                                // then the RVM will not output anything to the console
                                // the role will be set by the RVM to rrOUTPUT if the console
                                // has been disabled, otherwise the role will be set to rrNONE
unsigned char *mem;             // pointer to the program code
unsigned long mlen;		    	// program code length in bytes
unsigned char threads;			// active threads parallel to the main one (thread 0)
unsigned char thd;				// current thread
unsigned char enbrk;			// enable Ctrl-C user break
unsigned char enxcon;			// enable external console on attached display driver
rvm_t *rvm[1+MAX_THREADS];      // runtime process structures

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

// get a positive integer number from the source or return -1 if can't get any number
signed long long getnum(char **c);

// call RVM function at given address within the current thread
// (addr) - entry point of execution
// (rtnaddr) - return address after exiting the function
void call_addr(unsigned long addr, unsigned long rtnaddr);

// copy file
// (*fn) file name in the current drive
// (*path) path and file name for the new location
// result is 0 if successful, or negative in case of a failure
signed long long fcopy(char *fn, char *path);

// pull signed integer from the stack
void pulls(rdata_t *d);

// pull unsigned integer from the stack
void pullu(rdata_t *d);

// pull real number from the stack
void pullr(rdata_t *d);

// pull text from the stack
void pullt(rdata_t *d);

// pull data from the stack in FIFO order
// IMPORTANT: returns the basic container types only
void pullfifo(rdata_t *d);

// push data to the stack
void push(rdata_t *d);

// RVM invalidation functions
void _invalid_(void);
void _unsupported_(void);
void __nop(void);

// top level interface variables and primitive drivers ======================

// font definition
typedef struct {
	unsigned short start;		// code of the first character in the font
	unsigned short characters;	// number of character definitions in the font
	unsigned char width;		// font width
								// This parameter specifies the number of columns in the characters.
								// In fonts where the field (.width) is 0, every character definition
								// starts with a byte that defines how many columns are present in
								// this character. The actual number of bytes to follow depend on the
								// height of the font as well: for fonts with height 8 lines or less,
								// every byte represents one column, for fonts with height 16 lines
								// or less, every column takes two bytes, and so on
								// in fonts with fixed width where (.width) is greater than 0, the
								// leading width-specifying byte in every definition is missing since
								// the width is already know for all characters
	unsigned char height;		// character definition height in pixels
								// This parameter also automatically defines the number of
								// bytes needed for one column of the character
	unsigned char blankL;		// number of blank columns to add on the left side of every character
	unsigned char blankR;		// number of blank columns to add on the right side of every character
	unsigned char blankU;		// number of blank rows to add on the top side of every character
	unsigned char blankD;		// number of blank rows to add on the bottom side of every character
	char *name;					// optional font name as ASCIIZ string
} font_header_t;

typedef struct {
	font_header_t header;
	unsigned char definitions[];
} font_t;

// orientation constants
#define LANDSCAPE	1
#define PORTRAIT	2
#define RLANDSCAPE	3
#define RPORTRAIT	4

// direction constants (using clock small arrow directions)
#define DIR_0000	0
#define DIR_0130	1
#define DIR_0300	2
#define DIR_0430	3
#define DIR_0600	4
#define DIR_0730	5
#define DIR_0900	6
#define DIR_1030	7

// display parameters and variables
extern signed int resH, resV;			// screen resolution
extern signed char orientation;			// screen orientation
extern signed int posX, posY;			// virtual cursor position
extern const font_t *font;				// current font table
extern unsigned char fontScale;			// scaling factor for the font printing
extern signed long fontFcol, fontBcol;	// current front colour and back colour for fonts

// default system font 5x8 pixels
extern const font_t sysFont0508;

// touch interface variables
#define MAX_TOUCH_POINTS	10

extern volatile unsigned char touch_points;
extern volatile signed int touch_x[MAX_TOUCH_POINTS];
extern volatile signed int touch_y[MAX_TOUCH_POINTS];
extern volatile signed int touch_p[MAX_TOUCH_POINTS];

// touch interface primitives
extern void (*touch_InitPort)(void);
extern void (*touch_Detach)(void);
extern void (*touch_ReadTouch)(void);

// external driver primitives
extern void (*display_InitPort)(void);
extern void (*display_Detach)(void);
extern void (*display_Cls)(void);
extern void (*display_PutChr)(int ch);
extern void (*display_ScrollUp)(void);
extern void (*display_DrawRect)(int x1, int y1, int x2, int y2, long col);
extern void (*display_ReadRect)(int x1, int y1, int x2, int y2, long *colarray);

// display and touch NULL driver
void null_Attach(void);	// attach NULL driver
void null_Detach(void);
void null_InitPort(void);
void null_Cls(void);
void null_PutChr(int ch);
void null_ScrollUp(void);
void null_DrawRect(int x1, int y1, int x2, int y2, long col);
void null_ReadRect(int x1, int y1, int x2, int y2, long *colarray);
void null_ReadTouch(void);

#ifdef __cplusplus
}
#endif

#endif // RVM_H
