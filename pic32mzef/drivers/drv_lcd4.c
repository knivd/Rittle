#include <stdio.h>
#include "drv_lcd4.h"

#define row0addr	0x00		// start address in RAM for every row of the screen
#define row1addr	0x40
#define row2addr	0x14
#define row3addr	0x54

#define LCD_CGADR	0b01000000	// set CGRAM address
#define LCD_DDADR	0b10000000	// set DDRAM address

unsigned char lcd4_lockf = 0;	// scroll lock flag
unsigned char lcd4_buf[4][40];	// screen buffer


void LCD4eStrobe(void) {
    LCD4_EN=1;
    dlyus(40);
    LCD4_EN=0;
	dlyus(40);
}


void LCD4wrNibble(unsigned char data) {
    LCD4_D7=((data&8)? 1 : 0);
    LCD4_D6=((data&4)? 1 : 0);
    LCD4_D5=((data&2)? 1 : 0);
    LCD4_D4=((data&1)? 1 : 0);
    LCD4eStrobe();
}


void LCD4wrByte(unsigned char data) {
    LCD4wrNibble(data>>4);
    LCD4wrNibble(data);
    LCD4_RS=0;
}


void lcd4_GotoXY(int px, int py) {
	unsigned char addr=px;
	posX=px;
	posY=py;
	if(posX>=resH || posY>=resV) return;
	switch(posY) {
		default: return;	// undefined address
		case 0:
			addr+=row0addr;
			break;
		case 1:
			addr+=row1addr;
			break;
		case 2:
			addr+=row2addr;
			break;
		case 3:
			addr+=row3addr;
			break;
	}
	LCD4wrByte((unsigned char)(LCD_DDADR | addr));
}


void lcd4_InitPort(void) {
	PORTSetPinsDigitalOut(IOPORT_E, (BIT_0 | BIT_1 | BIT_2 | BIT_3));
	PORTSetPinsDigitalOut(IOPORT_F, (BIT_0 | BIT_1));
	LCD4_D7 = LCD4_D6 = LCD4_D5 = LCD4_D4 = 0;
	LCD4_EN=0;
	LCD4_RS=0;
}


void LCD4init(void) {
	lcd4_InitPort();
    dlyus(200000);
	LCD4wrNibble(0x03);
	dlyus(10000);
	LCD4eStrobe();
	dlyus(10000);
	LCD4eStrobe();
	dlyus(10000);
	LCD4wrNibble(0x02);
	dlyus(10000);
	LCD4wrByte(0x28);
	LCD4wrByte(0x14);
	LCD4wrByte(0x06);
	LCD4wrByte(0x0c);
	LCD4wrByte(0x01);
	dlyus(50000);
	lcd4_GotoXY(0,0);
	memset(lcd4_buf,' ',sizeof(lcd4_buf));
}


void lcd4_Cls(void) {
	LCD4wrByte(0x01);
	dlyus(50000);
	memset(lcd4_buf,' ',sizeof(lcd4_buf));
	lcd4_GotoXY(0,0);
}


void LCD4updateScr(void) {
	unsigned char t,r;
	for(r=0; r<resV; r++) {
		for(t=0; t<resH; t++) {
			lcd4_GotoXY(t,r);
			LCD4_RS=1;
			LCD4wrByte((unsigned char)(lcd4_buf[r][t] & 0x7f));
		}
	}
}


void LCD4scrollUp(void) {
	unsigned char t,r;
	unsigned char x=posX, y=posY;
	for(r=0; r<(resV-1); r++) memcpy(lcd4_buf[r],lcd4_buf[r+1],40);
	memset(lcd4_buf[resV-1],' ',40);
	LCD4updateScr();
	lcd4_GotoXY(x,y);
}


void LCD4scrollDown(void) {
	unsigned char t,r;
	unsigned char x=posX, y=posY;
	for(r=(resV-1); r; r--) memcpy(lcd4_buf[r-1],lcd4_buf[r],40);
	memset(lcd4_buf[0],' ',40);
	LCD4updateScr();
	lcd4_GotoXY(x,y);
}


void LCD4scrollLeft(void) {
	unsigned char t,r;
	unsigned char x=posX, y=posY;
	for(r=0; r<resV; r++) {
		for(t=0; t<(resH-1); t++) lcd4_buf[r][t]=lcd4_buf[r][t+1];
	}
	for(r=0; r<resV; r++) lcd4_buf[r][resH-1]=' ';
	LCD4updateScr();
	lcd4_GotoXY(x,y);
}


void LCD4scrollRight(void) {
	unsigned char t,r;
	unsigned char x=posX, y=posY;
	for(r=0; r<resV; r++) {
		for(t=(resH-1); t; t--) lcd4_buf[r][t]=lcd4_buf[r][t-1];
	}
	for(r=0; r<resV; r++) lcd4_buf[r][0]=' ';
	LCD4updateScr();
	lcd4_GotoXY(x,y);
}


void lcd4_Scroll(char direction, int lines) {
	if(direction==DIR_0000) {
		while(lines--) {
			LCD4scrollUp();
		}
	}
	else if(direction==DIR_0130) {
		while(lines--) {
			LCD4scrollUp();
			LCD4scrollRight();
		}
	}
	else if(direction==DIR_0300) {
		while(lines--) {
			LCD4scrollRight();
		}
	}
	else if(direction==DIR_0430) {
		while(lines--) {
			LCD4scrollRight();
			LCD4scrollDown();
		}
	}
	else if(direction==DIR_0600) {
		while(lines--) {
			LCD4scrollDown();
		}
	}
	else if(direction==DIR_0730) {
		while(lines--) {
			LCD4scrollDown();
			LCD4scrollLeft();
		}
	}
	else if(direction==DIR_0900) {
		while(lines--) {
			LCD4scrollLeft();
		}
	}
	else if(direction==DIR_1030) {
		while(lines--) {
			LCD4scrollLeft();
			LCD4scrollUp();
		}
	}
}


void lcd4_PutChr(int ch) {
	if(ch==0) {	// character 0 will only display virtual cursor
		lcd4_GotoXY(posX,posY);
		LCD4_RS=1;
		LCD4wrByte((unsigned char)'|');
		return;
	}
	if(ch<=0 || ch>0xff) return;
	if(posX>=resH || posY>=resV) return;
	unsigned char t,r;
	switch(ch) {
		case KEY_BACKSPC:
			if(posX) lcd4_GotoXY(posX-1,posY);	// BACKSPACE
			// lcd4_PutChr(' ');
			lcd4_buf[posY][posX]=' ';
			lcd4_GotoXY(posX-1,posY);
			break;
		case KEY_TAB:
			for(t=0; t<3; t++) lcd4_PutChr(' '); // HORIZONTAL TABULATION - 3 spaces
			break;
		case KEY_ENTER:
			lcd4_GotoXY(0,posY);		// CARRIAGE RETURN
			break;
		case KEY_NLINE:					// NEW LINE (scroll up after the most bottom line)
			if(++posY>=resV) {
				posY=resV-1;
				if(!lcd4_lockf) LCD4scrollUp();
			}
			lcd4_GotoXY(posX,posY);
			break;
		case KEY_FFEED:
			lcd4_Cls();					// FORM FEED - clear screen and set cursor at position 0,0
			break;
		case KEY_BEEP:
			// TODO
			break;
		case KEY_CLREOL:
			r=posX;						// CLEAR TO THE END OF LINE
			for(t=0; t<resH; t++) {
				lcd4_GotoXY(t,resV-1);
				LCD4_RS=1;
				LCD4wrByte(' ');
				lcd4_buf[posY][posX]=' ';
			}
			lcd4_GotoXY(r,posY);
			break;
		default:
			lcd4_GotoXY(posX,posY);
			LCD4_RS=1;
			LCD4wrByte((unsigned char)(ch & 0x7f));	// send the character to the LCD
			lcd4_buf[posY][posX]=(char)ch;
			if((++posX)<resH) lcd4_GotoXY(posX,posY);
			else {
				if(++posY>=resV) {
					if(!lcd4_lockf) {
						posY=resV-1;
						LCD4scrollUp();
					}
				}
				lcd4_GotoXY(0,posY);
			}
			break;
	}
}


void LCD4string(unsigned char *data) {
	while(*data) lcd4_PutChr(*(data++));
}


// define new character with "code" and bitmask built from 5 bytes (one character is 5x8 pixels)
// character definition starts from the left column and bit 0 in every byte is the top line
void LCD4charDef(unsigned char code, unsigned char *bitmap) {
	unsigned char t,r,a;
	LCD4wrByte((unsigned char)(LCD_CGADR | ((code&7)<<3)));	// only codes 0...7 are available for redefinition
	for(r=0; r<8; r++) {
		a=0;
		for(t=0; t<6; t++) {
			a<<=1;
			if(bitmap[t]&BIT(r)) a|=1;
		}
		LCD4_RS=1;
		LCD4wrByte(a);
	}
	lcd4_GotoXY(posX,posY);	// restore cursor position
}


void __LCD4(void) {
	rdata_t d,d1,d2;
	pullfifo(&d);
	if(d.type!=rTEXT || d.data.text==0) {
		xv=-23;
		return;
	}

	else if(!strcmp("init:L",d.data.text) || !strcmp("init:S",d.data.text)) {
		pullfifo(&d1);
		pullfifo(&d2);
		if(d1.type!=rSINT64 || d1.data.sint64<8 || d1.data.sint64>40 ||
			d2.type!=rSINT64 || d2.data.sint64<1 || d2.data.sint64>4) {
			xv=-3;
			return;
		}

		// initialise the LCD module
		resH=(unsigned char)d1.data.sint64;
		if(resH==0) resH=1;
		resV=(unsigned char)d2.data.sint64;
		if(resV==0) resV=1;
		lcd4_lockf=((d.data.text[strlen(d.data.text)-1]=='L')? 1 : 0);
		LCD4init();
	}

	else if(!strcmp("clear",d.data.text)) lcd4_Cls();
	else if(!strcmp("scroll:U",d.data.text)) lcd4_Scroll(DIR_0000,1);
	else if(!strcmp("scroll:R",d.data.text)) lcd4_Scroll(DIR_0300,1);
	else if(!strcmp("scroll:D",d.data.text)) lcd4_Scroll(DIR_0600,1);
	else if(!strcmp("scroll:L",d.data.text)) lcd4_Scroll(DIR_0900,1);

	else if(!strcmp("goto",d.data.text)) {
		pullfifo(&d1);
		pullfifo(&d2);
		if(d1.type!=rSINT64 || d1.data.sint64<0 || d1.data.sint64>=resH ||
			d2.type!=rSINT64 || d2.data.sint64<0 || d2.data.sint64>=resV) {
			xv=-3;
			return;
		}
		lcd4_GotoXY((unsigned char)d1.data.sint64,(unsigned char)d2.data.sint64);
	}

	else if(!strcmp("print",d.data.text)) {
		while(rvm[thd]->dsp) {
			pullfifo(&d1);
			if(d1.type==rTEXT) {
				if(d1.data.text) LCD4string(d1.data.text);
			}
			else if(d1.type==rSINT64 || d1.type==rSINT32 || d1.type==rSINT16 || d1.type==rSINT8) {
				char buf[25];
				sprintf(buf,"%lli",d1.data.sint64);
				LCD4string(buf);
			}
			else if(d1.type==rREAL) {
				char buf[25];
				sprintf(buf,"%G",d1.data.real);
				LCD4string(buf);
			}
			// else xv=-23;
		}
	}

	else if(!strcmp("defchar",d.data.text)) {
		pullfifo(&d1);
		pullfifo(&d2);
		if(d1.type!=rSINT64 || d1.data.sint64<0 || d1.data.sint64>7 || d2.type!=rSINT64) {
			xv=-3;
			return;
		}
		LCD4charDef((unsigned char)d1.data.sint64, (unsigned char *)&d2.data.sint64);
	}

	else xv=-23;
}


// detach and release the used pins
void lcd4_Detach(void) {
	PORTResetPins(IOPORT_E, (BIT_0 | BIT_1 | BIT_2 | BIT_3));
	PORTResetPins(IOPORT_F, (BIT_0 | BIT_1));
}


// attach this driver to the RVM
void lcd4_Attach(void) {
	lcd4_lockf=0;	// no lock
	LCD4init();
	display_InitPort=lcd4_InitPort;
	display_Detach=lcd4_Detach;
	display_Cls=lcd4_Cls;
	display_ScrollUp=LCD4scrollUp;
	display_PutChr=lcd4_PutChr;
}
