#include <stdio.h>
#include "drv_ssd196.h"

#define GPIO0       0
#define GPIO1       1
#define GPIO2       2
#define GPIO3       3

#define LCD_RESET   (1<<GPIO0)	// LCD Reset signal (Reset for display panel, NOT ssd1963)
#define LCD_SPENA	0
#define LCD_SPCLK	0
#define LCD_SPDAT	0

#define SSD1963_LANDSCAPE		0b00
#define SSD1963_PORTRAIT		((1 << 7) | (1 << 5))
#define SSD1963_RLANDSCAPE		(SSD1963_LANDSCAPE | 0b11)
#define SSD1963_RPORTRAIT		(SSD1963_PORTRAIT | 0b11)

// SSD1963 command table
#define SSD_NOP					0x00	// no operation
#define SSD_SOFT_RESET			0x01	// software reset
#define SSD_GET_PWR_MODE		0x0A	// get the current power mode
#define SSD_GET_ADDR_MODE		0x0B	// get the frame memory to the display panel read order
#define SSD_GET_PIXEL_FORMAT	0x0C	// get the current pixel format
#define SSD_GET_DISPLAY_MODE	0x0D	// return the display mode
#define SSD_GET_SIGNAL_MODE		0x0E
#define SSD_GET_DIAGNOSTIC		0x0F
#define SSD_ENT_SLEEP			0x10
#define SSD_EXIT_SLEEP			0x11
#define SSD_ENT_PARTIAL_MODE	0x12
#define SSD_ENT_NORMAL_MODE		0x13
#define SSD_EXIT_INVERT_MODE	0x20
#define SSD_ENT_INVERT_MODE		0x21
#define SSD_SET_GAMMA			0x26
#define SSD_BLANK_DISPLAY		0x28
#define SSD_ON_DISPLAY			0x29
#define SSD_SET_COLUMN			0x2A
#define SSD_SET_PAGE			0x2B
#define SSD_WR_MEMSTART			0x2C
#define SSD_RD_MEMSTART			0x2E
#define SSD_SET_PARTIAL_AREA	0x30
#define SSD_SET_SCROLL_AREA		0x33
#define SSD_SET_TEAR_OFF		0x34	// synchronization information is not sent from the display
#define SSD_SET_TEAR_ON			0x35	// sync. information is sent from the display
#define SSD_SET_ADDR_MODE		0x36	// set fram buffer read order to the display panel
#define SSD_SET_SCROLL_START	0x37
#define SSD_EXIT_IDLE_MODE		0x38
#define SSD_ENT_IDLE_MODE		0x39
#define SSD_SET_PIXEL_FORMAT	0x3A	// defines how many bits per pixel is used
#define SSD_WR_MEM_AUTO			0x3C
#define SSD_RD_MEM_AUTO			0x3E
#define SSD_SET_TEAR_SCANLINE	0x44
#define SSD_GET_SCANLINE		0x45
#define SSD_RD_DDB_START		0xA1
#define SSD_RD_DDB_AUTO			0xA8
#define SSD_SET_PANEL_MODE		0xB0
#define SSD_GET_PANEL_MODE		0xB1
#define SSD_SET_HOR_PERIOD		0xB4
#define SSD_GET_HOR_PERIOD		0xB5
#define SSD_SET_VER_PERIOD		0xB6
#define SSD_GET_VER_PERIOD		0xB7
#define SSD_SET_GPIO_CONF		0xB8
#define SSD_GET_GPIO_CONF		0xB9
#define SSD_SET_GPIO_VAL		0xBA
#define SSD_GET_GPIO_STATUS		0xBB
#define SSD_SET_POST_PROC		0xBC
#define SSD_GET_POST_PROC		0xBD
#define SSD_SET_PWM_CONF		0xBE
#define SSD_GET_PWM_CONF		0xBF
#define SSD_SET_LCD_GEN0		0xC0
#define SSD_GET_LCD_GEN0		0xC1
#define SSD_SET_LCD_GEN1		0xC2
#define SSD_GET_LCD_GEN1		0xC3
#define SSD_SET_LCD_GEN2		0xC4
#define SSD_GET_LCD_GEN2		0xC5
#define SSD_SET_LCD_GEN3		0xC6
#define SSD_GET_LCD_GEN3		0xC7
#define SSD_SET_GPIO0_ROP		0xC8
#define SSD_GET_GPIO0_ROP		0xC9
#define SSD_SET_GPIO1_ROP		0xCA
#define SSD_GET_GPIO1_ROP		0xCB
#define SSD_SET_GPIO2_ROP		0xCC
#define SSD_GET_GPIO2_ROP		0xCD
#define SSD_SET_GPIO3_ROP		0xCE
#define SSD_GET_GPIO3_ROP		0xCF
#define SSD_SET_ABC_DBC_CONF	0xD0
#define SSD_GET_ABC_DBC_CONF	0xD1
#define SSD_SET_DBC_HISTO_PTR	0xD2
#define SSD_GET_DBC_HISTO_PTR	0xD3
#define SSD_SET_DBC_THRES		0xD4
#define SSD_GET_DBC_THRES		0xD5
#define SSD_SET_ABM_TMR			0xD6
#define SSD_GET_ABM_TMR			0xD7
#define SSD_SET_AMB_LVL0		0xD8
#define SSD_GET_AMB_LVL0		0xD9
#define SSD_SET_AMB_LVL1		0xDA
#define SSD_GET_AMB_LVL1		0xDB
#define SSD_SET_AMB_LVL2		0xDC
#define SSD_GET_AMB_LVL2		0xDD
#define SSD_SET_AMB_LVL3		0xDE
#define SSD_GET_AMB_LVL3		0xDF
#define SSD_PLL_START			0xE0	// start the PLL
#define SSD_PLL_STOP			0xE1	// disable the PLL
#define SSD_SET_PLL_MN			0xE2
#define SSD_GET_PLL_MN			0xE3
#define SSD_GET_PLL_STATUS		0xE4	// get the current PLL status
#define SSD_ENT_DEEP_SLEEP		0xE5
#define SSD_SET_PCLK			0xE6	// set pixel clock (LSHIFT signal) frequency
#define SSD_GET_PCLK			0xE7	// get pixel clock (LSHIFT signal) freq. settings
#define SSD_SET_DATA_INTERFACE	0xF0
#define SSD_GET_DATA_INTERFACE	0xF1

#define WriteCommandSlow(cmd)	{ LCD_SSD_DC=0; DB8_write(cmd,5); LCD_SSD_DC=1; }
#define WriteCommand(cmd)		{ LCD_SSD_DC=0; DB8_write(cmd,0); LCD_SSD_DC=1; }
#define WriteDataSlow(data)		{ DB8_write(data,5); }
#define WriteData(data)			{ DB8_write(data,0); }

// parameters for the SSD1963 display panel (refer to the glass data sheet)
int SSD1963HorizPulseWidth, SSD1963HorizBackPorch, SSD1963HorizFrontPorch;
int SSD1963VertPulseWidth, SSD1963VertBackPorch, SSD1963VertFrontPorch;
int SSD1963PClock1, SSD1963PClock2, SSD1963PClock3;
int SSD1963Mode1, SSD1963Mode2;
int ScrollStart;


/*********************************************************************
* defines start/end coordinates for memory access from host to SSD1963
* also maps the start and end points to suit the orientation
********************************************************************/
void SetAreaSSD1963(int x1, int y1, int x2, int y2) {
    int start_x, start_y, end_x, end_y;
    switch(orientation) {
        case LANDSCAPE:
        case RLANDSCAPE: start_x = x1;
                         end_x = x2;
                         start_y = y1;
                         end_y = y2;
                         break;
        case PORTRAIT:
        case RPORTRAIT:  start_x = y1;
                         end_x = y2;
                         start_y = (resV - 1) - x2;
                         end_y = (resV - 1) - x1;
                         break;
        default: return;
    }
	WriteCommand(SSD_SET_COLUMN);
	WriteData(start_x>>8);
	WriteData(start_x);
	WriteData(end_x>>8);
	WriteData(end_x);
	WriteCommand(SSD_SET_PAGE);
	WriteData(start_y>>8);
	WriteData(start_y);
	WriteData(end_y>>8);
	WriteData(end_y);
}


/*********************************************************************
* Set a GPIO pin to state high(1) or low(0)
*
* PreCondition: Set the GPIO pin an output prior using this function
*
* Arguments: BYTE pin	- 	LCD_RESET
*							LCD_SPENA
*							LCD_SPCLK
*							LCD_SPDAT
*
*			 BOOL state - 	0 for low
*							1 for high
*********************************************************************/
static void GPIO_WR(char pin, char state) {
	int _gpioStatus = 0;
	if(state==1) _gpioStatus = _gpioStatus|pin;
	else _gpioStatus = _gpioStatus&(~pin);
	WriteCommand(SSD_SET_GPIO_VAL);		// Set GPIO value
	WriteData(_gpioStatus);
}


/*********************************************************************
* SetBacklight(BYTE intensity)
* Some boards may use of PWM feature of ssd1963 to adjust the backlight
* intensity and this function supports that.  However, most boards have
* a separate PWM input pin
*
* Input: intensity = 0 (off) to 100 (full on)
* Note: The base frequency of PWM set to around 300Hz with PLL set to 120MHz
*		This parameter is hardware dependent
********************************************************************/
void SetBacklightSSD1963(int intensity) {
	WriteCommand(SSD_SET_PWM_CONF);		// Set PWM configuration for backlight control
	WriteData(0x0E);					// PWMF[7:0] = 2, PWM base freq = PLL/(256*(1+5))/256 = 300Hz for a PLL freq = 120MHz
	WriteData((intensity * 255)/100);	// Set duty cycle, from 0x00 (total pull-down) to 0xFF (99% pull-up , 255/256)
	WriteData(0x01);					// PWM enabled and controlled by host MCU
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x00);
}


void lcdSSD196_InitPort(void) {
	CNPUESET=(BIT(0) | BIT(4));
	PORTSetPinsDigitalOut(IOPORT_E, (BIT_0 | BIT_1 | BIT_4));
	LCD_SSD_CS=1;
	LCD_SSD_DC=1;
	LCD_SSD_RES=1;
	DB8_init();
}


// internal for DrawRect()
void ssdRect(int x1, int y1, int x2, int y2, int col) {
    long i;
    SetAreaSSD1963(x1, y1, x2, y2);	// setup the area to be filled
    WriteCommand(SSD_WR_MEMSTART);
    for(i = (x2 - x1 + 1) * (y2 - y1 + 1); i > 0; i--) {	// fill the area
		DB8_write((col>>16),0);
		DB8_write((col>>8),0);
		DB8_write(col,0);
	}
}


void lcdSSD196_DrawRect(int x1, int y1, int x2, int y2, long col) {
	lcdSSD196_InitPort();
	long t;

    // ensure coordinates are within the screen boundaries
	if(x2 < x1) { t=x1; x1=x2; x2=t; }
    if(y2 < y1) { t=y1; y1=y2; y2=t; }
	if(x2<0 || x1>=resH || y2<0 || y1>=resV) return;	// entirely outside the screen
    t = y2 - y1;	// get the distance between the top and bottom

    // set y1 to the physical location in the frame buffer (only really has an effect when scrolling is in action)
    if(orientation == RLANDSCAPE) y1 = (y1 + (resV - ScrollStart)) % resV;
    else y1 = (y1 + ScrollStart) % resV;
    y2 = y1 + t;		// and set y2 to the same
    if(y2 >= resV) {	// if the box splits over the frame buffer boundary
		ssdRect(x1, y1, x2, resV - 1, col);		// draw the top part
        ssdRect(x1, 0, x2, y2 - resV , col);	// and the bottom part
    }
	else ssdRect(x1, y1, x2, y2, col);	// the whole box is within the frame buffer - much easier
}


//###
void lcdSSD196_ReadRect(int x1, int y1, int x2, int y2, long *colarray) {
	lcdSSD196_InitPort();




}


//###
void lcdSSD196_ScrollUp(void) {
	lcdSSD196_InitPort();



}


void lcdSSD196_Cls(void) {
	lcdSSD196_InitPort();
	signed long bc=fontBcol;
	if(bc<0) bc=0;
	lcdSSD196_DrawRect(0,0,resH-1,resV-1,bc);
}


void lcdSSD196_Detach(void) {
	PORTResetPins(IOPORT_E, (BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4)));
	CNPUESET=(BIT(2) | BIT(3) | BIT(4));
}


void lcdSSD196_Attach(void) {
	display_InitPort=lcdSSD196_InitPort;
	display_Detach=lcdSSD196_Detach;
	display_Cls=lcdSSD196_Cls;
	display_ScrollUp=lcdSSD196_ScrollUp;
	display_DrawRect=lcdSSD196_DrawRect;
	display_ReadRect=lcdSSD196_ReadRect;



	// ###
	resH = 800;                         // this is a 7" glass alternative version (high brightness)
	resV = 480;
	SSD1963HorizPulseWidth = 3;
	SSD1963HorizBackPorch = 88;
	SSD1963HorizFrontPorch = 37;
	SSD1963VertPulseWidth = 3;
	SSD1963VertBackPorch = 32;
	SSD1963VertFrontPorch = 10;
	SSD1963PClock1 = 0x03;
	SSD1963PClock2 = 0xff;
	SSD1963PClock3 = 0xff;
	SSD1963Mode1 = 0x10;                        // 18-bit for 7" panel
	SSD1963Mode2 = 0x80;
	// ###



	lcdSSD196_InitPort();
	LCD_SSD_RES=1;
	dlyus(50000);
	LCD_SSD_RES=0;
    dlyus(10000);
    LCD_SSD_RES=1;
    dlyus(50000);

	// IMPORTANT: At this stage the SSD1963 is running at a slow speed and cannot respond to high speed commands
    //            So we use slow speed versions of WriteCommand/WriteData with a 3 uS delay between each control signal change

	// Set MN(multipliers) of PLL, VCO = crystal freq * (N+1)
	// PLL freq = VCO/M with 250MHz < VCO < 800MHz
	// The max PLL freq is around 120MHz

	WriteCommandSlow(SSD_SET_PLL_MN);	// Set PLL with OSC = 10MHz (hardware), command is 0xE3
	WriteDataSlow(0x23);				// Multiplier N = 35, VCO (>250MHz)= OSC*(N+1), VCO = 360MHz
	WriteDataSlow(0x02);				// Divider M = 2, PLL = 360/(M+1) = 120MHz
	WriteDataSlow(0x54);				// Validate M and N values

	WriteCommandSlow(SSD_PLL_START);	// Start PLL command
	WriteDataSlow(0x01);				// enable PLL

	dlyus(5000);						// wait for it to stabilise
	WriteCommandSlow(SSD_PLL_START);	// Start PLL command again
	WriteDataSlow(0x03);				// now, use PLL output as system clock

	WriteCommandSlow(SSD_SOFT_RESET);	// Soft reset
	dlyus(20000);

	// Configure for the TFT panel, varies from individual manufacturer
	WriteCommandSlow(SSD_SET_PCLK);		// set pixel clock (LSHIFT signal) frequency
	WriteDataSlow(SSD1963PClock1);		// parameters set by DISPLAY INIT
	WriteDataSlow(SSD1963PClock2);
	WriteDataSlow(SSD1963PClock3);
    dlyus(5000);

	// Set panel mode, varies from individual manufacturer
	WriteCommand(SSD_SET_PANEL_MODE);
	WriteData(SSD1963Mode1);			// parameters set by DISPLAY INIT
	WriteData(SSD1963Mode2);
	WriteData((resH - 1) >> 8);			// Set panel size
	WriteData(resH - 1);
	WriteData((resV - 1) >> 8);
	WriteData(resV - 1);
	WriteData(0x00);					// RGB sequence

	// Set horizontal period
	WriteCommand(SSD_SET_HOR_PERIOD);
	#define HT (resH + SSD1963HorizPulseWidth + SSD1963HorizBackPorch + SSD1963HorizFrontPorch)
	WriteData((HT - 1) >> 8);
	WriteData(HT - 1);
	#define HPS (SSD1963HorizPulseWidth + SSD1963HorizBackPorch)
	WriteData((HPS - 1) >> 8);
	WriteData(HPS - 1);
	WriteData(SSD1963HorizPulseWidth - 1);
	WriteData(0x00);
	WriteData(0x00);
	WriteData(0x00);

	// Set vertical period
	WriteCommand(SSD_SET_VER_PERIOD);
	#define VT (SSD1963VertPulseWidth + SSD1963VertBackPorch + SSD1963VertFrontPorch + resV)
	WriteData((VT - 1) >> 8);
	WriteData(VT - 1);
	#define VSP (SSD1963VertPulseWidth + SSD1963VertBackPorch)
	WriteData((VSP - 1) >> 8);
	WriteData(VSP - 1);
	WriteData(SSD1963VertPulseWidth - 1);
	WriteData(0x00);
	WriteData(0x00);

	// Set pixel data interface
	WriteCommand(SSD_SET_DATA_INTERFACE);
    WriteData(0x00);					// 8-bit colour format

    // initialise the GPIOs
	WriteCommand(SSD_SET_GPIO_CONF);	// Set all GPIOs to output, controlled by host
	WriteData(0x0f);					// Set GPIO0 as output
	WriteData(0x01);					// GPIO[3:0] used as normal GPIOs

	// LL Reset to LCD!!!
	GPIO_WR(LCD_SPENA, 1);
	GPIO_WR(LCD_SPCLK, 1);
	GPIO_WR(LCD_SPDAT,1);
	GPIO_WR(LCD_RESET,1);
	GPIO_WR(LCD_RESET,0);
	dlyus(5000);
	GPIO_WR(LCD_RESET,1);

    // setup the pixel write order
    WriteCommand(SSD_SET_ADDR_MODE);
    switch(orientation) {
        case LANDSCAPE:		WriteData(SSD1963_LANDSCAPE); break;
        case PORTRAIT:		WriteData(SSD1963_PORTRAIT); break;
        case RLANDSCAPE:	WriteData(SSD1963_RLANDSCAPE); break;
        case RPORTRAIT:		WriteData(SSD1963_RPORTRAIT); break;
    }

    // Set the scrolling area
	WriteCommand(SSD_SET_SCROLL_AREA);
	WriteData(0);
	WriteData(0);
	WriteData(resV >> 8);
	WriteData(resV);
	WriteData(0);
	WriteData(0);
    ScrollStart=0;

	WriteCommand(SSD_ON_DISPLAY);		// Turn on and show the image on display
}
