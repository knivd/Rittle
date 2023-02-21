#ifndef PLATFORM_H
#define PLATFORM_H

// LOW-LEVEL AND HARDWARE-DEPENDENT FUNCTIONS

// platform: PIC32MZ2048EFH064 (TQFP64 and QFN64)

// pin  1:	DO/DI/AI
// pin  2:	DO/DI/AI			SD Card SEL2#
// pin  3:  DO/DI/AI			SD Card SEL1#
// pin  4:  DO/DI/AI			System SCLK
// pin  5:  DO/DI/AI			System MISO
// pin  6:  DO/DI/AI			System MOSI
// pin  7:  GND
// pin  8:  Vdd (+3.3V)
// pin  9:  [5V] MCLR#
// pin 10:	DO/DI/AI
// pin 11:	DO/DI/AI			COM4 Rx
// pin 12:	DO/DI/AI
// pin 13:	DO/DI/AI			COM4 Tx
// pin 14:	DO/DI/AI			COM5 Rx / PWM (group 2)
// pin 15:	DO/DI/AI			PWM (group 1) / Vref-
// pin 16:	DO/DI/AI			COM5 Tx / Vref+
// pin 17:  PGEC/DO/DI/AI		Console Rx
// pin 18:  PGED/DO/DI/AI		Console Tx
// pin 19:  AVdd (Fltr +3.3V)
// pin 20:  AGND (Fltr GND)
// pin 21:	DO/DI/AI			PWM (group 1)						DB0
// pin 22:	DO/DI/AI			SPI2 MISO			LCD(MISO)		DB1
// pin 23:	DO/DI/AI			SPI2 MOSI			LCD(MOSI)		DB2
// pin 24:	DO/DI/AI								LCD(DC)			DB3
// pin 25:  GND
// pin 26:  Vdd (+3.3V)
// pin 27:	DO/DI/AI								LCD(RESET#)		DB4
// pin 28:	DO/DI/AI			SPI2 slave CS#		LCD(CS#)		DB5
// pin 29:	DO/DI/AI			SPI2 SCLK			LCD(SCLK)		DB6
// pin 30:	DO/DI/AI			PWM (group 3)						DB7
// pin 31:  24MHz CLK Input
// pin 32:	CLK Sleep Control
// pin 33:  [5V] USB VBUS Input
// pin 34:  Vusb (+3.3V)
// pin 35:  GND
// pin 36:  USB Console D-
// pin 37:  USB Console D+
// pin 38:	DO/DI				PWM (group 2)
// pin 39:  Vdd (+3.3V)
// pin 40:  GND
// pin 41:	[5V] DO/DI			COM3 Rx / IIC2 SDA		Touch SDA
// pin 42:	[5V] DO/DI			COM3 Tx / IIC2 SCL		Touch SCL
// pin 43:	[5V] DO/DI			IIC1 SDA
// pin 44:	[5V] DO/DI			IIC1 SCL
// pin 45:	[5V] DO/DI			PWM (group 3)
// pin 46:	[5V] DO/DI			PWM (group 3) / Wake#	Touch IRQ#
// pin 47:	[5V] DO/DI									Touch CS#
// pin 48:	[5V] DO/DI			32.768kHz Input
// pin 49:	[5V] DO/DI			SPI1 SCLK
// pin 50:	[5V] DO/DI			SPI1 MISO
// pin 51:	[5V] DO/DI			SPI1 MOSI
// pin 52:	[5V] DO/DI			COM1 Rx
// pin 53:	[5V] DO/DI			COM1 Tx
// pin 54:  Vdd (+3.3V)
// pin 55:  GND
// pin 56:	[5V] DO/DI			COM2 Rx				LCD4(RS)
// pin 57:	[5V] DO/DI			COM2 Tx				LCD4(EN)
// pin 58:	[5V] DO/DI			SPI1 slave CS#		LCD4(D4)		DBRST#
// pin 59:  GND
// pin 60:  Vdd (+3.3V)
// pin 61:	[5V] DO/DI								LCD4(D5)		DBDC
// pin 62:	[5V] DO/DI								LCD4(D6)		DBRD#
// pin 63:	[5V] DO/DI								LCD4(D7)		DBWR#
// pin 64:	DO/DI/AI												DBCS#


#define HW_PLATFORM "PIC32MZ-EF"
#define SW_VERSION ":K6.0-alpha"	// must always start with ':'

#ifdef __cplusplus
extern "C" {
#endif

#include "..\core\rittle.h"
#include "..\pic32mzef\plibs\plib.h"
#include "..\pic32mzef\addons.h"
#include "..\pic32mzef\drivers\drv_lcd4.h"
#include "..\pic32mzef\drivers\drv_lcds.h"
#include "..\pic32mzef\drivers\drv_trspi.h"
#include "..\pic32mzef\drivers\drv_ssd196.h"

#define IFS_SIZE 1632       // reserved IFS: storage size in kB (MUST BE AT LEAST 256)
                            // IMPORTANT: (IFS_SIZE) must be divisible of (BYTE_ROW_SIZE)
#define RAM_DISK_SIZE 128   // RAM disk size in kB (this can be 0 if RAM disk is not required)

#define POSCCLK 24000000UL  // external primary oscillator frequency (Hz)
#define SOSCCLK 32768UL     // external secondary oscillator frequency (Hz)

#define SD1_nCS LATEbits.LATE7	// RE7 is the CS# of SD1 on the Rittle board
#define SD2_nCS LATEbits.LATE6	// RE6 is optionally CS# of SD2 on the Rittle board

#define SPI1_nCS LATEbits.LATE0		// SPI1 slave select line
#define SPI2_nCS LATBbits.LATB13	// SPI2 slave select line

#define DB8_nRD	LATEbits.LATE2	// 8-bit interface RD# line
#define DB8_nWR	LATEbits.LATE3	// 8-bit interface WR# line

// default clocks
// IMPORTANT: PBCLK2 and PBCLK3 must be kept 64 MHz at all CPU frequencies
//			  PBCLK1, PBCLK4, PBCLK5 must be as high as possible up to 100 MHz
//			  PBCLK7 depends on the CPU frequency selection
//			  PBCLK8 is never used and remains disabled at all times
#define SYSCLK	192000000UL // (max 252) main system clock
#define PBCLK1	(SYSCLK/2)	// (max 100) peripheral clock for WDT, DeadmanT, Flash
#define PBCLK2	64000000UL	// (max 100) peripheral clock for PMP, I2C, UART, SPI
#define PBCLK3	64000000UL	// (max 100) peripheral clock for ADC, Comp, Timers, OutCapt, InCapt
#define PBCLK4	(SYSCLK/2)	// (max 200) peripheral clock for Ports
#define PBCLK5	(SYSCLK/2)	// (max 100) vperipheral clock for Crypto RNG, USB, CAN, Eth, SQI
#define PBCLK7	(SYSCLK/1)	// (max 252) peripheral clock for CPU, DeadmanT
#define PBCLK8	(SYSCLK/2)	// (max 100) peripheral clock for EBI (NOT USED)

unsigned short sys_freq_mhz;	// current system clock frequency in MHz

#define COM_PORTS 5				// total number of supported COM ports (console not included)
#define SPI_PORTS 2				// total number of SPI ports
#define IIC_PORTS 2				// total number of I2C ports

// alternative key codes (for platform-dependent consoles)
#define KEY_BACKSPC_ALT	0x7f
#define KEY_LEFT_ALT	((KEY_ESC<<8)+'K')
#define KEY_RIGHT_ALT	((KEY_ESC<<8)+'M')
#define KEY_UP_ALT		((KEY_ESC<<8)+'H')
#define KEY_DOWN_ALT	((KEY_ESC<<8)+'P')
#define KEY_HOME_ALT	((KEY_ESC<<8)+'G')
#define KEY_END_ALT		((KEY_ESC<<8)+'O')
#define KEY_DEL_ALT		((KEY_ESC<<8)+'S')
#define KEY_INS_ALT		((KEY_ESC<<8)+'R')
#define KEY_PGUP_ALT	((KEY_ESC<<8)+'I')
#define KEY_PGDN_ALT	((KEY_ESC<<8)+'Q')
#define KEY_F1_ALT		((KEY_ESC<<8)+';')
#define KEY_F2_ALT		((KEY_ESC<<8)+'<')
#define KEY_F3_ALT		((KEY_ESC<<8)+'=')
#define KEY_F4_ALT		((KEY_ESC<<8)+'>')

// #define CONSOLE_ECHO 	// enable echo output in the console
// #define CONSOLE_DEBUG	// enable echo code output in the console

#define CONSOLE_RX_BUFFER 64	// console reception buffer size
#define COM_RX_BUFFER 64		// COM port reception buffer size

// UART reception buffers
volatile unsigned short *com_rx[COM_PORTS];
volatile unsigned short com_rx_in[COM_PORTS];	// incoming character index
volatile unsigned short com_rx_out[COM_PORTS];	// outgoing character index

// callback functions
unsigned long com_rx_callback[COM_PORTS];
unsigned long spi_rx_callback[SPI_PORTS];
unsigned long iic_rx_callback[IIC_PORTS];
unsigned long touch_callback;

// flags
#define XFLAG_ECLK		BIT(0)	// 24 MHz external clock is present and can be used
#define XFLAG_ECLK_WARN	BIT(1)	// warning about missing clock has been shown already
#define XFLAG_INT0		BIT(2)	// INT0 has been detected
#define XFLAG_COM0		BIT(3)	// indicates if COM0 is enabled
#define XFLAG_MIRROR	BIT(4)	// indicates that the display is in mirror mode
#define XFLAG_TOUCH		BIT(5)	// indicates that touch panel is enabled
#define XFLAG_TOUCH_IRQ	BIT(6)	// indicates that touch panel has fired an interrupt
#define XFLAG_TOUCH_ROT BIT(7)	// touch screen X/Y axis swap flag

extern volatile unsigned short xflags;

// internal flash disk data entry point
volatile unsigned char *ifs;

// load default system options after reset
void init_cold(void);

// initialisation of some things that are between cold and warm
void init_luke(void);

// initialise the hardware and platform related functions and structures
void init_warm(void);

// reset PIC32MZ
void sysreset(void);

// permanent UART console on RB6.RX and RB7.TX
void openCOM0(unsigned long bps, UART_LINE_CONTROL_MODE f);

// (re-)initialise the system SPI channel
// (mode): SPI mode 0...3
// (speed): SPI clock speed in Hz
void init_sys_SPI(unsigned char mode, unsigned long speed);

// transmit/receive data to/from SD/MMC via SPI2
unsigned char sys_SPI(unsigned char txd);

// blocking delay for given number of microseconds
void __attribute__ ((nomips16, nomicromips)) dlyus(unsigned long us);

// read the core clock counter
// the core clock counter is a free-running counter clocked at precisely 1MHz
#define cclk() ReadTimer89()

// random function making use of the hardware RNG
int rnd(void);

// real-time execution link to handling platform functions
void platform_link(void);

// return system clock frequency in MHz
unsigned short get_clock(void);

// set system clock frequency and return 0 if successful or -21 in case of invalid parameter
signed long set_clock(unsigned short freq);

// initialise the 8-bit bus control lines
// NOTE: does not initialise the default 8-bit CS# line
void DB8_init(void);

// write byte on the 8-bit bus
// (us) if not zero: cycle time in microseconds
void DB8_write(unsigned char data, unsigned char us);

// read byte from the 8-bit bus
// (us) if not zero: cycle time in microseconds
unsigned char DB8_read(unsigned char us);

// ===================================
// required basic I/O helper functions
// ===================================

int kbhit(void);
int __attribute__((used)) _mon_getc(int blocking);
void __attribute__((used)) _mon_putc(char ch);
void __attribute__((used)) _mon_puts(const char *s);
void __attribute__((used)) _mon_write (const char *s, unsigned int count);

// exception handler function
void __attribute__((nomips16)) _general_exception_handler(void);

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_H
