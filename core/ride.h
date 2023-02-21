#ifndef RIDE_H
#define RIDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rittle.h"

char *fname;    // RIDE project file name
char *text;		// text in the editor

// depth of the undo stack
#define UNDO_DEPTH  5

// integrated development environment
void ride(char *param);

#ifdef __cplusplus
}
#endif

#endif // RIDE_H
