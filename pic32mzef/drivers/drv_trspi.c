#include <stdio.h>
#include "drv_trspi.h"

#define TOUCH_CS		LATCbits.LATC13	// pin 47
#define SPI_TOUCH_CLK	250000			// SPI touch SCLK frequency
#define X_PLATE_R		10000			// X-plate resistance
										// (this is just an arbitrary value here since
										// the actual plate resistance is not know)
#define TOUCH_SAMPLES	80

typedef struct {
	int x[TOUCH_SAMPLES];
	int y[TOUCH_SAMPLES];
	int z1[TOUCH_SAMPLES];
	int z2[TOUCH_SAMPLES];
} trspi_data_t;

trspi_data_t *td=NULL;


void trspi_InitPort(void) {
	PORTSetPinsDigitalIn(IOPORT_D, BIT_0);	// RD0 will be used as interrupt input
	CNPUDSET=BIT_0;		// enable the internal pull-up resistor on RD0
	PORTSetPinsDigitalOut(IOPORT_C, BIT_13);
	CNPUCSET=BIT_13;
	TOUCH_CS=1;
	init_sys_SPI(0,SPI_TOUCH_CLK);
	touch_points=0;
}


static void get_v(unsigned char cmd, int *v, unsigned int index) {
	sys_SPI(cmd);
	int tH = ((int)sys_SPI(0) << 5);
	int tL = ((int)sys_SPI(0) >> 3);
	*(v+index) = (int)(tH | tL);
	dlyus(2);
	if(index<1) return;
	do {
		if(*(v+index) < *(v+index-1)) {
			int z=*(v+index);
			*(v+index)=*(v+index-1);
			*(v+index-1)=z;
		}
	} while(--index);
}


void trspi_ReadTouch(void) {
	xfree((unsigned char **)&td);
	xalloc((unsigned char **)&td, sizeof(trspi_data_t));
	if(!td) {
		touch_points=0;
		xflags&=~XFLAG_TOUCH_IRQ;
		return;
	}
	trspi_InitPort();

	int t;
	for(t=0; t<TOUCH_SAMPLES; t++) {
		TOUCH_CS=0;
		dlyus(2);
		get_v(0b10110011, td->z1, t);	// Z1
		get_v(0b11000011, td->z2, t);	// Z2
		get_v(0b11010011, td->x, t);	// X
		get_v(0b10010011, td->y, t);	// Y
		sys_SPI(0b10010000);			// PENIRQ on
		dlyus(2);
		TOUCH_CS=1;
	}

	// take the middle of the sampling extract and average it
	td->x[0]=0; td->y[0]=0; td->z1[0]=0; td->z2[0]=0;
	for(t=TOUCH_SAMPLES/4; t<(TOUCH_SAMPLES-(TOUCH_SAMPLES/4)); t++) {
		td->x[0]+=td->x[t];
		td->y[0]+=td->y[t];
		td->z1[0]+=td->z1[t];
		td->z2[0]+=td->z2[t];
	}
	td->x[0]/=(TOUCH_SAMPLES/2);
	td->y[0]/=(TOUCH_SAMPLES/2);
	td->z1[0]/=(TOUCH_SAMPLES/2);
	td->z2[0]/=(TOUCH_SAMPLES/2);
	if(xflags & XFLAG_TOUCH_ROT) {	// swap the X/Y axes
		int z=td->x[0];
		td->x[0]=td->y[0];
		td->y[0]=z;
	}

	// if calibrated - convert to pixels
	if(touch_calx && touch_caly) {
		double v;
		v=((double)resH-30.0);
		if(v<1.0) v=1.0;
		v=(((double)abs((touch_calx & 0xffff) - (touch_calx >> 16))) / v);
		if(v>0) td->x[0]=(int)xround((double)td->x[0]/v);
		if(td->x[0]<0) td->x[0]=0;
		if(td->x[0]>=resH) td->x[0]=resH-1;
		if((touch_calx >> 16) > (touch_calx & 0xffff)) td->x[0]=(resH-1)-td->x[0];
		v=((double)resV-30.0);
		if(v<1.0) v=1.0;
		v=(((double)abs((touch_caly & 0xffff) - (touch_caly >> 16))) / v);
		if(v>0) td->y[0]=(int)xround((double)td->y[0]/v);
		if(td->y[0]<0) td->y[0]=0;
		if(td->y[0]>=resV) td->y[0]=resV-1;
		if((touch_caly >> 16) > (touch_caly & 0xffff)) td->y[0]=(resV-1)-td->y[0];
	}

	touch_x[0] = td->x[0];
	touch_y[0] = td->y[0];
	if(td->z1[0]) {	// calculate pressure
		touch_p[0] = X_PLATE_R * ((td->x[0]/4096) * ((td->z2[0]/td->z1[0]) - 1));
		touch_points=1;	// resistive touch screens only support 1 touch point
	}
	else touch_p[0]=-1;
	xfree((unsigned char **)&td);
	xflags&=~XFLAG_TOUCH_IRQ;
}


void trspi_Detach(void) {
	PORTResetPins(IOPORT_C, BIT_13);
	CNPUCSET=BIT_13;	// keep the pull-up on
}


void trspi_Attach(void) {
	touch_InitPort = trspi_InitPort;
	touch_Detach = trspi_Detach;
	touch_ReadTouch = trspi_ReadTouch;
	trspi_InitPort();
	TOUCH_CS=0;
	sys_SPI(0b10010000);	// PENIRQ on
	TOUCH_CS=1;

	// configure RD0 as TIRQ# input
	PORTSetPinsDigitalIn(IOPORT_D, BIT_0);	// RD0 will be used as interrupt input
	CNPUDSET=BIT_0;			// enable the internal pull-up resistor on RD0
	xflags&=~XFLAG_TOUCH_IRQ;
	xflags|=XFLAG_TOUCH;
}
