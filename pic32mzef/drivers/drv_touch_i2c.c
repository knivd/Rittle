#include <stdio.h>
#include "drv_touch2046.h"


void t2046_ReadTouch(char *pendingPairs) {




}


void t2046_ReadTouchPair(int *x, int *y) {
	if(tPendingPairs<1) t2046_ReadTouch(&tPendingPairs);
	if(tPendingPairs<1) return;







}


void t2046_InitPort(void) {

	// enable and configure I2C5
	CNPUFSET=(BIT_4 | BIT_5);	// enable the built-in pull-ups (in case there are none outside)
	I2CSetFrequency(I2C5, PBCLK2, 100000);	// initialise the I2C port at 100kbps
	I2CConfigure(I2C5, (I2C_ENABLE_SLAVE_CLOCK_STRETCHING | I2C_ENABLE_HIGH_SPEED));
	I2CEnable(I2C5, TRUE);

	// configure RD0 as TIRQ# input
	PORTSetPinsDigitalIn(IOPORT_D, BIT_0);	// RD0 will be used as interrupt input
	CNPUDSET|=BIT(0);		// enable the internal pull-up resistor on RD0
	INTSoftwareNMITrigger();
	xflags&=~XFLAG_INT0;
	mINT0ClearIntFlag();
	mINT0IntEnable(TRUE);	// enable the external interrupt

	xflags|=XFLAG_TOUCH;
}


void t2046_Detach(void) {
	I2CEnable(I2C5, FALSE);
	PORTResetPins(IOPORT_F, (BIT_4 | BIT_5));
}


void t2046_Attach(void) {
	touch_InitPort = t2046_InitPort;
	touch_Detach = t2046_Detach;
	touch_ReadTouch = t2046_ReadTouch;
	touch_ReadTouchPair = t2046_ReadTouchPair;







}

