#ifndef RSC_H
#define RSC_H

// RITTLE COMPILER

#ifdef __cplusplus
extern "C" {
#endif

#include "rittle.h"

// compile and return error code (negative number) or total compiled code length (positive number or 0)
// (level) must be 0 when the function is called from outside
// (scf) is a "speculative compilation flag" - performs compilation without writing into the destination
//				the destination however still needs to point to a valid memory address
//				this can be used to estimate the size of a compiled code before performing an actual compilation
// (dbginfo) is a flag specifying whether debug commentaries should be included in the output code
signed long compile(char **source, unsigned char **destination, unsigned short level, char scf, char dbginfo);

#ifdef __cplusplus
}
#endif

#endif // RSC_H
