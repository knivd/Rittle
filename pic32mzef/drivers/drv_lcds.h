#ifndef LCDSPI_H
#define LCDSPI_H

// SPI displays such ST7735, ILI9346, and others

#ifdef __cplusplus
extern "C" {
#endif

#include "..\platform.h"

// (disV_value) is the hardware vertical resolution of the display
void lcdSPI_Attach(unsigned short disV_value);

#ifdef __cplusplus
}
#endif

#endif // LCDSPI_H
