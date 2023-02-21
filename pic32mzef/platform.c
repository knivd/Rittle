#include <stdio.h>
#include <string.h>
#include "cfg_bits.h"
#include "uconsole.h"
#include "platform.h"

// macro to reserve flash memory for saving/loading data and initialise to 0xFF's
// note that (bytes) need to be a multiple of:
// - BYTE_PAGE_SIZE (and aligned that way) if you intend to erase
// - BYTE_ROW_SIZE (and aligned that way) if you intend to write rows
// - sizeof(int) if you intend to write words
#define ALLOCATE(name,align,bytes) const unsigned char name[(bytes)] __attribute__ ((aligned(align),space(prog),section(".ifs"))) = {[0 ...(bytes)-1]=0xff}
ALLOCATE(ifs_data,BYTE_PAGE_SIZE,(IFS_SIZE*1024));

volatile unsigned short xflags=0;
volatile unsigned short *console_rx=NULL;
volatile unsigned short console_rx_in=0;	// incoming character index
volatile unsigned short console_rx_out=0;	// outgoing character index
volatile unsigned long int_debounce=0;		// interrupt debounce counter


// permanent UART console on RB6.RX and RB7.TX
void openCOM0(unsigned long bps, UART_LINE_CONTROL_MODE f) {
	console_rx_in=console_rx_out=0;
	console_rx=NULL;
	xalloc((unsigned char **)&console_rx, (CONSOLE_RX_BUFFER*sizeof(short)));	// no error here
    PORTSetPinsDigitalIn(IOPORT_B, BIT_6);  // RB6 is permanently RX for the console
    PORTSetPinsDigitalOut(IOPORT_B, BIT_7); // RB7 is permanently TX for the console
    CNPUBSET=(BIT_6 | BIT_7);   // set pull-ups on the console port
    PPSUnLock;
    PPSOutput(3, RPB7, U6TX);
    PPSInput(4, U6RX, RPB6);
    PPSLock;
	UARTEnable(UART6, 0);
	UARTConfigure(UART6, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetLineControl(UART6, f);
    UARTSetDataRate(UART6, PBCLK2, bps);
	UARTSetFifoMode(UART6, UART_INTERRUPT_ON_RX_NOT_EMPTY);
	INTSetVectorPriority(INT_SOURCE_USART_6_RECEIVE, INT_PRIORITY_LEVEL_3);
	INTClearFlag(INT_SOURCE_USART_6_RECEIVE);
	INTEnable(INT_SOURCE_USART_6_RECEIVE, INT_ENABLED);
	UARTEnable(UART6, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
	xflags|=XFLAG_COM0;
}


void __attribute__ ((used)) __attribute__((nomips16,nomicromips))
	__attribute__((optimize("-O0"))) init_warm(void) {
	INTDisableInterrupts();

	// disable all run-time peripheral modules
	SpiChnEnable(SPI_CHANNEL1, FALSE);
	SpiChnEnable(SPI_CHANNEL3, FALSE);
	I2CEnable(I2C1, FALSE);
	I2CEnable(I2C5, FALSE);
	INTEnable(INT_SOURCE_USART_1_RECEIVE, INT_DISABLED);
	UARTEnable(UART1, 0);
	INTEnable(INT_SOURCE_USART_2_RECEIVE, INT_DISABLED);
	UARTEnable(UART2, 0);
	INTEnable(INT_SOURCE_USART_3_RECEIVE, INT_DISABLED);
	UARTEnable(UART3, 0);
    INTEnable(INT_SOURCE_USART_4_RECEIVE, INT_DISABLED);
	UARTEnable(UART4, 0);
	INTEnable(INT_SOURCE_USART_5_RECEIVE, INT_DISABLED);
	UARTEnable(UART5, 0);
	INTEnable(INT_SOURCE_USART_6_RECEIVE, INT_DISABLED);
	UARTEnable(UART6, 0);

	// reset RX buffers and call back functions
	xfree((unsigned char **)&console_rx);
	xfree((unsigned char **)&com_rx[0]);
	xfree((unsigned char **)&com_rx[1]);
	xfree((unsigned char **)&com_rx[2]);
	xfree((unsigned char **)&com_rx[3]);
    xfree((unsigned char **)&com_rx[4]);
	memset((unsigned char *)com_rx_in,0,sizeof(com_rx_in));
	memset((unsigned char *)com_rx_out,0,sizeof(com_rx_out));
	memset(&com_rx_callback,0,sizeof(com_rx_callback));
	memset(&spi_rx_callback,0,sizeof(spi_rx_callback));
	memset(&iic_rx_callback,0,sizeof(iic_rx_callback));
	//touch_InitPort=null_InitPort;
	//touch_Detach=null_Detach;
	//touch_ReadTouch=null_ReadTouch;
	touch_callback=0;
	touch_points=0;

	// close PWM ports
	CloseOC1();
	CloseOC2();
	CloseOC4();
	CloseOC5();
	CloseOC7();
	CloseOC8();
	CloseOC9();
	CloseTimer23();
	CloseTimer45();
	CloseTimer67();

	// reset the PMD registers and disable the hardware that is never used
	PMD1 = PMD2 = PMD3 = PMD4 = PMD5 = PMD6 = PMD7 = 0;
	PMD1|=BIT(12);	// comparator voltage reference
	PMD2|=(BIT(0) | BIT(1));	// comparators 1 and 2
	PMD3|=0x1ff;	// input capture 1..9
	PMD5|=(BIT(11) | BIT(12) | BIT(13));	// SPI4, SPI5, SPI6
	PMD5|=(BIT(17) | BIT(18) | BIT(19));	// I2C2, I2C3, I2C4
	PMD5|=(BIT(28) | BIT(29));	// CAN1 and 2
	PMD6|=(BIT(8) | BIT(9) | BIT(10) | BIT(11));	// reference output clocks
	PMD6|=BIT(16);	// PMP
	PMD6|=BIT(17);	// EBI
	PMD6|=BIT(23);	// SQI
	PMD6|=BIT(28);	// Ethernet
	PMD7|=BIT(4);	// DMA
	PMD7|=BIT(22);	// Crypto

	// full port reset
	TRISBSET = TRISCSET = TRISDSET = TRISESET = TRISFSET = TRISGSET = ULONG_MAX;
    ANSELBCLR = ANSELECLR = ANSELGCLR =  0;
    LATBCLR = LATCCLR = LATDCLR = LATECLR = LATFCLR = LATGCLR = 0;
    CNENBCLR = CNENCCLR = CNENDCLR = CNENECLR = CNENFCLR = CNENGCLR = 0;
    CNCONBCLR = CNCONCCLR = CNCONDCLR = CNCONECLR = CNCONFCLR = CNCONGCLR = 0;
    CNPUBCLR = CNPUCCLR = CNPUDCLR = CNPUECLR = CNPUFCLR = CNPUGCLR = 0;
    CNPDBCLR  = CNPDCCLR = CNPDDCLR = CNPDECLR = CNPDFCLR = CNPDGCLR = 0;

	// keep some pull-ups on
	CNPUBSET=BIT(13);	// LCD CS# pin
	CNPUCSET=BIT(13);	// Touch CS# pin
	CNPUDSET=(BIT(0) | BIT(9) | BIT(10));	// Wake# and the IIC1 pins
	CNPUESET=(BIT(2) | BIT(3) | BIT(4) |	// DBRD#, DBWR#, DBCS#
				BIT(5) | BIT(6) | BIT(7));	// SD1#, SD2#, EXP# pins
	CNPUFSET=(BIT(4) | BIT(5));				// the IIC2 pins

	// set slew rate for the ports
	SRCON0B = SRCON0E = SRCON0F = SRCON0G = 0;
	SRCON1B = SRCON1E = SRCON1F = SRCON1G = ULONG_MAX;

	// reserve the external oscillator SLEEP control output
	if(xflags & XFLAG_ECLK) {
		PORTSetPinsDigitalOut(IOPORT_C, BIT_15);
		LATCbits.LATC15=0;
	}

	// reinitialise driver port
	display_InitPort();

	// basic initialisation of the ADC
	ADC12Setup(ADC12_VREF_AVDD_AVSS, ADC12_CHARGEPUMP_DISABLE,
				ADC12_OUTPUT_DATA_FORMAT_INTEGER, FALSE,
				ADC12_FAST_SYNC_SYSTEM_CLOCK_DISABLE, ADC12_FAST_SYNC_PERIPHERAL_CLOCK_DISABLE,
				ADC12_INTERRUPT_BIT_SHIFT_LEFT_0_BITS, 0,
				ADC12_CLOCK_SOURCE_FRC, 1, ADC12_WARMUP_CLOCK_128);

	// initialise and open the serial console
    openCOM0(115200, (UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1));

	// open the USB console
	if(xflags & XFLAG_ECLK) {
		USBCSR0bits.SOFTCONN=1;
		initUSBConsole();
	}

	// missing clock warning
	if((xflags & XFLAG_ECLK)==0 && (xflags & XFLAG_ECLK_WARN)==0) {
		printf("WARNING: missing external clock\r\n\n");
		xflags|=XFLAG_ECLK_WARN;
	}

	// configure and enable interrupts
	INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
	INTEnableSystemMultiVectoredInt();
}


void __attribute__ ((used)) __attribute__((nomips16,nomicromips))
	__attribute__((optimize("-O0"))) init_luke(void) {
    mJTAGPortEnable(DEBUG_JTAGPORT_OFF);
	DisableWDT();
	set_clock(sys_freq_mhz);
	SysWaitStateConfig(1000000ul*sys_freq_mhz);
	SysPerformanceConfig((1000000ul*sys_freq_mhz), PCACHE_PREFETCH_ENABLE_ALL);

	SystemUnlock();
	CFGCONbits.IOLOCK=0;	// enable writing to the PPSx registers
	CFGCONbits.PMDLOCK=0;	// enable writing to the PMDx registers
	// see Errata #9
	// CFGCONbits.USBSSEN=1;	// shut down the USB in sleep mode
	CFGCONbits.OCACLK=1;	// OC outputs will use their own timers
	CFGCONbits.ICACLK=1;	// IC inputs will use their own timers
	CFGCONbits.IOANCPEN=0;	// disable the analogue port charge pump
	CFGCONbits.JTAGEN=0;	// disable JTAG
	CFGCONbits.TDOEN=0;		// disable TDO output
	CFGCONbits.TROEN=0;		// disable trace output
	OSCPBOutputClockDisable(OSC_PERIPHERAL_BUS_8);	// PBCLK8 is never needed
	SystemLock();

	PRISS = 0x76543210;		// assign shadow set #7-#1 to priority level #7-#1 ISRs

    // initialise and enable a free-running 32-bit Timer89 with clock exactly 1 MHz
    OpenTimer89((T89_ON | T89_IDLE_CON | T89_GATE_OFF | T89_32BIT_MODE_ON | T89_SOURCE_INT | T89_PS_1_64), 0xffffffff);
	WriteTimer89(0);

    // enable the true hardware random number generator
    RNGCONbits.TRNGEN = 1;

	// INT0
	PORTSetPinsDigitalIn(IOPORT_D, BIT_0);	// RD0 will be used as interrupt input
	CNPUDSET=BIT(0);		// enable the internal pull-up resistor on RD0
	xflags&=~(XFLAG_INT0 | XFLAG_TOUCH_IRQ);
	mINT0SetEdgeMode(0);	// configure INT0 for falling edge
	mINT0SetIntPriority(7);	// set highest priority
	mINT0ClearIntFlag();
	mINT0IntEnable(TRUE);
}


void __attribute__ ((used)) __attribute__((nomips16,nomicromips))
	__attribute__((optimize("-O0"))) init_cold(void) {
	INTDisableInterrupts();
	SystemUnlock();
	OSCCONbits.SLP2SPD=1;	// use FRC until the system clock is ready
	SystemLock();

	// TODO: check for present external clock before switching to it
	xflags |= XFLAG_ECLK;

	sys_freq_mhz=SYSCLK/1000000ul;
    init_luke();

	// initialise and run the RTCC (initially from the internal clock)
	// default time: 1 January 2020 (Wednesday), 00:00:00
	rtccTime tm;	// time structure
	rtccDate dt;	// date structure
	unsigned long t,t0,t1;

    RtccInit(RTCC_LPRC);                    // init the RTCC
//    RtccInit(RTCC_SOSC);                  // init the RTCC
	while(RtccGetClkStat()!=RTCC_CLK_ON);	// wait for the SOSC to be actually running and RTCC
                                            // to have its clock source could wait here 32ms at most
	t0=t1=RtccGetTime();
	t=1100;
	while(t-- && t0==t1) {
		t0=RtccGetTime();
		dlyus(1000);
	}
	if(t0==t1) {	// check if the RTCC clock has been initialised
		tm.l=0;
		dt.l=0;
		dt.wday=0x03;
		dt.mday=0x01;
		dt.mon=0x01;
		dt.year=0x20;
		RtccOpen(RTCC_SOSC, tm.l, dt.l, 0);	// try external crystal oscillator first
		t0=t1=RtccGetTime();
		t=1100;
		while(t-- && t0==t1) {
			t0=RtccGetTime();
			dlyus(1000);
		}
		if(t0==t1) RtccOpen(RTCC_LPRC, tm.l, dt.l, 0);	// fall back to internal oscillator
	}

	// other stuff
    xfree((unsigned char **)ramdisk);
    ifs=(volatile unsigned char *)ifs_data; // start address of the flash disk
    srand(RNGSEED2);
	enxcon=1;		// enable by default external console on attached display drivers
}


// console
void __ISR(_UART6_RX_VECTOR, ipl3auto) IntUart6Handler(void) {
	while(UARTReceivedDataIsAvailable(UART6)) {
		UART_DATA c=UARTGetData(UART6);
		#ifdef CONSOLE_ECHO
			_mon_putc(c.__data);
		#endif
		int e=(UARTGetLineStatus(UART6)&0b1110);
		U6STA&=~0b1110;
		if(!e && console_rx) {
			console_rx[console_rx_in++]=c.__data;
			if(console_rx_in>=CONSOLE_RX_BUFFER) console_rx_in=0;
		}
	}
	INTClearFlag(INT_SOURCE_USART_6_RECEIVE);
}


// COM1
void __ISR(_UART2_RX_VECTOR, ipl4auto) IntUart2Handler(void) {
	unsigned char port=0;	// COM1 uses UART2
	while(UARTReceivedDataIsAvailable(UART2)) {
		UART_DATA c=UARTGetData(UART2);
		int e=(UARTGetLineStatus(UART2)&0b1110);
		U2STA&=~0b1110;
		if(!e) {
			com_rx[port][com_rx_in[port]++]=c.__data;
			if(com_rx_in[port]>=COM_RX_BUFFER) com_rx_in[port]=0;
		}
	}
	INTClearFlag(INT_SOURCE_USART_2_RECEIVE);
}


// COM2
void __ISR(_UART3_RX_VECTOR, ipl4auto) IntUart3Handler(void) {
	unsigned char port=1;	// COM2 uses UART3
	while(UARTReceivedDataIsAvailable(UART3)) {
		UART_DATA c=UARTGetData(UART3);
		int e=(UARTGetLineStatus(UART3)&0b1110);
		U3STA&=~0b1110;
		if(!e) {
			com_rx[port][com_rx_in[port]++]=c.__data;
			if(com_rx_in[port]>=COM_RX_BUFFER) com_rx_in[port]=0;
		}
	}
	INTClearFlag(INT_SOURCE_USART_3_RECEIVE);
}


// COM3
void __ISR(_UART5_RX_VECTOR, ipl4auto) IntUart5Handler(void) {
	unsigned char port=2;	// COM3 uses UART5
	while(UARTReceivedDataIsAvailable(UART5)) {
		UART_DATA c=UARTGetData(UART5);
		int e=(UARTGetLineStatus(UART5)&0b1110);
		U5STA&=~0b1110;
		if(!e) {
			com_rx[port][com_rx_in[port]++]=c.__data;
			if(com_rx_in[port]>=COM_RX_BUFFER) com_rx_in[port]=0;
		}
	}
	INTClearFlag(INT_SOURCE_USART_5_RECEIVE);
}


// COM4
void __ISR(_UART1_RX_VECTOR, ipl4auto) IntUart1Handler(void) {
	unsigned char port=3;	// COM4 uses UART1
	while(UARTReceivedDataIsAvailable(UART1)) {
		UART_DATA c=UARTGetData(UART1);
		int e=(UARTGetLineStatus(UART1)&0b1110);
		U1STA&=~0b1110;
		if(!e) {
			com_rx[port][com_rx_in[port]++]=c.__data;
			if(com_rx_in[port]>=COM_RX_BUFFER) com_rx_in[port]=0;
		}
	}
	INTClearFlag(INT_SOURCE_USART_1_RECEIVE);
}


// COM5
void __ISR(_UART4_RX_VECTOR, ipl4auto) IntUart4Handler(void) {
	unsigned char port=4;	// COM5 uses UART4
	while(UARTReceivedDataIsAvailable(UART4)) {
		UART_DATA c=UARTGetData(UART4);
		int e=(UARTGetLineStatus(UART4)&0b1110);
		U1STA&=~0b1110;
		if(!e) {
			com_rx[port][com_rx_in[port]++]=c.__data;
			if(com_rx_in[port]>=COM_RX_BUFFER) com_rx_in[port]=0;
		}
	}
	INTClearFlag(INT_SOURCE_USART_4_RECEIVE);
}


// wake up from INT0
void __ISR_AT_VECTOR(_EXTERNAL_0_VECTOR, ipl7auto) Int0Handler(void) {
	if(((unsigned long)ReadTimer89()-int_debounce)>50000) {
		xflags|=XFLAG_INT0;
		if(xflags && XFLAG_TOUCH) xflags|=XFLAG_TOUCH_IRQ;
	}
	int_debounce=(unsigned long)ReadTimer89();
	mINT0ClearIntFlag();
}


// get character from the console without removing it from the buffer
// return the current character from the input buffer, or EOF in case the buffer is empty
int __attribute__ ((used)) kbhit(void) {
	if(console_rx_in!=console_rx_out) return console_rx[console_rx_out];
    else return 0;
}


// getch() hook
char __attribute__ ((used)) getch(void) {
    return _mon_getc(1);
}


// get character from the console
// (blocking) non-zero indicates that the function must be blocking and wait for character
// return the current character from the input buffer, or EOF in case the buffer is empty
int __attribute__ ((used)) _mon_getc(int blocking) {
	int ch=EOF;
	if(blocking && enxcon) display_PutChr(0);	// draw cursor
    while(blocking && (console_rx_in==console_rx_out) && ch==EOF) ch=serUSBGetC();
    if(ch>EOF ) {
        #ifdef CONSOLE_ECHO
			_mon_putc(ch);
        #endif
		#ifdef CONSOLE_DEBUG
			_mon_putc((((ch>>4)&15)>9)? ('7'+((ch>>4)&15)) : ('0'+((ch>>4)&15)));
			_mon_putc(((ch&15)>9)? ('7'+(ch&15)) : ('0'+(ch&15)));
        #endif
		return ch;
    }
    if(console_rx_in==console_rx_out) {
        if(!blocking) return EOF;
    }
    ch=console_rx[console_rx_out++];
    if(console_rx_out>=CONSOLE_RX_BUFFER) console_rx_out=0;
    return ch;
}


// write a character to the console
// (ch) character
void __attribute__ ((used)) _mon_putc(char ch) {
	if(xflags & XFLAG_COM0) {
		while(!(UARTTransmitterIsReady(UART6))) continue;
		UART_DATA ud;
		ud.__data=(UINT16)ch;
		UARTSendData(UART6, ud);
		while(!(UARTTransmissionHasCompleted(UART6))) continue;
	}
    SerUSBPutC(ch);					// output to USB console
	if(enxcon) display_PutChr(ch);	// output to the attached display device
}


// write a string to the console
// (s) string
void __attribute__ ((used)) _mon_puts(const char *s) {
    while(s && *s) _mon_putc((int)*(s++));
}


// write to the console
// (s) data
// (count) number of bytes
void __attribute__ ((used)) _mon_write (const char *s, unsigned int count) {
    while(count--) _mon_putc(*(s++));
}


// CPU exceptions handler
void __attribute__ ((nomips16)) _general_exception_handler(void)	{
    const static char *szException[] = {
        "Interrupt",                        // 0
        "Unknown",                          // 1
        "Unknown",                          // 2
        "Unknown",                          // 3
        "Address error (load or ifetch)",   // 4
        "Address error (store)",            // 5
        "Bus error (ifetch)",               // 6
        "Bus error (load/store)",           // 7
        "SysCall",                          // 8
        "Breakpoint",                       // 9
        "Reserved instruction",             // 10
        "Coprocessor unusable",             // 11
        "Arithmetic overflow",              // 12
        "Trap (possible divide by zero)",   // 13
        "Unknown",                          // 14
        "Unknown",                          // 15
        "Implementation specific 1",        // 16
        "CorExtend Unusable",               // 17
        "Coprocessor 2"                     // 18
    };
    volatile static unsigned long codeException;
    volatile static unsigned long addressException;
    const char *pszExcept;
    asm volatile ("mfc0 %0,$13" : "=r" (codeException));
    asm volatile ("mfc0 %0,$14" : "=r" (addressException));
    codeException=(codeException&0x7c)>>2;
    if(codeException<19) {
        pszExcept=szException[codeException];
        printf("\r\n\nCPU EXCEPTION: '%s' at address $%04lx\r\nRestarting...\r\n\n\n",pszExcept,addressException);
        int t=10000000;
        while(t--) asm volatile ("nop");    // just a little delay...
    }
    SoftReset();
}


// reset PIC32MZ
void __attribute__ ((used)) sysreset(void) {
    SoftReset();
}


// random generator using the TRNG in PIC32
int __attribute__ ((used)) rnd(void) {
    return (int)RNGSEED1;
}


// blocking delay for given number of microseconds
// Timer89 is free-running and clocked at 1MHz
void __attribute__ ((used)) __attribute__ ((nomips16, nomicromips)) dlyus(unsigned long us) {
    unsigned long c=(unsigned long)ReadTimer89();
    us+=c;
    while(c>us) c=(unsigned long)ReadTimer89();	// handle cases when (e) rolls over the counter
    while(c<us) c=(unsigned long)ReadTimer89();
}


// (re-)initialise the system SPI channel
void init_sys_SPI(unsigned char mode, unsigned long speed) {
	SpiChnEnable(SPI_CHANNEL2, FALSE);
	PORTSetPinsDigitalOut(IOPORT_G, (BIT_6 | BIT_8));   // RG6 is SCLK2 and RG8 is MOSI2
    PORTSetPinsDigitalIn(IOPORT_G, BIT_7);  // RG7 is MISO2
    PPSUnLock;
    PPSOutput(1, RPG8, SDO2);
    PPSInput(2, SDI2, RPG7);
    PPSLock;
    SpiOpenFlags f=(SPI_OPEN_MSTEN | SPI_OPEN_SMP_END | SPI_OPEN_MODE8 | SPI_OPEN_ON);
	if(mode==0) f|=SPI_OPEN_CKE_REV;
	else if(mode==3) f|=SPI_OPEN_CKP_HIGH;
	else if(mode==2) f|=(SPI_OPEN_CKP_HIGH | SPI_OPEN_CKE_REV);
	SpiChnOpen(SPI_CHANNEL2, f, (PBCLK2/speed));
    SpiChnEnable(SPI_CHANNEL2, TRUE);
}


// transmit/receive data to/from SD/MMC via SPI2
unsigned char sys_SPI(unsigned char txd) {
    SpiChnGetRov(SPI_CHANNEL2, TRUE);
    SpiChnPutC(SPI_CHANNEL2, txd);				// send data
    unsigned char rxd=SpiChnGetC(SPI_CHANNEL2);	// receive data
	return rxd;
}


void platform_link(void) {
	if(rvm[thd]->dsp) return;
	if(spi_rx_callback[0] && SPI1_nCS==0) call_addr(spi_rx_callback[0], rvm[thd]->pc);
	if(spi_rx_callback[1] && SPI2_nCS==0) call_addr(spi_rx_callback[1], rvm[thd]->pc);
	if(com_rx_callback[0]) call_addr(com_rx_callback[0], rvm[thd]->pc);
	if(com_rx_callback[1]) call_addr(com_rx_callback[1], rvm[thd]->pc);
	if(com_rx_callback[2]) call_addr(com_rx_callback[2], rvm[thd]->pc);
	if(com_rx_callback[3]) call_addr(com_rx_callback[3], rvm[thd]->pc);
    if(com_rx_callback[4]) call_addr(com_rx_callback[4], rvm[thd]->pc);
	if(xflags & XFLAG_TOUCH_IRQ) {
		if(touch_callback) {
			touch_ReadTouch();
			if(touch_points) call_addr(touch_callback, rvm[thd]->pc);
		}
		xflags&=~XFLAG_TOUCH_IRQ;
	}
}


// return system clock frequency
unsigned short get_clock(void) {
	return sys_freq_mhz;
}


// set system clock frequency in MHz and return 0 if successful or -21 in case of invalid parameter
signed long __attribute__((nomips16,nomicromips)) __attribute__((optimize("-O0"))) set_clock(unsigned short freq) {
	unsigned int mul=48;	// PLL multiplier value (multiplying input clock 8 MHz)
	OSC_SYSPLL_OUT_DIV div=OSC_SYSPLL_OUT_DIV_2;		// PLL divider value
	// reminder: peripheral clocks (especially PBCLK3) must be kept at 64 MHz
	OSC_PB_CLOCK_DIV_TYPE pdiv145=OSC_PB_CLOCK_DIV_2;	// divider for PBCLK1/4/5 (up to 100 MHz)
	OSC_PB_CLOCK_DIV_TYPE pdiv23=OSC_PB_CLOCK_DIV_3;	// divider for PBCLK2/3 (fixed 64 MHz)
	OSC_PB_CLOCK_DIV_TYPE pdiv7=OSC_PB_CLOCK_DIV_1;		// divider for PBCLK7
	if(freq==4) {
		mul=64;
		div=OSC_SYSPLL_OUT_DIV_8;
		pdiv145=OSC_PB_CLOCK_DIV_1;
		pdiv23=OSC_PB_CLOCK_DIV_1;
		pdiv7=OSC_PB_CLOCK_DIV_16;
	}
	else if(freq==16) {
		mul=64;
		div=OSC_SYSPLL_OUT_DIV_8;
		pdiv145=OSC_PB_CLOCK_DIV_1;
		pdiv23=OSC_PB_CLOCK_DIV_1;
		pdiv7=OSC_PB_CLOCK_DIV_4;
	}
	else if(freq==64) {
		mul=64;
		div=OSC_SYSPLL_OUT_DIV_8;
		pdiv145=OSC_PB_CLOCK_DIV_1;
		pdiv23=OSC_PB_CLOCK_DIV_1;
		pdiv7=OSC_PB_CLOCK_DIV_1;
	}
	else if(freq==128) {
		mul=64;
		div=OSC_SYSPLL_OUT_DIV_4;
		pdiv145=OSC_PB_CLOCK_DIV_2;
		pdiv23=OSC_PB_CLOCK_DIV_2;
		pdiv7=OSC_PB_CLOCK_DIV_1;
	}
	else if(freq==192) {
		mul=48;
		div=OSC_SYSPLL_OUT_DIV_2;
		pdiv145=OSC_PB_CLOCK_DIV_2;
		pdiv23=OSC_PB_CLOCK_DIV_3;
		pdiv7=OSC_PB_CLOCK_DIV_1;
	}
	else if(freq==256) {
		mul=64;
		div=OSC_SYSPLL_OUT_DIV_2;
		pdiv145=OSC_PB_CLOCK_DIV_3;
		pdiv23=OSC_PB_CLOCK_DIV_4;
		pdiv7=OSC_PB_CLOCK_DIV_1;
	}
	else return -21;
	OSCPLLClockUnlock();	// temporarily switch to FRC
	OSCClockSourceSwitch(OSC_FRC,
				OSC_SYSPLL_FREQ_RANGE_BYPASS,
				OSC_SYSPLL_IN_DIV_1,
				1, 1, TRUE);
	OSCPLLClockLock();
	SystemUnlock();
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_1, pdiv145);
	OscPBOutputClockEnable(OSC_PERIPHERAL_BUS_1);
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_2, pdiv23);
	OscPBOutputClockEnable(OSC_PERIPHERAL_BUS_2);
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_3, pdiv23);
	OscPBOutputClockEnable(OSC_PERIPHERAL_BUS_3);
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_4, pdiv145);
	OscPBOutputClockEnable(OSC_PERIPHERAL_BUS_4);
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_5, pdiv145);
	OscPBOutputClockEnable(OSC_PERIPHERAL_BUS_5);
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_7, pdiv7);
	OscPBOutputClockEnable(OSC_PERIPHERAL_BUS_7);
	OscPBClockDivisorSet(OSC_PERIPHERAL_BUS_8, pdiv145);	// not used but still have to make sure it is within limits
	SystemLock();
	unsigned long sysfreq=(8000000*mul)/(div+1);	// calculate the new SYSCLK from (mul) and (div) and PLL input clock 8 MHz
	SysWaitStateConfig(sysfreq);
	SysPerformanceConfig(sysfreq, PCACHE_PREFETCH_ENABLE_ALL);
	OSCPLLClockUnlock();
	if(xflags & XFLAG_ECLK) {
		OSCClockSourceSwitch(OSC_PRIMARY_WITH_PLL,
					OSC_SYSPLL_FREQ_RANGE_5M_TO_10M,
					OSC_SYSPLL_IN_DIV_3,
					mul, div, TRUE);
	}
	else {
		OSCClockSourceSwitch(OSC_FRC_WITH_PLL,
					OSC_SYSPLL_FREQ_RANGE_5M_TO_10M,
					OSC_SYSPLL_IN_DIV_1,
					mul, div, TRUE);
	}
	OSCPLLClockLock();
	sys_freq_mhz=freq;
    init_warm();
	return 0;
}


void DB8_init(void) {
	CNPUESET=(BIT_2 | BIT_3);
	PORTSetPinsDigitalOut(IOPORT_E, (BIT_2 | BIT_3));
	DB8_nRD=1;
	DB8_nWR=1;
}


void __attribute__((nomips16,nomicromips)) DB8_write(unsigned char data, unsigned char us) {
	PORTSetPinsDigitalOut(IOPORT_B, (BIT_8 | BIT_9 | BIT_10 | BIT_11 | BIT_12 | BIT_13 | BIT_14 | BIT_15));
	LATBCLR=(0xff00);	// clear bits 8..15
	LATBSET=((unsigned int)data << 8);
	DB8_nWR=0;
	if(us) dlyus(us);
	else {
		Nop();
		Nop();
		Nop();
	}
	DB8_nWR=1;
}


unsigned char __attribute__((nomips16,nomicromips)) DB8_read(unsigned char us) {
	PORTSetPinsDigitalIn(IOPORT_B, (BIT_8 | BIT_9 | BIT_10 | BIT_11 | BIT_12 | BIT_13 | BIT_14 | BIT_15));
	DB8_nRD=0;
	if(us) dlyus(us);
	else {
		Nop();
		Nop();
		Nop();
	}
	unsigned int r=PORTB;
	DB8_nRD=1;
	return (unsigned char)(r>>8);
}
