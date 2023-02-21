#include <stdio.h>
#include <stdlib.h>
#include "uconsole.h"
#include "addons.h"

// ports are numbered according to the schematic of the Rittle Board
// (equivalent to the actual pin numbering in PIC32MZ2048EFH064)
// .adcchn==2 is used to mark ports that could be reserved for external clock feed
const port_t portdef[PORTS] = {
{ 1,    IOPORT_E,   5,  17,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 2,    IOPORT_E,   6,  16,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 3,    IOPORT_E,   7,  15,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 4,    IOPORT_G,   6,  14,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 5,    IOPORT_G,   7,  13,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 6,    IOPORT_G,   8,  12,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 10,   IOPORT_G,   9,  11,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 11,   IOPORT_B,   5,  45,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 12,   IOPORT_B,   4,   4,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 13,   IOPORT_B,   3,   3,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 14,   IOPORT_B,   2,   2,   (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
{ 15,   IOPORT_B,   1,   1,   (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
{ 16,   IOPORT_B,   0,   0,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 17,   IOPORT_B,   6,  46,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 18,   IOPORT_B,   7,  47,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 21,   IOPORT_B,   8,  48,   (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
{ 22,   IOPORT_B,   9,  49,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 23,   IOPORT_B,  10,   5,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 24,   IOPORT_B,  11,   6,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 27,   IOPORT_B,  12,   7,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 28,   IOPORT_B,  13,   8,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 29,   IOPORT_B,  14,   9,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) },
{ 30,   IOPORT_B,  15,  10,   (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
{ 31,   IOPORT_C,  12,  -2,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 32,   IOPORT_C,  15,  -2,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 38,   IOPORT_F,   3,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
{ 41,   IOPORT_F,   4,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 42,   IOPORT_F,   5,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 43,   IOPORT_D,   9,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 44,   IOPORT_D,  10,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 45,   IOPORT_D,  11,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
{ 46,   IOPORT_D,   0,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
{ 47,   IOPORT_C,  13,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 48,   IOPORT_C,  14,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 49,   IOPORT_D,   1,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 50,   IOPORT_D,   2,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 51,   IOPORT_D,   3,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 52,   IOPORT_D,   4,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 53,   IOPORT_D,   5,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 56,   IOPORT_F,   0,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 57,   IOPORT_F,   1,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 58,   IOPORT_E,   0,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 61,   IOPORT_E,   1,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 62,   IOPORT_E,   2,  -1,   (PFN_DOUT | PFN_DIN |                     PFN_PUP | PFN_PDN | PFN_OD) },
{ 63,   IOPORT_E,   3,  -1,   (PFN_DOUT | PFN_DIN |			            PFN_PUP | PFN_PDN | PFN_OD) },
{ 64,   IOPORT_E,   4,  18,   (PFN_DOUT | PFN_DIN | PFN_AIN |           PFN_PUP | PFN_PDN | PFN_OD) }
};


void __port(void) {
	if(rvm[thd]->dsp>1) {
		rdata_t d1,d2;
		pullfifo(&d2);
		if(d2.type!=rTEXT) {
			xv=-23;
			return;
		}
		while(rvm[thd]->dsp) {
			pullfifo(&d1);
            if(d1.type!=rSINT64) {
				xv=-23;
				return;
			}
			unsigned short p=0;
			while(p<PORTS && portdef[p].pnum!=d1.data.sint64) p++;
			if(p>=PORTS || d2.data.text==NULL) {
				xv=-3;	// argument out of range (i.e. unable to find the port number)
				return;
			}
			if((portdef[p].adcchn==-2) && (xflags & XFLAG_ECLK)) {
				xv=-21;	// accessing ports that are reserved for external clock is not supported
				return;
			}
			PORTResetPins(portdef[p].port, BIT(portdef[p].pbit));
			char *f=d2.data.text;
			while(*f && !xv) {
				if(*f==0) break;		// void function will simply reset the port

				else if(*f=='+') {		// switch pull-up
					if(portdef[p].func & PFN_PUP) {
						f++;
						switch(portdef[p].port) {
							#if defined _PORTA
								case IOPORT_A: CNPUASET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTB
								case IOPORT_B: CNPUBSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTC
								case IOPORT_C: CNPUCSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTD
								case IOPORT_D: CNPUDSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTE
								case IOPORT_E: CNPUESET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTF
								case IOPORT_F: CNPUFSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTG
								case IOPORT_G: CNPUGSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTH
								case IOPORT_H: CNPUHSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTJ
								case IOPORT_J: CNPUJSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTK
								case IOPORT_K: CNPUKSET=BIT(portdef[p].pbit); break;
							#endif
						}
					} else xv=-23;
				}

				else if(*f=='-') {	// switch pull-down
					if(portdef[p].func & PFN_PDN) {
						f++;
						switch(portdef[p].port) {
							#if defined _PORTA
								case IOPORT_A: CNPDASET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTB
								case IOPORT_B: CNPDBSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTC
								case IOPORT_C: CNPDCSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTD
								case IOPORT_D: CNPDDSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTE
								case IOPORT_E: CNPDESET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTF
								case IOPORT_F: CNPDFSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTG
								case IOPORT_G: CNPDGSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTH
								case IOPORT_H: CNPDHSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTJ
								case IOPORT_J: CNPDJSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTK
								case IOPORT_K: CNPDKSET=BIT(portdef[p].pbit); break;
							#endif
						}
					} else xv=-23;
				}

				else if(*f=='#') {	// switch open-drain
					if(portdef[p].func & PFN_OD) {
						f++;
						switch(portdef[p].port) {
							#if defined _PORTA
								case IOPORT_A: ODCASET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTB
								case IOPORT_B: ODCBSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTC
								case IOPORT_C: ODCCSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTD
								case IOPORT_D: ODCDSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTE
								case IOPORT_E: ODCESET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTF
								case IOPORT_F: ODCFSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTG
								case IOPORT_G: ODCGSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTH
								case IOPORT_H: ODCHSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTJ
								case IOPORT_J: ODCJSET=BIT(portdef[p].pbit); break;
							#endif
							#if defined _PORTK
								case IOPORT_K: ODCKSET=BIT(portdef[p].pbit); break;
							#endif
						}
					} else xv=-23;
				}

				else break;	// unrecognised switch - proceed further
			}
			if(xv) break;

			if(!strcmp("DIN",f) && (portdef[p].func & PFN_DIN)) {
				PORTSetPinsDigitalIn(portdef[p].port, BIT(portdef[p].pbit));
			}

			else if(!strcmp("DOUT",f) && (portdef[p].func & PFN_DOUT)) {
				PORTSetPinsDigitalOut(portdef[p].port, BIT(portdef[p].pbit));
			}

			else if(!strcmp("AIN",f) && (portdef[p].func & PFN_AIN)) {
				PORTSetPinsAnalogIn(portdef[p].port, BIT(portdef[p].pbit));
				ADC12SelectAnalogInputMode(portdef[p].adcchn, ADC12_INPUT_MODE_SINGLE_ENDED_UNSIGNED);
			}

			else if(!strncmp("PWM",f,3) && (portdef[p].func & PFN_PWM)) {	// PWM functions use OC8 and OC9
				skip(&f,3);
				if(*f!=':') {	// the expected format is "PWM:frequency"
					xv=-24;
					return;
				}
				skip(&f,1);
				long freq=(signed long)getnum(&f);
				if(freq<0 || freq>PBCLK3) {
					xv=-23;
					return;
				}
				PORTSetPinsDigitalOut(portdef[p].port, BIT(portdef[p].pbit));
				if(freq>0) freq=(PBCLK3/freq);
				// group 1 on Timer23
				if(portdef[p].pnum==15) {
					if(freq>0) {	// enable PWM on the port
						PPSUnLock;
						PPSOutput(2, RPB1, OC4);
						PPSLock;
						if((T2CON & T_ON)==0) {
							OpenTimer23((T23_ON | T23_IDLE_CON | T23_GATE_OFF | T23_32BIT_MODE_ON | T23_PS_1_1), freq);
							WriteTimer23(0);
						}
						OpenOC4((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC |
									OC_CONTINUE_PULSE), freq, freq/2);
					}
					else PPSOutput(2, RPB1, NULL);	// disable PWM on the port (only)

				}
				else if(portdef[p].pnum==21) {
					if(freq>0) {	// enable PWM on the port
						PPSUnLock;
						PPSOutput(3, RPB8, OC5);
						PPSLock;
						if((T2CON & T_ON)==0) {
							OpenTimer23((T23_ON | T23_IDLE_CON | T23_GATE_OFF | T23_32BIT_MODE_ON | T23_PS_1_1), freq);
							WriteTimer23(0);
						}
						OpenOC5((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC |
									OC_CONTINUE_PULSE), freq, freq/2);
					}
					else PPSOutput(3, RPB8, NULL);	// disable PWM on the port (only)
				}
				// group2 on Timer45
				else if(portdef[p].pnum==14) {
					if(freq>0) {	// enable PWM on the port
						PPSUnLock;
						PPSOutput(4, RPB2, OC1);
						PPSLock;
						if((T4CON & T_ON)==0) {
							OpenTimer45((T45_ON | T45_IDLE_CON | T45_GATE_OFF | T45_32BIT_MODE_ON | T45_PS_1_1), freq);
							WriteTimer45(0);
						}
						OpenOC1((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC |
									OC_CONTINUE_PULSE), freq, freq/2);
					}
					else PPSOutput(4, RPB2, NULL);	// disable PWM on the port (only)
				}
				else if(portdef[p].pnum==38) {
					if(freq>0) {	// enable PWM on the port
						PPSUnLock;
						PPSOutput(4, RPF3, OC2);
						PPSLock;
						if((T4CON & T_ON)==0) {
							OpenTimer45((T45_ON | T45_IDLE_CON | T45_GATE_OFF | T45_32BIT_MODE_ON | T45_PS_1_1), freq);
							WriteTimer45(0);
						}
						OpenOC2((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC |
									OC_CONTINUE_PULSE), freq, freq/2);
					}
					else PPSOutput(4, RPF3, NULL);	// disable PWM on the port (only)
				}
				// group 3 on Timer67
				else if(portdef[p].pnum==30) {
					if(freq>0) {	// enable PWM on the port
						PPSUnLock;
						PPSOutput(3, RPB15, OC8);
						PPSLock;
						if((T6CON & T_ON)==0) {
							OpenTimer67((T67_ON | T67_IDLE_CON | T67_GATE_OFF | T67_32BIT_MODE_ON | T67_PS_1_1), freq);
							WriteTimer67(0);
						}
						OpenOC8((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC |
									OC_CONTINUE_PULSE), freq, freq/2);
					}
					else PPSOutput(3, RPB15, NULL);	// disable PWM on the port (only)
				}
				else if(portdef[p].pnum==45) {
					if(freq>0) {	// enable PWM on the port
						PPSUnLock;
						PPSOutput(2, RPD11, OC7);
						PPSLock;
						if((T6CON & T_ON)==0) {
							OpenTimer67((T67_ON | T67_IDLE_CON | T67_GATE_OFF | T67_32BIT_MODE_ON | T67_PS_1_1), freq);
							WriteTimer67(0);
						}
						OpenOC7((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC |
									OC_CONTINUE_PULSE), freq, freq/2);
					}
					else PPSOutput(2, RPD11, NULL);	// disable PWM on the port (only)
				}
				else if(portdef[p].pnum==46) {
					if(freq>0) {	// enable PWM on the port
						PPSUnLock;
						PPSOutput(4, RPD0, OC9);
						PPSLock;
						if((T6CON & T_ON)==0) {
							OpenTimer67((T67_ON | T67_IDLE_CON | T67_GATE_OFF | T67_32BIT_MODE_ON | T67_PS_1_1), freq);
							WriteTimer67(0);
						}
						OpenOC9((OC_ON | OC_IDLE_CON | OC_TIMER_MODE32 | OC_TIMER2_SRC |
									OC_CONTINUE_PULSE), freq, freq/2);
					}
					else PPSOutput(4, RPD0, NULL);	// disable PWM on the port (only)
				}
			}

			else xv=-23;	// invalid parameter (i.e. could not recognise the specified port function)
			if(xv) break;
		}
	}
	else xv=-24;
}


void __DOUT(void) {
	if(rvm[thd]->dsp>1) {
		rdata_t d1,d2;
		pullfifo(&d2);	// value
		if(d2.type!=rSINT64 || d2.data.sint64<0 || d2.data.sint64>1) {
			xv=-23;
			return;
		}
		while(rvm[thd]->dsp) {
			pullfifo(&d1);	// port
            if(d1.type!=rSINT64) {
				xv=-23;
				return;
			}
			unsigned short p=0;
			while(p<PORTS && portdef[p].pnum!=d1.data.sint64) p++;
			if(p>=PORTS) {
				xv=-3;	// argument out of range (i.e. unable to find the port number)
				return;
			}
			if(d2.data.sint64) PORTSetBits(portdef[p].port, BIT(portdef[p].pbit));
			else PORTClearBits(portdef[p].port, BIT(portdef[p].pbit));
		}
	}
	else xv=-24;
}


void __DTOG(void) {
	if(rvm[thd]->dsp) {
		rdata_t d1;
		while(rvm[thd]->dsp) {
			pullfifo(&d1);	// port
            if(d1.type!=rSINT64) {
				xv=-23;
				return;
			}
			unsigned short p=0;
			while(p<PORTS && portdef[p].pnum!=d1.data.sint64) p++;
			if(p>=PORTS) {
				xv=-3;	// argument out of range (i.e. unable to find the port number)
				return;
			}
			PORTToggleBits(portdef[p].port, BIT(portdef[p].pbit));
		}
	}
	else xv=-24;
}


void __DIN(void) {
	rdata_t d1;
	pullu(&d1);	// port
	if(xv) return;
	unsigned short p=0;
	while(p<PORTS && portdef[p].pnum!=d1.data.sint64) p++;
	if(p>=PORTS) {
		xv=-3;	// argument out of range (i.e. unable to find the port number)
		return;
	}
	d1.data.sint64=!!(PORTReadBits(portdef[p].port, BIT(portdef[p].pbit)));
	d1.type=rSINT8;
	push(&d1);
}


void __AIN(void) {
	rdata_t d1;
	pullu(&d1);	// port
	if(xv) return;
	unsigned short p=0;
	while(p<PORTS && portdef[p].pnum!=d1.data.sint64) p++;
	if(p>=PORTS) {
		xv=-3;	// argument out of range (i.e. unable to find the port number)
		return;
	}
	EnableADC12();
	SetChanADC12(portdef[p].adcchn);
    ADC12SetupChannel(ADC12GetChannel(portdef[p].adcchn),
						ADC12_DATA_12BIT, 2, 14, ADC12_EARLY_INTERRUPT_PRIOR_CLOCK_1);
	OpenADC12(portdef[p].adcchn);
	dlyus(10);
	unsigned short v=ReadADC12(portdef[p].adcchn);
    CloseADC12();
	d1.data.real=(double)v/4096.0;
	d1.type=rREAL;
	push(&d1);
}


void __setPWM(void) {
	rdata_t d1,d2;
	pullu(&d1);	// port
	pullr(&d2);	// duty
	if(xv) return;
	unsigned short p=0;
	while(p<PORTS && portdef[p].pnum!=d1.data.sint64) p++;
	if(p>=PORTS || d2.data.real<0 || d2.data.real>1) {
		xv=-3;	// argument out of range (i.e. unable to find the port number)
		return;
	}
	if((portdef[p].func & PFN_PWM)==0) {
		xv=-21;
		return;
	}
	d2.data.real=1.0-d2.data.real;
	if(d1.data.sint64==14) SetPulseOC1((unsigned long)(d2.data.real*(double)ReadDCOC1PWM()), ReadDCOC1PWM());
	else if(d1.data.sint64==15) SetPulseOC4((unsigned long)(d2.data.real*(double)ReadDCOC4PWM()), ReadDCOC4PWM());
	else if(d1.data.sint64==21) SetPulseOC5((unsigned long)(d2.data.real*(double)ReadDCOC5PWM()), ReadDCOC5PWM());
	else if(d1.data.sint64==30) SetPulseOC8((unsigned long)(d2.data.real*(double)ReadDCOC8PWM()), ReadDCOC8PWM());
	else if(d1.data.sint64==38) SetPulseOC2((unsigned long)(d2.data.real*(double)ReadDCOC2PWM()), ReadDCOC2PWM());
	else if(d1.data.sint64==45) SetPulseOC7((unsigned long)(d2.data.real*(double)ReadDCOC7PWM()), ReadDCOC7PWM());
	else if(d1.data.sint64==46) SetPulseOC9((unsigned long)(d2.data.real*(double)ReadDCOC9PWM()), ReadDCOC9PWM());
	else xv=-21;
}


void __Vref(void) {
	rdata_t d1;
	pullt(&d1);
	if(xv) return;
	if(!strcmp("Vdd/Vss",d1.data.text)) {
		ADC12Setup(ADC12_VREF_AVDD_AVSS, ADC12_CHARGEPUMP_DISABLE,
				ADC12_OUTPUT_DATA_FORMAT_INTEGER, FALSE,
				ADC12_FAST_SYNC_SYSTEM_CLOCK_DISABLE, ADC12_FAST_SYNC_PERIPHERAL_CLOCK_DISABLE,
				ADC12_INTERRUPT_BIT_SHIFT_LEFT_0_BITS, 0,
				ADC12_CLOCK_SOURCE_FRC, 1, ADC12_WARMUP_CLOCK_128);
	}
	else if(!strcmp("Vref+/Vss",d1.data.text)) {
		PORTSetPinsAnalogIn(IOPORT_B, BIT(0));	// RB0 is Vref+
		ADC12Setup(ADC12_VREF_EXTVREFP_AVSS, ADC12_CHARGEPUMP_DISABLE,
				ADC12_OUTPUT_DATA_FORMAT_INTEGER, FALSE,
				ADC12_FAST_SYNC_SYSTEM_CLOCK_DISABLE, ADC12_FAST_SYNC_PERIPHERAL_CLOCK_DISABLE,
				ADC12_INTERRUPT_BIT_SHIFT_LEFT_0_BITS, 0,
				ADC12_CLOCK_SOURCE_FRC, 1, ADC12_WARMUP_CLOCK_128);
	}
	else if(!strcmp("Vdd/Vref-",d1.data.text)) {
		PORTSetPinsAnalogIn(IOPORT_B, BIT(1));	// RB1 is Vref-
		ADC12Setup(ADC12_VREF_AVDD_EXTVREFN, ADC12_CHARGEPUMP_DISABLE,
				ADC12_OUTPUT_DATA_FORMAT_INTEGER, FALSE,
				ADC12_FAST_SYNC_SYSTEM_CLOCK_DISABLE, ADC12_FAST_SYNC_PERIPHERAL_CLOCK_DISABLE,
				ADC12_INTERRUPT_BIT_SHIFT_LEFT_0_BITS, 0,
				ADC12_CLOCK_SOURCE_FRC, 1, ADC12_WARMUP_CLOCK_128);
	}
	else if(!strcmp("Vref+/Vref-",d1.data.text)) {
		PORTSetPinsAnalogIn(IOPORT_B, (BIT(0) | BIT(1)));	// RB0 is Vref+ and RB1 is Vref-
		ADC12Setup(ADC12_VREF_EXTVREFP_EXTVREFN, ADC12_CHARGEPUMP_DISABLE,
				ADC12_OUTPUT_DATA_FORMAT_INTEGER, FALSE,
				ADC12_FAST_SYNC_SYSTEM_CLOCK_DISABLE, ADC12_FAST_SYNC_PERIPHERAL_CLOCK_DISABLE,
				ADC12_INTERRUPT_BIT_SHIFT_LEFT_0_BITS, 0,
				ADC12_CLOCK_SOURCE_FRC, 1, ADC12_WARMUP_CLOCK_128);
	}
	else xv=-23;	// invalid parameter
}


void __enable(void) {
	rdata_t d1, d2;
	pullfifo(&d1);	// interface
	pullfifo(&d2);	// parameters
	if(d1.type!=rTEXT || d2.type!=rTEXT) {
		xv=-23;
		return;
	}

	// SPI interfaces
	// parse d2.data.text parameter: "M/S, bps [,0/1/2/3 mode] [,8/16/32 bits]"
	if(!strcmp("SPI1",d1.data.text) || !strcmp("SPI2",d1.data.text)) {
		char *p=d2.data.text;
		skip(&p,0);
		if(*p!='M' && *p!='S') {	// master or slave mode must be defined
			xv=-23;
			return;
		}
		char role=*p;
		skip(&p,1);
		long bps=1;
		char mode=0, wlen=0;
		if(role=='M') {
			bps=(signed long)getnum(&p);	// the 'bps' parameter is read only for Master
			skip(&p,0);
		}
		if(*p==',') {	// SPI mode is defined
			skip(&p,1);
			mode=(char)getnum(&p);
			skip(&p,0);
			if(*p==',') {	// word length is defined
				skip(&p,1);
				wlen=(char)getnum(&p);
				skip(&p,0);
			} else wlen=8;
		} else mode=0;
		if(*p || bps<1 || bps>PBCLK2 || mode<0 || mode>3 || (wlen!=8 && wlen!=16 && wlen!=32)) {
			xv=-23;
			return;
		}
		if(role=='S') {	// check for call back function in slave mode
			if(rvm[thd]->dsp) {		// check for @callback parameter
				pullfifo(&d2);		// pull callback function reference
				if(d2.type!=rFUNC) {
					xv=-23;
					return;
				}
				d2.data.sint64&=ULONG_MAX;
			} else d2.data.sint64=0;	// no callback
		} else d2.data.sint64=0;		// no callback
		if(!strcmp("SPI1",d1.data.text)) {	// SPI1 is using PIC32's SPI1
			SpiOpenFlags f=0;
			if(wlen==32) f|=SPI_OPEN_MODE32;
			else if(wlen==16) f|=SPI_OPEN_MODE16;
			else f|=SPI_OPEN_MODE8;
			if(mode==0) f|=SPI_OPEN_CKE_REV;
			else if(mode==2) f|=(SPI_OPEN_CKP_HIGH | SPI_OPEN_CKE_REV);
			else if(mode==3) f|=SPI_OPEN_CKP_HIGH;
			PORTSetPinsDigitalOut(IOPORT_D, (BIT_1 | BIT_3));   // RD1 is SCLK1 and RD3 is MOSI1
			PORTSetPinsDigitalIn(IOPORT_D, BIT_2);  // RD2 is MISO1
			PPSUnLock;
			PPSOutput(2, RPD3, SDO1);
			PPSInput(1, SDI1, RPD2);
			PPSLock;
			SpiChnOpen(SPI_CHANNEL1, (f | SPI_OPEN_MSTEN | SPI_OPEN_SMP_END | SPI_OPEN_ON), (PBCLK2/bps));
			SpiChnEnable(SPI_CHANNEL1, TRUE);
			if(role=='S') {
				spi_rx_callback[0]=(unsigned long)d2.data.sint64;
				PORTSetPinsDigitalIn(IOPORT_E, BIT_0);  // SPI1 slave nCS
			}
		}
		else if(!strcmp("SPI2",d1.data.text)) {	// SPI2 is using PIC32's SPI3
			SpiOpenFlags f=0;
			if(wlen==32) f|=SPI_OPEN_MODE32;
			else if(wlen==16) f|=SPI_OPEN_MODE16;
			else f|=SPI_OPEN_MODE8;
			if(mode==0) f|=SPI_OPEN_CKE_REV;
			else if(mode==2) f|=(SPI_OPEN_CKP_HIGH | SPI_OPEN_CKE_REV);
			else if(mode==3) f|=SPI_OPEN_CKP_HIGH;
			PORTSetPinsDigitalOut(IOPORT_B, (BIT_10 | BIT_14));	// RB14 is SCLK3 and RB10 is MOSI3
			PORTSetPinsDigitalIn(IOPORT_B, BIT_9);  // RB9 is MISO3
			PPSUnLock;
			PPSOutput(1, RPB10, SDO3);
			PPSInput(1, SDI3, RPB9);
			PPSLock;
			SpiChnOpen(SPI_CHANNEL3, (f | SPI_OPEN_MSTEN | SPI_OPEN_SMP_END | SPI_OPEN_ON), (PBCLK2/bps));
			SpiChnEnable(SPI_CHANNEL3, TRUE);
			spi_rx_callback[1]=(unsigned long)d2.data.sint64;
			if(role=='S') {
				spi_rx_callback[1]=(unsigned long)d2.data.sint64;
				PORTSetPinsDigitalIn(IOPORT_B, BIT_13);	// SPI2 slave nCS
			}
		}
	}

	// I2C interfaces
	// parse d2.data.text parameter: "M/S, bps"
	else if(!strcmp("IIC1",d1.data.text) || !strcmp("IIC2",d1.data.text)) {
		char *p=d2.data.text;
		skip(&p,0);
		if(*p!='M' && *p!='S') {	// master or slave mode must be defined
			xv=-23;
			return;
		}
		char role=*p;
		skip(&p,1);
		long bps=1;
		if(role=='M') {
			if(*p!=',') {
				xv=-24;
				return;
			}
			skip(&p,1);
			bps=(signed long)getnum(&p);	// the 'bps' parameter is read only for Master
			skip(&p,0);
		}
		if(*p || bps<1 || bps>5000000) {
			xv=-23;
			return;
		}
		if(role=='S') {	// check for call back function in slave mode
			if(rvm[thd]->dsp) {		// check for @callback parameter
				pullfifo(&d2);		// pull callback function reference
				if(d2.type!=rFUNC) {
					xv=-23;
					return;
				}
				d2.data.sint64&=ULONG_MAX;
			} else d2.data.sint64=0;	// no callback
		} else d2.data.sint64=0;		// no callback
		if(!strcmp("IIC1",d1.data.text)) {		// IIC1 is using PIC32's I2C1
			CNPUDSET=(BIT_9 | BIT_10);	// enable the built-in pull-ups (in case there are none outside)
			I2CSetFrequency(I2C1, PBCLK2, bps);
			I2CConfigure(I2C1, (I2C_ENABLE_SLAVE_CLOCK_STRETCHING | I2C_ENABLE_HIGH_SPEED));
			I2CEnable(I2C1, TRUE);
			if(role=='S') iic_rx_callback[0]=(unsigned long)d2.data.sint64;
		}
		else if(!strcmp("IIC2",d1.data.text)) {	// IIC2 is using PIC32's I2C5
			CNPUFSET=(BIT_4 | BIT_5);   // enable the built-in pull-ups (in case there are none outside)
			I2CSetFrequency(I2C5, PBCLK2, bps);
			I2CConfigure(I2C5, (I2C_ENABLE_SLAVE_CLOCK_STRETCHING | I2C_ENABLE_HIGH_SPEED));
			I2CEnable(I2C5, TRUE);
			if(role=='S') iic_rx_callback[1]=(unsigned long)d2.data.sint64;
		}
	}

	// UART interfaces
	// parse d2.data.text parameter: "bps, [8/9] [N/E/O] [1/2]", @rx_callback
	else if(!strcmp("COM0",d1.data.text)
		|| !strcmp("COM1",d1.data.text) || !strcmp("COM2",d1.data.text)
		|| !strcmp("COM3",d1.data.text) || !strcmp("COM4",d1.data.text)
        || !strcmp("COM5",d1.data.text)) {
		char *p=d2.data.text;
		skip(&p,0);
		long bps=(long)getnum(&p);
		skip(&p,0);
		char wlen='8';
		char par='N';
		char stop='1';
		if(*p==',') {	// protocol is defined
			skip(&p,1);
			wlen=*p;
			if(wlen!='8' && wlen!='9') {
				xv=-23;
				return;
			}
			skip(&p,1);
			par=*p;
			if(par!='N' && par!='E' && par!='O') {
				xv=-23;
				return;
			}
			skip(&p,1);
			stop=*p;
			if(stop!='1' && stop!='2') {
				xv=-23;
				return;
			}
			skip(&p,1);
		}
		if(*p || bps<1 || bps>25000000) {
			xv=-23;
			return;
		}
		UART_LINE_CONTROL_MODE f=0;
		if(wlen=='9') f|=UART_DATA_SIZE_9_BITS;
		else f|=UART_DATA_SIZE_8_BITS;
		if(par=='E') f|=UART_PARITY_EVEN;
		else if(par=='O') f|=UART_PARITY_ODD;
		else f|=UART_PARITY_NONE;
		if(stop=='2') f|=UART_STOP_BITS_2;
		else f|=UART_STOP_BITS_1;
		if(rvm[thd]->dsp) {		// check for @callback parameter
			pullfifo(&d2);		// pull callback function reference
			if(d2.type!=rFUNC) {
				xv=-23;
				return;
			}
			d2.data.sint64&=ULONG_MAX;
		} else d2.data.sint64=0;	// no callback
		if(!strcmp("COM0",d1.data.text)) openCOM0(bps,f);	// COM0 (console) is using PIC32's UART6
		else if(!strcmp("COM1",d1.data.text)) {				// COM1 is using PIC32's UART2
			PORTSetPinsDigitalIn(IOPORT_D, BIT_4);  // RD4 is RX for COM1
			PORTSetPinsDigitalOut(IOPORT_D, BIT_5); // RD5 is TX for COM1
			CNPUDSET=(BIT_4 | BIT_5);   // set pull-ups
			PPSUnLock;
			PPSOutput(4, RPD5, U2TX);
			PPSInput(3, U2RX, RPD4);
			PPSLock;
			com_rx[0]=NULL;
			xalloc((unsigned char **)&com_rx[0], (COM_RX_BUFFER*sizeof(short)));
			if(!com_rx[0]) xv=-7;
			com_rx_in[0]=com_rx_out[0]=0;
			UARTConfigure(UART2, UART_ENABLE_PINS_TX_RX_ONLY);
			UARTSetLineControl(UART2, f);
			UARTSetDataRate(UART2, PBCLK2, bps);
			UARTSetFifoMode(UART2, UART_INTERRUPT_ON_RX_NOT_EMPTY);
			INTSetVectorPriority(INT_SOURCE_USART_2_RECEIVE, INT_PRIORITY_LEVEL_4);
			INTClearFlag(INT_SOURCE_USART_2_RECEIVE);
			INTEnable(INT_SOURCE_USART_2_RECEIVE, INT_ENABLED);
			UARTEnable(UART2, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
			com_rx_callback[0]=(unsigned long)d2.data.sint64;
		}
		else if(!strcmp("COM2",d1.data.text)) {		// COM2 is using PIC32's UART3
			PORTSetPinsDigitalIn(IOPORT_F, BIT_0);  // RF0 is RX for COM2
			PORTSetPinsDigitalOut(IOPORT_F, BIT_1); // RF1 is TX for COM2
			CNPUFSET=(BIT_0 | BIT_1);   // set pull-ups
			PPSUnLock;
			PPSOutput(1, RPF1, U3TX);
			PPSInput(2, U3RX, RPF0);
			PPSLock;
			com_rx[1]=NULL;
			xalloc((unsigned char **)&com_rx[1], (COM_RX_BUFFER*sizeof(short)));
			if(!com_rx[1]) xv=-7;
			com_rx_in[1]=com_rx_out[1]=0;
			UARTConfigure(UART3, UART_ENABLE_PINS_TX_RX_ONLY);
			UARTSetLineControl(UART3, f);
			UARTSetDataRate(UART3, PBCLK2, bps);
			UARTSetFifoMode(UART3, UART_INTERRUPT_ON_RX_NOT_EMPTY);
			INTSetVectorPriority(INT_SOURCE_USART_3_RECEIVE, INT_PRIORITY_LEVEL_4);
			INTClearFlag(INT_SOURCE_USART_3_RECEIVE);
			INTEnable(INT_SOURCE_USART_3_RECEIVE, INT_ENABLED);
			UARTEnable(UART3, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
			com_rx_callback[1]=(unsigned long)d2.data.sint64;
		}
		else if(!strcmp("COM3",d1.data.text)) {		// COM3 is using PIC32's UART5
			PORTSetPinsDigitalIn(IOPORT_F, BIT_4);  // RF4 is RX for COM3
			PORTSetPinsDigitalOut(IOPORT_F, BIT_5); // RF5 is TX for COM3
			CNPUFSET=(BIT_4 | BIT_5);   // set pull-ups
			PPSUnLock;
			PPSOutput(2, RPF5, U5TX);
			PPSInput(1, U5RX, RPF4);
			PPSLock;
			com_rx[2]=NULL;
			xalloc((unsigned char **)&com_rx[2], (COM_RX_BUFFER*sizeof(short)));
			if(!com_rx[2]) xv=-7;
			com_rx_in[2]=com_rx_out[2]=0;
			UARTConfigure(UART5, UART_ENABLE_PINS_TX_RX_ONLY);
			UARTSetLineControl(UART5, f);
			UARTSetDataRate(UART5, PBCLK2, bps);
			UARTSetFifoMode(UART5, UART_INTERRUPT_ON_RX_NOT_EMPTY);
			INTSetVectorPriority(INT_SOURCE_USART_5_RECEIVE, INT_PRIORITY_LEVEL_4);
			INTClearFlag(INT_SOURCE_USART_5_RECEIVE);
			INTEnable(INT_SOURCE_USART_5_RECEIVE, INT_ENABLED);
			UARTEnable(UART5, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
			com_rx_callback[2]=(unsigned long)d2.data.sint64;
		}
		else if(!strcmp("COM4",d1.data.text)) {		// COM4 is using PIC32's UART1
			PORTSetPinsDigitalIn(IOPORT_B, BIT_5);  // RB5 is RX for COM4
			PORTSetPinsDigitalOut(IOPORT_B, BIT_3); // RB3 is TX for COM4
			CNPUBSET=(BIT_3 | BIT_5);   // set pull-ups
			PPSUnLock;
			PPSOutput(2, RPB3, U1TX);
			PPSInput(1, U1RX, RPB5);
			PPSLock;
			com_rx[3]=NULL;
			xalloc((unsigned char **)&com_rx[3], (COM_RX_BUFFER*sizeof(short)));
			if(!com_rx[3]) xv=-7;
			com_rx_in[3]=com_rx_out[3]=0;
			UARTConfigure(UART1, UART_ENABLE_PINS_TX_RX_ONLY);
			UARTSetLineControl(UART1, f);
			UARTSetDataRate(UART1, PBCLK2, bps);
			UARTSetFifoMode(UART1, UART_INTERRUPT_ON_RX_NOT_EMPTY);
			INTSetVectorPriority(INT_SOURCE_USART_1_RECEIVE, INT_PRIORITY_LEVEL_4);
			INTClearFlag(INT_SOURCE_USART_1_RECEIVE);
			INTEnable(INT_SOURCE_USART_1_RECEIVE, INT_ENABLED);
			UARTEnable(UART1, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
			com_rx_callback[3]=(unsigned long)d2.data.sint64;
		}
        else if(!strcmp("COM5",d1.data.text)) {		// COM5 is using PIC32's UART4
			PORTSetPinsDigitalIn(IOPORT_B, BIT_2);  // RB2 is RX for COM5
			PORTSetPinsDigitalOut(IOPORT_B, BIT_0); // RB0 is TX for COM5
			CNPUBSET=(BIT_0 | BIT_2);   // set pull-ups
			PPSUnLock;
			PPSOutput(3, RPB0, U4TX);
			PPSInput(4, U4RX, RPB2);
			PPSLock;
			com_rx[4]=NULL;
			xalloc((unsigned char **)&com_rx[4], (COM_RX_BUFFER*sizeof(short)));
			if(!com_rx[4]) xv=-7;
			com_rx_in[4]=com_rx_out[4]=0;
			UARTConfigure(UART4, UART_ENABLE_PINS_TX_RX_ONLY);
			UARTSetLineControl(UART4, f);
			UARTSetDataRate(UART4, PBCLK2, bps);
			UARTSetFifoMode(UART4, UART_INTERRUPT_ON_RX_NOT_EMPTY);
			INTSetVectorPriority(INT_SOURCE_USART_4_RECEIVE, INT_PRIORITY_LEVEL_4);
			INTClearFlag(INT_SOURCE_USART_4_RECEIVE);
			INTEnable(INT_SOURCE_USART_4_RECEIVE, INT_ENABLED);
			UARTEnable(UART4, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_TX | UART_RX));
			com_rx_callback[4]=(unsigned long)d2.data.sint64;
		}

	}

	else xv=-23;	// invalid parameter
}


void __disable(void) {
	rdata_t d1;
	pullt(&d1);	// interface
	if(xv) return;
	if(!strcmp("SPI1",d1.data.text)) {
		SpiChnEnable(SPI_CHANNEL1, FALSE);
		PPSUnLock;
		PPSOutput(2, RPD3, NULL);
		PPSLock;
		PORTResetPins(IOPORT_D, (BIT_1 | BIT_2 | BIT_3));
		spi_rx_callback[0]=0;	// remove callback
	}
	else if(!strcmp("SPI2",d1.data.text)) {
		SpiChnEnable(SPI_CHANNEL3, FALSE);
		PPSUnLock;
		PPSOutput(1, RPB10, NULL);
		PPSLock;
		PORTResetPins(IOPORT_B, (BIT_9 | BIT_10 | BIT_14));
		spi_rx_callback[1]=0;	// remove callback
	}
	else if(!strcmp("IIC1",d1.data.text)) {
		I2CEnable(I2C1, FALSE);
		PORTResetPins(IOPORT_D, (BIT_9 | BIT_10));
		iic_rx_callback[0]=0;	// remove callback
	}
	else if(!strcmp("IIC2",d1.data.text)) {
		I2CEnable(I2C5, FALSE);
		PORTResetPins(IOPORT_F, (BIT_4 | BIT_5));
		iic_rx_callback[1]=0;	// remove callback
	}
	else if(!strcmp("COM0",d1.data.text)) {
		xflags&=~XFLAG_COM0;
		INTEnable(INT_SOURCE_USART_6_RECEIVE, INT_DISABLED);
		UARTEnable(UART6, 0);
		PPSUnLock;
		PPSOutput(3, RPB7, NULL);
		PPSLock;
		PORTResetPins(IOPORT_B, (BIT_6 | BIT_7));
	}
	else if(!strcmp("COM1",d1.data.text)) {
		INTEnable(INT_SOURCE_USART_2_RECEIVE, INT_DISABLED);
		UARTEnable(UART2, 0);
		PPSUnLock;
		PPSOutput(4, RPD5, NULL);
		PPSLock;
		PORTResetPins(IOPORT_D, (BIT_4 | BIT_5));
		xfree((unsigned char **)&com_rx[0]);
		com_rx_callback[0]=0;	// remove callback
	}
	else if(!strcmp("COM2",d1.data.text)) {
		INTEnable(INT_SOURCE_USART_3_RECEIVE, INT_DISABLED);
		UARTEnable(UART3, 0);
		PPSUnLock;
		PPSOutput(1, RPF1, NULL);
		PPSLock;
		PORTResetPins(IOPORT_F, (BIT_0 | BIT_1));
		xfree((unsigned char **)&com_rx[1]);
		com_rx_callback[1]=0;	// remove callback
	}
	else if(!strcmp("COM3",d1.data.text)) {
		INTEnable(INT_SOURCE_USART_5_RECEIVE, INT_DISABLED);
		UARTEnable(UART5, 0);
		PPSUnLock;
		PPSOutput(2, RPF5, NULL);
		PPSLock;
		PORTResetPins(IOPORT_F, (BIT_4 | BIT_5));
		xfree((unsigned char **)&com_rx[2]);
		com_rx_callback[2]=0;	// remove callback
	}
	else if(!strcmp("COM4",d1.data.text)) {
		INTEnable(INT_SOURCE_USART_1_RECEIVE, INT_DISABLED);
		UARTEnable(UART1, 0);
		PPSUnLock;
		PPSOutput(2, RPB3, NULL);
		PPSLock;
		PORTResetPins(IOPORT_B, (BIT_3 | BIT_5));
		xfree((unsigned char **)&com_rx[3]);
		com_rx_callback[3]=0;	// remove callback
	}
    else if(!strcmp("COM5",d1.data.text)) {
		INTEnable(INT_SOURCE_USART_4_RECEIVE, INT_DISABLED);
		UARTEnable(UART4, 0);
		PPSUnLock;
		PPSOutput(3, RPB0, NULL);
		PPSLock;
		PORTResetPins(IOPORT_B, (BIT_0 | BIT_2));
		xfree((unsigned char **)&com_rx[4]);
		com_rx_callback[4]=0;	// remove callback
	}
	else xv=-23;
}


// internal function
// send character to specified UART
void tx(UART_MODULE uart, int ch) {
	while(!(UARTTransmitterIsReady(uart))) continue;
	UART_DATA ud;
	ud.__data=(UINT16)ch;
    UARTSendData(uart, ud);
}


// internal
// send data to specified UART
void txUART(UART_MODULE uart, rdata_t *d) {
	if(d->type==rSINT64 || d->type==rSINT32 || d->type==rSINT16 || d->type==rSINT8) {
		tx(uart, (int)d->data.sint64);
	}
	else if(d->type==rREAL) {
		char *p=(char *)&d->data.real;
		char cnt=tokens[d->type].inc;
		while(cnt--) tx(uart, (int)*(p++));
	}
	else if(d->type==rTEXT) {
		char *p=d->data.text;
		while(*p) tx(uart, (int)*(p++));
	}
}


// receive character from specified UART
// (blocking) non-zero indicates that the function must be blocking and wait for character
// return the current character from the input buffer, or EOF in case the buffer is empty
int rxUART(UART_MODULE uart, int blocking) {
	unsigned char port;
	if(uart==1) port=3;
	else if(uart==2) port=0;
	else if(uart==3) port=1;
	else if(uart==5) port=2;
    else if(uart==4) port=4;
	else return EOF;
	if(blocking) INTEnableInterrupts();
    while(com_rx_in[port]==com_rx_out[port]) {
        if(!blocking) return EOF;
    }
    int ch=com_rx[port][com_rx_out[port]++];
    if(com_rx_out[port]>=COM_RX_BUFFER) com_rx_out[port]=0;
    return ch;
}


// internal
// transmit/receive through specified SPI
unsigned long txrx(SpiChannel spi, unsigned long txd) {
    SpiChnGetRov(spi, TRUE);
    SpiChnPutC(spi, txd);				// send data
    unsigned long rxd=SpiChnGetC(spi);	// receive data
	return rxd;
}


// transmit block through specified SPI
void txsSPI(SpiChannel spi, void *src, unsigned long count) {
	volatile unsigned long *con = (volatile unsigned long *)(0xBF821000+(0x200*((unsigned long)spi)));	//SPIxCON
	unsigned long bits=*con;
	if(bits & _SPIxCON_MASK_(MODE32_MASK)) {	// 32-bit transfer
		count/=4;
		unsigned long *s=(unsigned long *)src;
		while(count--) txrx(spi, *(s++));
	}
	if(bits & _SPIxCON_MASK_(MODE16_MASK)) {	// 16-bit transfer
		count/=2;
		unsigned short *s=(unsigned short *)src;
		while(count--) txrx(spi, (unsigned long)*(s++));
	}
	else {	// 8-bit transfer
		unsigned char *s=(unsigned char *)src;
		while(count--) txrx(spi, (unsigned long)*(s++));
	}
}


// receive block through specified SPI
void rxsSPI(SpiChannel spi, void *dst, unsigned long count) {
	volatile unsigned long *con = (volatile unsigned long *)(0xBF821000+(0x200*((unsigned long)spi)));	//SPIxCON
	unsigned long bits=*con;
	if(bits & _SPIxCON_MASK_(MODE32_MASK)) {	// 32-bit transfer
		count/=4;
		unsigned long *d=(unsigned long *)dst;
		while(count--) *(d++)=txrx(spi, UINT_MAX);
	}
	if(bits & _SPIxCON_MASK_(MODE16_MASK)) {	// 16-bit transfer
		count/=2;
		unsigned short *d=(unsigned short *)dst;
		while(count--) *(d++)=(unsigned short)txrx(spi, UINT_MAX);
	}
	else {	// 8-bit transfer
		unsigned char *d=(unsigned char *)dst;
		while(count--) *(d++)=(unsigned char)txrx(spi, UINT_MAX);
	}
}


// internal
// send data to specified UART
void txSPI(SpiChannel spi, rdata_t *d) {
	if(d->type==rSINT64 || d->type==rSINT32 || d->type==rSINT16 || d->type==rSINT8) {
		txrx(spi, (unsigned long)d->data.sint64);
	}
	else if(d->type==rREAL) {
		txsSPI(spi, (void *)&(d->data.real), tokens[d->type].inc);
	}
	else if(d->type==rTEXT) {
		txsSPI(spi, (void *)d->data.text, strlen(d->data.text));
	}
}


// internal
// send data to specified I2C
void txI2C(I2C_MODULE i2c, rdata_t *d) {
	if(d->type==rSINT64 || d->type==rSINT32 || d->type==rSINT16 || d->type==rSINT8) {
		while(!I2CTransmitterIsReady(i2c)) continue;
		I2CSendByte(i2c, (char)d->data.sint64);
		while(!I2CTransmissionHasCompleted(i2c)) continue;
	}
	else if(d->type==rREAL) {
		char *p=(char *)&d->data.real;
		char cnt=tokens[d->type].inc;
		I2C_RESULT r=I2C_SUCCESS;
		while(r==I2C_SUCCESS && cnt--) {
			while(!I2CTransmitterIsReady(i2c)) continue;
			r=I2CSendByte(i2c, *(p++));
			while(!I2CTransmissionHasCompleted(i2c)) continue;
			if(!I2CByteWasAcknowledged(i2c)) break;
		}
	}
	else if(d->type==rTEXT) {
		char *p=d->data.text;
		I2C_RESULT r=I2C_SUCCESS;
		while(r==I2C_SUCCESS && *p) {
			while(!I2CTransmitterIsReady(i2c)) continue;
			r=I2CSendByte(i2c, *(p++));
			while(!I2CTransmissionHasCompleted(i2c)) continue;
			if(!I2CByteWasAcknowledged(i2c)) break;
		}
	}
}


// transmit data
void __trmt(void) {
	if(rvm[thd]->dsp>1) {
		rdata_t d1,d2;
		pullfifo(&d2);	// interface
		if(d2.type!=rTEXT) {
			xv=-23;
			return;
		}
		while(rvm[thd]->dsp) {
			pullfifo(&d1);	// data
			if(!strcmp(d2.data.text,"SPI1")) txSPI(SPI_CHANNEL1,&d2);
			else if(!strcmp(d2.data.text,"SPI2")) txSPI(SPI_CHANNEL3,&d2);
			else if(!strcmp(d2.data.text,"COM1")) txUART(UART2,&d2);
			else if(!strcmp(d2.data.text,"COM2")) txUART(UART3,&d2);
			else if(!strcmp(d2.data.text,"COM3")) txUART(UART5,&d2);
			else if(!strcmp(d2.data.text,"COM4")) txUART(UART1,&d2);
            else if(!strcmp(d2.data.text,"COM5")) txUART(UART4,&d2);
			else if(!strncmp(d2.data.text,"IIC1",4)) {
				if(d2.data.text[4]==':' && d2.data.text[5]=='S') {
					while(!I2CBusIsIdle(I2C1)) continue;
					I2CStart(I2C1);
				}
				else if(d2.data.text[4]==':' && d2.data.text[5]=='R') I2CRepeatStart(I2C1);
				else if(d2.data.text[4]==':' && d2.data.text[5]=='P') {
					I2CStop(I2C1);
					I2C_STATUS status;
					do {
						status=I2CGetStatus(I2C1);
					} while (!(status & I2C_STOP));
					break;
				}
				txI2C(I2C1,&d1);
			}
			else if(!strncmp(d2.data.text,"IIC2",4)) {
				if(d2.data.text[4]==':' && d2.data.text[5]=='S') {
					while(!I2CBusIsIdle(I2C5)) continue;
					I2CStart(I2C5);
				}
				else if(d2.data.text[4]==':' && d2.data.text[5]=='R') I2CRepeatStart(I2C5);
				else if(d2.data.text[4]==':' && d2.data.text[5]=='P') {
					I2CStop(I2C5);
					I2C_STATUS status;
					do {
						status=I2CGetStatus(I2C5);
					} while (!(status & I2C_STOP));
					break;
				}
				txI2C(I2C5,&d1);
			}
			else {
				xv=-23;
				break;
			}
		}
	}
	else xv=-24;
}


// receive data
void __recv(void) {
	rdata_t d1,d2;
	pullt(&d2);	// interface
	if(xv) return;
	d1.data.sint64=0;
	if(!strcmp(d2.data.text,"SPI1")) d1.data.sint64=(signed long)txrx(SPI_CHANNEL1,UINT_MAX);
	else if(!strcmp(d2.data.text,"SPI2")) d1.data.sint64=(signed long)txrx(SPI_CHANNEL3,UINT_MAX);
	else if(!strcmp(d2.data.text,"COM1")) d1.data.sint64=(int)rxUART(UART2,0);
	else if(!strcmp(d2.data.text,"COM2")) d1.data.sint64=(int)rxUART(UART3,0);
	else if(!strcmp(d2.data.text,"COM3")) d1.data.sint64=(int)rxUART(UART5,0);
	else if(!strcmp(d2.data.text,"COM4")) d1.data.sint64=(int)rxUART(UART1,0);
    else if(!strcmp(d2.data.text,"COM5")) d1.data.sint64=(int)rxUART(UART4,0);
	else if(!strcmp(d2.data.text,"IIC1")) {
		if(d2.data.text[4]==':' && d2.data.text[5]=='S') {
			while(!I2CBusIsIdle(I2C1)) continue;
			I2CStart(I2C1);
		}
		else if(d2.data.text[4]==':' && d2.data.text[5]=='R') I2CRepeatStart(I2C1);
		else if(d2.data.text[4]==':' && d2.data.text[5]=='P') {
			I2CStop(I2C1);
			I2C_STATUS status;
			do {
				status=I2CGetStatus(I2C1);
			} while (!(status & I2C_STOP));
		}
		I2CReceiverEnable(I2C1, TRUE);
		unsigned short tmout=100;
		while(tmout-- && !I2CReceivedDataIsAvailable(I2C1)) dlyus(100);
        d1.data.sint64=(unsigned char)I2CGetByte(I2C1);
	}
	else if(!strcmp(d2.data.text,"IIC2")) {
		if(d2.data.text[4]==':' && d2.data.text[5]=='S') {
			while(!I2CBusIsIdle(I2C5)) continue;
			I2CStart(I2C5);
		}
		else if(d2.data.text[4]==':' && d2.data.text[5]=='R') I2CRepeatStart(I2C5);
		else if(d2.data.text[4]==':' && d2.data.text[5]=='P') {
			I2CStop(I2C5);
			I2C_STATUS status;
			do {
				status=I2CGetStatus(I2C5);
			} while (!(status & I2C_STOP));
		}
		I2CReceiverEnable(I2C5, TRUE);
		unsigned short tmout=100;
		while(tmout-- && !I2CReceivedDataIsAvailable(I2C5)) dlyus(100);
        d1.data.sint64=(unsigned char)I2CGetByte(I2C5);
	}
	else xv=-23;
	cast(&d1.data.sint64,&d1);
	push(&d1);
}


void __gettime(void) {
	rtccTime tm;	// time structure
	rtccDate dt;	// date structure
	RtccGetTimeDate(&tm, &dt);
	rdata_t d;
	d.type=rTEXT;
	d.data.text=NULL;
	xalloc((unsigned char **)&d.data.text,17);
	if(d.data.text) {
		memset(d.data.text,0,17);
		char t, p=15;
		for(t=0; t<8; t++) {
			d.data.text[p--]=(char)(((tm.l & 15)<=9)? ('0'+(tm.l & 15)) : '#');
			tm.l>>=4;
		}
		for(t=0; t<8; t++) {
			d.data.text[p--]=(char)(((dt.l & 15)<=9)? ('0'+(dt.l & 15)) : ' ');
			dt.l>>=4;
		}
		push(&d);
	}
	else xv=-7;
}


void __settime(void) {
	rdata_t d;
	pullt(&d);
	if(xv) return;
	if(d.data.text==0 || strlen(d.data.text)!=16) {
		xv=-23;
		return;
	}
	char t;
	for(t=0; t<strlen(d.data.text); t++) {
		if(*(d.data.text+t)<'0' ||*(d.data.text+t)>'9') {
			xv=-23;
			return;
		}
	}
	unsigned long tm=0;
	unsigned long dt=0;
	char *p=d.data.text;
	for(t=0; t<8; t++) {
		dt<<=4;
		dt=dt+(*(p++)-'0');
	}
	for(t=0; t<8; t++) {
		tm<<=4;
		tm=tm+(*(p++)-'0');
	}
	RtccSetTimeDate(tm, dt);
}


void __srctime(void) {
	rdata_t d, dcal;
	pulls(&dcal);
	pullt(&d);
	if(xv) return;
	rtccTime tm;	// time structure
	rtccDate dt;	// date structure
	RtccGetTimeDate(&tm, &dt);
	if(!strcmp("LPRC",d.data.text)) RtccOpen(RTCC_LPRC, tm.l, dt.l, (int)dcal.data.sint64);
	else if(!strcmp("SOSC",d.data.text)) RtccOpen(RTCC_SOSC, tm.l, dt.l, (int)dcal.data.sint64);
	else xv=-23;
}


void __clock(void) {
	rdata_t d;
	pullu(&d);	// frequency in MHz
	if(!xv) xv=set_clock((unsigned long)d.data.sint64);
}


void __attribute__((nomips16,nomicromips)) __attribute__((optimize("-O0"))) __sleep(void) {
	rdata_t d;
	pullu(&d);	// sleep time
	if(xv) return;
    if(xflags & XFLAG_ECLK) CloseUSBConsole();
	INTDisableInterrupts();

	// stop all (used) peripherals
	USBCSR0bits.SOFTCONN=0;
	ADCCON1bits.ON=0;
	T8CONbits.ON=0;
	T9CONbits.ON=0;
	U6MODEbits.ON=0;
    U1MODEbits.ON=0;
	U2MODEbits.ON=0;
	U3MODEbits.ON=0;
    U4MODEbits.ON=0;
	U5MODEbits.ON=0;
	SPI1CONbits.ON=0;
	SPI3CONbits.ON=0;
	I2C1CONbits.ON=0;
	I2C5CONbits.ON=0;
	T1CONbits.ON=0;
	T2CONbits.ON=0;
	T3CONbits.ON=0;
	T4CONbits.ON=0;
	T5CONbits.ON=0;
	T6CONbits.ON=0;
	T7CONbits.ON=0;
	OC1CONbits.ON=0;
	OC2CONbits.ON=0;
	OC4CONbits.ON=0;
	OC5CONbits.ON=0;
	OC7CONbits.ON=0;
	OC8CONbits.ON=0;
	OC9CONbits.ON=0;

	// disable everything in the PMDx registers
	PMD1=ULONG_MAX;
	PMD2=ULONG_MAX;
	PMD3=ULONG_MAX;
	PMD4=ULONG_MAX;
	PMD5=ULONG_MAX;
	PMD6=(ULONG_MAX & ~BIT(0));	// keep the RTCC running
	PMD7=ULONG_MAX;

	// full port reset and set all as output in low state
	TRISBSET = TRISCSET = TRISDSET = TRISESET = TRISFSET = TRISGSET = 0;
    ANSELBCLR = ANSELECLR = ANSELGCLR =  0;
    LATBCLR = LATCCLR = LATDCLR = LATECLR = LATFCLR = LATGCLR = 0;
    CNENBCLR = CNENCCLR = CNENDCLR = CNENECLR = CNENFCLR = CNENGCLR = 0;
    CNCONBCLR = CNCONCCLR = CNCONDCLR = CNCONECLR = CNCONFCLR = CNCONGCLR = 0;
    CNPUBCLR = CNPUCCLR = CNPUDCLR = CNPUECLR = CNPUFCLR = CNPUGCLR = 0;
    CNPDBCLR  = CNPDCCLR = CNPDDCLR = CNPDECLR = CNPDFCLR = CNPDGCLR = 0;

	// set up clocks for sleep
	OSCPLLClockUnlock();	// system will be clocked from FRC while in sleep mode
	OSCClockSourceSwitch(OSC_FRC,
				OSC_SYSPLL_FREQ_RANGE_BYPASS,
				OSC_SYSPLL_IN_DIV_1,
				1, 1, TRUE);
	OSCPLLClockLock();
	SystemUnlock();
	// OSCPBOutputClockDisable(OSC_PERIPHERAL_BUS_1);
	OSCPBOutputClockDisable(OSC_PERIPHERAL_BUS_2);
	OSCPBOutputClockDisable(OSC_PERIPHERAL_BUS_3);
	// OSCPBOutputClockDisable(OSC_PERIPHERAL_BUS_4);
	// OSCPBOutputClockDisable(OSC_PERIPHERAL_BUS_5);	// See PIC32MZ-EF ERRATA #22
	// OSCPBOutputClockDisable(OSC_PERIPHERAL_BUS_7);
	SystemLock();
	if(xflags & XFLAG_ECLK) {
		PORTSetPinsDigitalOut(IOPORT_C, BIT_15);
		LATCbits.LATC15=1;			// disable the external oscillator
	}
	SysPerformanceConfig(1, PCACHE_PREFETCH_DISABLE);

	// initialise INT0 and countdown timer
	d.data.sint64/=(1UL<<ReadPostscalerWDT());
	PORTSetPinsDigitalIn(IOPORT_D, BIT_0);	// RD0 will be used as interrupt input
	CNPUDSET=BIT(0);		// enable the internal pull-up resistor on RD0
	xflags&=~XFLAG_INT0;
	mINT0ClearIntFlag();

	// sleep loop
	while((xflags & XFLAG_INT0)==0 && d.data.sint64--) {
		EnableWDT();
		PowerSaveSleep();
		resetNmiEventClear(_RNMICON_WDTS_MASK);
		RCONCLR=(1<<4 | 1<<3);
	}

	// wake up procedure
	DisableWDT();
	if(xflags & XFLAG_ECLK) LATCbits.LATC15=0;	// enable the external oscillator
	xflags&=~XFLAG_INT0;
	init_luke();
	init_warm();
}


void __display(void) {
	rdata_t d,d1,d2;
	pulls(&d2);	// size Y
	pulls(&d1);	// size X
	pullt(&d);	// display type
	if(xv) return;
	if(strcmp("NULL", d.data.text) && strcmp("CONSOLE", d.data.text) &&
		(d1.data.sint64==0 || abs(d1.data.sint64)>=4096 ||
		d2.data.sint64==0 || abs(d2.data.sint64)>=4096)) {
		xv=-3;
		return;
	}

	// handling the "CONSOLE" parameter first before doing anything else
	if(!strcmp("CONSOLE", d.data.text)) {
		if(d1.data.sint64>0 && d2.data.sint64>0) {
			fontFcol=(signed long)d1.data.sint64;
			fontBcol=0;
			fontScale=(unsigned char)(d2.data.sint64 & 255);
			enxcon=1;	// enable external console
		}
		else enxcon=0;	// disable external console
		return;
	}

	display_Detach();	// execute the function for detaching the current display
	null_Attach();

	// set display resolution
	char revH=0, revV=0;
	resH=(signed int)d1.data.sint64;	// set horizontal resolution
	if(resH==0) resH=1;
	if(resH<0) revH=1;
	resH=abs(resH);
	resV=(signed int)d2.data.sint64;	// set vertical resolution
	if(resV==0) resV=1;
	if(resV<0) revV=1;
	resV=abs(resV);

	// determine display orientation
	if(resH>resV) {
		if(revH || revV) orientation=RLANDSCAPE;
		else orientation=LANDSCAPE;
	}
	else if(resH<resV) {
		if(revH || revV) orientation=RPORTRAIT;
		else orientation=PORTRAIT;
	}
	else {
		if(revH && revV) orientation=RPORTRAIT;
		else if(revH==0 && revV) orientation=RLANDSCAPE;
		else if(revH && revV==0) orientation=PORTRAIT;
		else orientation=LANDSCAPE;
	}

	if(!strcmp("NULL", d.data.text)) {}	// nothing needed here

	else if(!strcmp("HD44780", d.data.text) || !strcmp("LCD4", d.data.text)) {
		if(resH<8 || resH>40 || resV<1 || resV>4 || orientation!=LANDSCAPE) {
			xv=-3;
			return;
		}
		lcd4_Attach();
	}

	else if(!strcmp("LRSPI*", d.data.text) || !strcmp("MLRSPI*", d.data.text)) {
		if(resH<32 || resH>480 || resV<32 || resV>480) {
			xv=-3;
			return;
		}
		if(!strcmp("MLRSPI*", d.data.text)) orientation=-orientation;
		lcdSPI_Attach(max(resH,resV));
	}

	else if(!strcmp("SSD196*", d.data.text) || !strcmp("MSSD196*", d.data.text)) {
		if(resH<32 || resH>1280 || resV<32 || resV>1280) {
			xv=-3;
			return;
		}
		if(!strcmp("MSSD196*", d.data.text)) orientation=-orientation;
		lcdSSD196_Attach();
	}

	else xv=-23;
	if(xv) return;
	display_DrawRect(0,0,(resH-1),(resV-1),fontBcol);
}


// internal: single calibration point for the touch panel
char touch_pcal(int px, int py, rdata_t *x, rdata_t *y) {
	char res=1;
	int ch;
	display_DrawRect(px-10,py-10,px+10,py+10,0xffffff);
	dlyus(250000);
	xflags&=~XFLAG_TOUCH_IRQ;
	touch_points=0;
	while(touch_points!=1) {
		ch=kbhit();
		if(ch>EOF) {
			while(kbhit()) getch();
			if(ch==KEY_ESC) {
				res=0;
				push(x);
				push(y);
				break;
			}
		}
		if(xflags & XFLAG_TOUCH_IRQ) {
			touch_ReadTouch();
			if(touch_points==0) {
				res=0;
				push(x);
				push(y);
				break;
			}
		}
	}
	display_DrawRect(px-10,py-10,px+10,py+10,0);
	return res;
}


void __touch(void) {
	if(rvm[thd]->dsp<1) {
		xv=-24;
		return;
	}
	rdata_t d;
	pullfifo(&d);	// type/command
	if(!xv && d.type!=rTEXT) xv=-23;
	if(xv) return;
	char comf=0;

	// handle the touch commands BEFORE the code for hardware initialisation
	if(!strcmp("POINTS", d.data.text)) {
		comf=1;
		rdata_t i;
		i.type=rSINT16;
		i.data.sint64=(signed short)touch_points;
		push(&i);
	}

	else if(!strcmp("READ", d.data.text)) {
		comf=1;
		rdata_t x,y,p;
		x.type=rSINT16; x.data.sint64=(signed short)-1;
		y.type=rSINT16; y.data.sint64=(signed short)-1;
		p.type=rSINT16; p.data.sint64=(signed short)-1;
		if(touch_points) {	// reads only one touch point here!
			unsigned char i=touch_points-1;
			x.data.sint64=(signed short)touch_x[i];
			y.data.sint64=(signed short)touch_y[i];
			p.data.sint64=(signed short)touch_p[i];
			touch_points--;
		}
		push(&x);
		push(&y);
		push(&p);
	}

	else if(!strcmp("SETCAL", d.data.text)) {
		comf=1;
		rdata_t x,y;
		pullu(&y);
		pullu(&x);
		if(!xv) {
			touch_calx=(unsigned long)x.data.sint64;
			touch_caly=(unsigned long)y.data.sint64;
		}
	}

	else if(!strcmp("CALIBRATE", d.data.text)) {
		signed int x1,x2,x3,x4,y1,y2,y3,y4;
		comf=1;
		rdata_t x,y;
		x.type=rSINT64; x.data.sint64=0;
		y.type=rSINT64; y.data.sint64=0;
		display_Cls();

		// top left
		if(!touch_pcal(15,15,&x,&y)) return;
		x1=touch_x[0];
		y1=touch_y[0];

		// top right
		if(!touch_pcal(resH-16,15,&x,&y)) return;
		x2=touch_x[0];
		y2=touch_y[0];

		// bottom left
		if(!touch_pcal(15,resV-16,&x,&y)) return;
		x3=touch_x[0];
		y3=touch_y[0];

		// bottom right
		if(!touch_pcal(resH-16,resV-16,&x,&y)) return;
		x4=touch_x[0];
		y4=touch_y[0];

		y1=(y1+y2)/2;	// averaged top
		x1=(x1+x3)/2;	// averaged left
		y2=(y3+y4)/2;	// averaged bottom
		x2=(x2+x4)/2;	// averaged right

		// return the calibration values
		x.data.sint64 = (x1 << 16) + (x2 & 0xffff);
		y.data.sint64 = (y1 << 16) + (y2 & 0xffff);

		push(&x);
		push(&y);
	}

	// exit here if a valid touch command was executed
	if(comf) return;

	// detach any currently operating touch panel
	touch_Detach();
	xflags&=~(XFLAG_TOUCH | XFLAG_TOUCH_IRQ | XFLAG_TOUCH_ROT);
	touch_InitPort=null_InitPort;
	touch_Detach=null_Detach;
	touch_ReadTouch=null_ReadTouch;
	touch_callback=0;	// remove the callback function
	touch_calx=0;
	touch_caly=0;

	if(!strcmp("NULL", d.data.text)) {}	// nothing is needed here

	else if(!strcmp("RSPI*46", d.data.text) || !strcmp("RRSPI*46", d.data.text)) {	// TSC2046 / XPT2046
		if(!strcmp("RRSPI*46", d.data.text)) xflags|=XFLAG_TOUCH_ROT;
		pullfifo(&d);	// callback
		if(d.type!=rFUNC) {
			xv=-5;
			return;
		}
		trspi_Attach();
		touch_callback=(unsigned long)d.data.sint64;
	}

	else xv=-23;
}
