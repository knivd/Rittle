#ifndef SSD13_H
#define SSD13_H

#ifdef __cplusplus
extern "C" {
#endif

#include "..\platform.h"

// (disV_value) is the hardware vertical resolution of the display
void lcdSSD1309_Attach(unsigned short disV_value);

#ifdef __cplusplus
}
#endif

#endif // SSD13_H
