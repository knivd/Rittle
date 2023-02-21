#include <stdio.h>
#include <stdarg.h>
#include "drv_lcds.h"

#define LCD_DC		LATBbits.LATB11		// pin 28
#define LCD_RES		LATBbits.LATB12		// pin 27
#define LCD_CS		LATBbits.LATB13		// pin 24

#define SPI_DISPLAY_CLK	16000000		// SPI display SCLK frequency

#define LCDSPI_NOP				0x00
#define LCDSPI_SWRESET			0x01
#define LCDSPI_RDDID			0x04
#define LCDSPI_RDDST			0x09
#define LCDSPI_SLPIN			0x10
#define LCDSPI_SLPOUT			0x11
#define LCDSPI_PTLON			0x12
#define LCDSPI_NORON			0x13
#define LCDSPI_INVOFF			0x20
#define LCDSPI_INVON			0x21
#define LCDSPI_DISPOFF			0x28
#define LCDSPI_DISPON			0x29
#define LCDSPI_CASET			0x2A
#define LCDSPI_RASET			0x2B
#define LCDSPI_RAMWR			0x2C
#define LCDSPI_RAMRD			0x2E
#define LCDSPI_PTLAR			0x30
#define LCDSPI_VSCRDEF			0x33
#define LCDSPI_MADCTL			0x36
#define LCDSPI_VSCSAD			0x37
#define LCDSPI_COLMOD			0x3A
#define LCDSPI_MEMWRC			0x3C
#define LCDSPI_MEMRDC			0x3E
#define LCDSPI_FRMCTR1			0xB1
#define LCDSPI_FRMCTR2			0xB2
#define LCDSPI_FRMCTR3			0xB3
#define LCDSPI_INVCTR			0xB4
#define LCDSPI_DISSET5			0xB6
#define LCDSPI_PWCTR1			0xC0
#define LCDSPI_PWCTR2			0xC1
#define LCDSPI_PWCTR3			0xC2
#define LCDSPI_PWCTR4			0xC3
#define LCDSPI_PWCTR5			0xC4
#define LCDSPI_VMCTR1			0xC5
#define LCDSPI_RDID1			0xDA
#define LCDSPI_RDID2			0xDB
#define LCDSPI_RDID3			0xDC
#define LCDSPI_RDID4			0xDD
#define LCDSPI_PWCTR6			0xFC
#define LCDSPI_GMCTRP1			0xE0
#define LCDSPI_GMCTRN1			0xE1

// MADCTL constants
#define LCDSPI_MADCTL_MY		BIT(7)	// invert the Y coordinate
#define LCDSPI_MADCTL_MX		BIT(6)	// invert the X coordinate
#define LCDSPI_MADCTL_MV		BIT(5)	// swap X and Y coordinates
#define LCDSPI_MADCTL_ML		BIT(4)	// invert vertical refresh direction
#define LCDSPI_MADCTL_BGR		BIT(3)	// invert colour order
#define LCDSPI_MADCTL_MH		BIT(2)	// invert horizontal refresh direction

// orientation constants for MADCTL
#define LCDSPI_Portrait			(0)
#define LCDSPI_Portrait180		(LCDSPI_MADCTL_MX | LCDSPI_MADCTL_MY)
#define LCDSPI_Landscape		(LCDSPI_MADCTL_MV | LCDSPI_MADCTL_MX)
#define LCDSPI_Landscape180		(LCDSPI_MADCTL_MV | LCDSPI_MADCTL_MY)

// mirrored orientation constants for MADCTL
#define LCDSPIm_Portrait		(LCDSPI_MADCTL_MX)
#define LCDSPIm_Portrait180		(LCDSPI_MADCTL_MY)
#define LCDSPIm_Landscape		(LCDSPI_MADCTL_MV)
#define LCDSPIm_Landscape180	(LCDSPI_MADCTL_MV | LCDSPI_MADCTL_MX | LCDSPI_MADCTL_MY)

unsigned short scrla=0;	// scroll address index
unsigned short disV;	// hardware vertical resolution of the display


static unsigned char SPIsend(unsigned char txd) {
	SpiChnGetRov(SPI_CHANNEL3, TRUE);
    SpiChnPutC(SPI_CHANNEL3, txd);					// send data
    return (unsigned char)SpiChnGetC(SPI_CHANNEL3);	// receive data
}


// send single command only
void spi_write_command(char cmd) {
	if(LCD_CS==0) LCD_CS=1;
	LCD_DC=0;
	LCD_CS=0;
    SPIsend(cmd);
	LCD_DC=1;
}


// command with following data
void spi_write_cd(char cmd, int count, ...){
	va_list ap;
	va_start(ap, count);
	LCD_DC=0;
	LCD_CS=0;
	SPIsend(cmd);
	LCD_DC=1;
	int t=count;
	while(0<t--) SPIsend((char)va_arg(ap, int));
	LCD_CS=1;
	va_end(ap);
}


// NOTE: CS# remains low after exit from here
// (rw) 0:write, 1:read
// NOTE: assuming that x1<=x2 and y1<=y2
void DefineRegion(int x1, int y1, int x2, int y2, char rw) {
	if(x1 < 0) x1=0; if(x2 >= resH) x2=resH-1;
    if(y1 < 0) y1=0; if(y2 >= resV) y2=resV-1;
	if(orientation==PORTRAIT) {			// only meaningful with hardware scroll
		y1=(y1+scrla)%resV;
		y2=(y2+scrla)%resV;
	}
	else if(orientation==RPORTRAIT) {	// only meaningful with hardware scroll
		y1=(y1+disV-scrla)%resV;
		y2=(y2+disV-scrla)%resV;
	}
	spi_write_command(LCDSPI_CASET);
	SPIsend(x1>>8);
	SPIsend(x1);
	SPIsend(x2>>8);
	SPIsend(x2);
	spi_write_command(LCDSPI_RASET);
	SPIsend(y1>>8);
	SPIsend(y1);
	SPIsend(y2>>8);
	SPIsend(y2);
	if(!rw) spi_write_command(LCDSPI_RAMWR);
	else spi_write_command(LCDSPI_RAMRD);
}


// draw a filled rectangle
// this is the basic drawing primitive used by most drawing routines
//    x1, y1, x2, y2 - the coordinates
//    c - the colour
void lcdSPI_DrawRect(int x1, int y1, int x2, int y2, long c){
	long t;

    // ensure coordinates are within the screen boundaries
	if(x2 < x1) { t=x1; x1=x2; x2=t; }
    if(y2 < y1) { t=y1; y1=y2; y2=t; }
	if(x2<0 || x1>=resH || y2<0 || y1>=resV) return;	// entirely outside the screen

	// send the data block in 18-bit colour format to the LCD
    DefineRegion(x1, y1, x2, y2, 0);
	t=(x2-x1+1)*(y2-y1+1);
    while(t--) {
		SPIsend(c>>16);	// R
		SPIsend(c>>8);	// G
		SPIsend(c);		// B
	}
	LCD_CS=1;
}


void lcdSPI_ReadRect(int x1, int y1, int x2, int y2, long *carray){
	long t, c;

    // ensure coordinates are within the screen boundaries
	if(x2 < x1) { t=x1; x1=x2; x2=t; }
    if(y2 < y1) { t=y1; y1=y2; y2=t; }
	if(x2<0 || x1>=resH || y2<0 || y1>=resV) return;	// entirely outside the screen

	// read the data block
    DefineRegion(x1, y1, x2, y2, 1);
	SPIsend(0);	// reading requires one dummy byte
	t=(x2-x1+1)*(y2-y1+1);
	unsigned char *b=(unsigned char *)carray;
	while(t--) {
		*(b+3)=0;
		*(b+2)=SPIsend(0);	// R
		*(b+1)=SPIsend(0);	// G
		*b=SPIsend(0);		// B
		b+=sizeof(long);
	}
	LCD_CS=1;
}


void lcdSPI_Cls(void) {
	scrla=0;
	spi_write_cd(LCDSPI_VSCSAD, 2, 0, 0);				// reset scroll
	spi_write_cd(LCDSPI_CASET,4,0,0,(resH>>8),resH);	// set column address
	spi_write_cd(LCDSPI_RASET,4,0,0,(resV>>8),resV);	// set row address
	signed long bc=fontBcol;
	if(bc<0) bc=0;
	lcdSPI_DrawRect(0,0,(resH-1),(resV-1),bc);
	posX=posY=0;
}


void lcdSPI_ScrollUp(void) {
	unsigned short f=fontScale*(font->header.blankU+font->header.height+font->header.blankD);
	signed long bc=fontBcol;
	if(bc<0) bc=0;

	// hardware scroll only works in portrait mode
	if(orientation==PORTRAIT || orientation==RPORTRAIT) {
		lcdSPI_DrawRect(0, 0, resH-1, f-1, bc);
		spi_write_cd(LCDSPI_VSCRDEF, 6, 0, 0, (disV>>8), disV, 0, 0);	// TFA+VSA+BFA must be fixed disV
		if(orientation==PORTRAIT) scrla=(scrla+f)%resV; else scrla=(scrla+disV-f)%resV;
		spi_write_cd(LCDSPI_VSCSAD, 2, (scrla>>8), scrla);
		dlyus(20);
	}

	// for landscape mode a software scroll has to be implemented
	else {
		unsigned char *b, *buf=NULL;
		xalloc((unsigned char **)&buf,(resH*3));
		if(!buf) return;
		int t,r;
		for(r=f; r<resV; r++) {
			DefineRegion(0, r, resH-1, r, 1);
			SPIsend(0);	// reading requires one dummy byte
			b=buf;
			t=resH*3;
			while(t--) *(b++)=SPIsend(0);
			DefineRegion(0, r-f, resH-1, r-f, 0);
			b=buf;
			t=resH*3;
			while(t--) SPIsend(*(b++));
		}
		xfree((unsigned char **)&buf);
	}

	lcdSPI_DrawRect(0, resV-f, resH-1, resV-1, bc);
}


void lcdSPI_InitPort(void) {

	// initialise the DC, CS#, RESET#
	PORTSetPinsDigitalOut(IOPORT_B, (BIT(11) | BIT(12) | BIT(13)));
	CNPUBSET=(BIT(11) | BIT(12) | BIT(13));
	LCD_CS=1;
	LCD_DC=1;

	// init the SPI port
	SpiChnEnable(SPI_CHANNEL3, FALSE);
	PORTSetPinsDigitalOut(IOPORT_B, (BIT(10) | BIT(14)));	// RB14 is SCLK3 and RB10 is MOSI3
	PORTSetPinsDigitalIn(IOPORT_B, BIT(9));  // RB9 is MISO3
	PPSUnLock;
	PPSOutput(1, RPB10, SDO3);
	PPSInput(1, SDI3, RPB9);
	PPSLock;
	//SpiOpenFlags f=SPI_OPEN_CKP_HIGH;	// SPI mode 3
	SpiOpenFlags f=SPI_OPEN_CKE_REV;	// SPI mode 0
	SpiChnOpen(SPI_CHANNEL3, (f /*| SPI_OPEN_ENHBUF*/ | SPI_OPEN_MODE8 | SPI_OPEN_MSTEN |
								SPI_OPEN_SMP_END | SPI_OPEN_ON), (PBCLK2/SPI_DISPLAY_CLK));
	SpiChnEnable(SPI_CHANNEL3, TRUE);
}


void lcdSPI_Detach(void) {
	SpiChnEnable(SPI_CHANNEL3, FALSE);
	PORTResetPins(IOPORT_B, (BIT(9) | BIT(10) | BIT(14)));
	PPSUnLock;
	PPSOutput(1, RPB10, NULL);
	PPSLock;
	PORTResetPins(IOPORT_B, (BIT(11) | BIT(12) | BIT(13)));
}


void lcdSPI_Attach(unsigned short disV_value) {
	display_InitPort=lcdSPI_InitPort;
	display_Detach=lcdSPI_Detach;
	display_Cls=lcdSPI_Cls;
	display_ScrollUp=lcdSPI_ScrollUp;
	display_DrawRect=lcdSPI_DrawRect;
	display_ReadRect=lcdSPI_ReadRect;

	// initialise the display
	disV=disV_value;
	lcdSPI_InitPort();
	LCD_RES=1;
	dlyus(100000);
	LCD_RES=0;
    dlyus(100000);
    LCD_RES=1;
    dlyus(200000);
	spi_write_command(LCDSPI_SWRESET); dlyus(150000);
	spi_write_command(LCDSPI_SLPOUT); dlyus(500000);		// out of sleep mode
	spi_write_cd(LCDSPI_COLMOD,1,0x66);					// set 18-bit colour mode
	scrla=0;
	spi_write_cd(LCDSPI_VSCSAD, 2, 0, 0);				// reset scroll
	spi_write_cd(LCDSPI_CASET,4,0,0,(resH>>8),resH);	// set column address
	spi_write_cd(LCDSPI_RASET,4,0,0,(resV>>8),resV);	// set row address
	spi_write_command(LCDSPI_INVOFF);
	spi_write_command(LCDSPI_NORON);					// normal display on
    spi_write_command(LCDSPI_DISPON);
	if(orientation>=0) {
		xflags&=~XFLAG_MIRROR;
		switch(orientation) {
			case LANDSCAPE:		spi_write_cd(LCDSPI_MADCTL, 1, LCDSPI_MADCTL_BGR | LCDSPI_Landscape); break;
			case PORTRAIT:		spi_write_cd(LCDSPI_MADCTL, 1, LCDSPI_MADCTL_BGR | LCDSPI_Portrait); break;
			case RLANDSCAPE:	spi_write_cd(LCDSPI_MADCTL, 1, LCDSPI_MADCTL_BGR | LCDSPI_Landscape180); break;
			case RPORTRAIT:		spi_write_cd(LCDSPI_MADCTL, 1, LCDSPI_MADCTL_BGR | LCDSPI_Portrait180); break;
		}
	}
	else {
		xflags|=XFLAG_MIRROR;
		orientation=-orientation;
		switch(orientation) {
			case LANDSCAPE:		spi_write_cd(LCDSPI_MADCTL, 1, LCDSPI_MADCTL_BGR | LCDSPIm_Landscape); break;
			case PORTRAIT:		spi_write_cd(LCDSPI_MADCTL, 1, LCDSPI_MADCTL_BGR | LCDSPIm_Portrait); break;
			case RLANDSCAPE:	spi_write_cd(LCDSPI_MADCTL, 1, LCDSPI_MADCTL_BGR | LCDSPIm_Landscape180); break;
			case RPORTRAIT:		spi_write_cd(LCDSPI_MADCTL, 1, LCDSPI_MADCTL_BGR | LCDSPIm_Portrait180); break;
		}
	}
	LCD_CS=1;	// ensure that the CS# signal is put inactive
}
