#ifndef TOUCH_RSPI_H
#define TOUCH_RSPI_H

// SPI resistive touch screen controllers based on XPT2046 / TSC2046 / ADS7846 / ADS7843

#ifdef __cplusplus
extern "C" {
#endif

#include "..\platform.h"

void trspi_Attach(void);

#ifdef __cplusplus
}
#endif

#endif // TOUCH_RSPI_H
