#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "rvm.h"

#define REFMASK 0xffffffffffff0000ULL
#define REFVAL  0x00000000feed0000ULL

// global variables
unsigned char ptrlen=sizeof(unsigned long);	// memory addresses are 32-bit
unsigned long execlen=0;		// total length of the currently executed code
char encout=1;					// enable console output
char xmode=0;					// execution mode as specified in execute() parameters
signed long (*dbgfunc)(void);	// callback debug function
signed long long uptcounter=0;	// incremental microsecond counter for the uptime() function;
unsigned long cclkss=0;			// cclk snapshot used for the up timer
unsigned long cclkss_old=0;		// previous cclk value
int conch_buf=EOF;				// buffer for conch()
int char_buf=0;					// buffer for char()

// format buffer variables
char prefill, postfill;			// pre-fill and post-fill characters in __format()
unsigned char fmtflags;			// format flags (used for processing the 'format' word)
						    	// [0.1]: 00:none, 01:'>', 10:'<', 11:'^'
						    	// [2.3]: 00:none, 01:'-', 10:'+', 11:reserved
						    	// [4]: pre-fill '#'
								// [5]: post-fill '#'
signed char fmtprec;			// format precision (digits after the decimal point)
								// value -1 = no decimal point
signed long long fmtlen;		// format field length
char fmtbuf[81];				// format buffer

// guaranteed string constants
char *BLANK = "";
char *CRLF = "\r\n";

// file structures
FIL *files[MAX_FILES];	// unused files are NULL

// graphics and touch driver variables
signed int resH=1, resV=1;			// screen resolution
signed char orientation=LANDSCAPE;	// screen orientation
signed int posX=0, posY=0;			// virtual cursor position
const font_t *font=&sysFont0508;	// default font 5x8 pixels
signed long fontFcol=LONG_MAX;		// current front colour for fonts
signed long fontBcol=0;				// current back colour for fonts
unsigned char fontScale=1;			// scaling factor for the font printing
unsigned short lastW=0;				// width of the last graphically printed character including the right blanking pixels
unsigned char no_crtl_chars=0;		// disable the control characters in gPutChr()

// touch interface variables
volatile unsigned char touch_points=0;
volatile signed int touch_x[MAX_TOUCH_POINTS];
volatile signed int touch_y[MAX_TOUCH_POINTS];
volatile signed int touch_p[MAX_TOUCH_POINTS];

// top level interface primitive drivers
void (*display_InitPort)(void) = null_InitPort;
void (*display_Detach)(void) = null_Detach;
void (*display_Cls)(void) = null_Cls;
void (*display_PutChr)(int ch) = null_PutChr;
void (*display_ScrollUp)(void) = null_ScrollUp;
void (*display_DrawRect)(int x1, int y1, int x2, int y2, long col) = null_DrawRect;
void (*display_ReadRect)(int x1, int y1, int x2, int y2, long *colarray) = null_ReadRect;

#define display_Pixel(x,y,c) display_DrawRect((int)x, (int)y, (int)x, (int)y, (long)c)


// GENERIC FUNCTIONS AND STACK OPERATION FUNCTIONS ==========================================

void freevars(vmvar_t *pv) {
	while(pv) {	// free all pre-allocated variables
		vmvar_t *npv=(vmvar_t *)pv->next;
		if(pv->type==rTEXT) {
			unsigned long t,z=1;
			for(t=0; t<MAX_DIMENSIONS; t++) if(pv->dim[t]>0) z*=pv->dim[t];
			unsigned char *w=(unsigned char *)((unsigned char *)pv+sizeof(vmvar_t));
			for(t=0; t<z; t++) {
                if(w>=memory && w<(memory+XMEM_SIZE)) xfree((unsigned char **)w);
				w+=sizeof(char *);
			}
		}
		xfree((unsigned char **)&pv);
		pv=npv;
	}
}


// convert data type in data element
// NOTE: for integer types converts to the most basic type only
void convert(rdata_t *d, rtype_t newtype) {
	if((newtype!=d->type) && (newtype!=rANY)) {
		if(d->type<=rSINT64 && d->type>=rSINT8) {
			if(newtype==rSINT8) d->data.sint64=((signed char)d->data.sint64 & UCHAR_MAX);
			else if(newtype==rSINT16) d->data.sint64=((signed short)d->data.sint64 & USHRT_MAX);
			else if(newtype==rSINT32) d->data.sint64=((signed long)d->data.sint64 & ULONG_MAX);
			else if(newtype==rSINT64) d->data.sint64=((signed long long)d->data.sint64 & ULONG_LONG_MAX);
			else if(newtype==rREAL) d->data.real=(double)d->data.sint64;
			else if(newtype==rFUNC) d->data.sint64=((signed long)d->data.sint64 & ULONG_MAX);
			else xv=-4;
		}
		else if(d->type==rREAL) {
			if(d->data.real<(double)LONG_LONG_MIN || d->data.real>(double)LONG_LONG_MAX) xv=-3;
			else if(newtype==rSINT64) d->data.sint64=(signed long long)xround(d->data.real);
			else if(newtype==rSINT32) d->data.sint64=(signed long)xround(d->data.real);
			else if(newtype==rSINT16) d->data.sint64=(signed short)xround(d->data.real);
			else if(newtype==rSINT8) d->data.sint64=(signed char)xround(d->data.real);
			else xv=-4;
		}
		else if(d->type==rFUNC) {
			if(newtype==rSINT32 || newtype!=rSINT64) d->data.sint64=((signed long)d->data.sint64 & ULONG_MAX);
			else xv=-4;
		}
		else xv=-4;
		if(!xv) d->type=newtype;
	}
}


// equalise two data types
// IMPORTANT: uses the basic container types only
void common(rdata_t *d1, rdata_t *d2) {
	rtype_t r=rINVALID;
	if((d1->type)!=(d2->type)) {
		if((d1->type)==rTEXT || (d2->type)==rTEXT) xv=-4;
		else if((d1->type)>(d2->type)) r=(d1->type);
		else r=(d2->type);
		if(r==rSINT8 || r==rSINT16 || r==rSINT32) r=rSINT64;
		if(r!=rINVALID) {
			convert(d1,r);
			convert(d2,r);
		}
	}
}


// push data to the stack
void push(rdata_t *d) {
	memcpy((unsigned char *)&(rvm[thd]->stack[rvm[thd]->dsp++]),(unsigned char *)d,sizeof(rdata_t));
	if(rvm[thd]->dsp>=MAX_STACK) {
		rvm[thd]->dsp=MAX_STACK-1;
		memmove((unsigned char *)rvm[thd]->stack,(unsigned char *)&(rvm[thd]->stack[1]),(rvm[thd]->dsp*sizeof(rdata_t)));	// free up one element in the stack
	}
}


// pull data from the stack (repeat last if the stack is empty)
// IMPORTANT: returns the basic container types only
void pull(rdata_t *d) {
	if(rvm[thd]->dsp) rvm[thd]->dsp--;
	memcpy((unsigned char *)d,(unsigned char *)&(rvm[thd]->stack[rvm[thd]->dsp]),sizeof(rdata_t));
	if(d->type==rSINT8 || d->type==rSINT16 || d->type==rSINT32) convert(d,rSINT64);
	else if(d->type==rTEXT) {
		if(d->data.text) d->data.tlen=strlen(d->data.text);
		else {
			d->data.text=(char *)&BLANK;	// making sure invalid texts are properly blanked
			d->data.tlen=0;
		}
	}
}


// pull data from the stack in FIFO order
// IMPORTANT: returns the basic container types only
void pullfifo(rdata_t *d) {
	memcpy((unsigned char *)d,(unsigned char *)&(rvm[thd]->stack[0]),sizeof(rdata_t));
	if(rvm[thd]->dsp) memmove((unsigned char *)&(rvm[thd]->stack[0]),(unsigned char *)&(rvm[thd]->stack[1]),(--rvm[thd]->dsp*sizeof(rdata_t)));
	if(d->type==rSINT8 || d->type==rSINT16 || d->type==rSINT32) convert(d,rSINT64);
	else if(d->type==rTEXT) {
		if(d->data.text) d->data.tlen=strlen(d->data.text);
		else {
			d->data.text=(char *)&BLANK;	// making sure invalid texts are properly blanked
			d->data.tlen=0;
		}
	}
}


// pull reference to variable from the stack
void pullref(rdata_t *d) {
	if(rvm[thd]->dsp) rvm[thd]->dsp--;
	memcpy((unsigned char *)d,(unsigned char *)&(rvm[thd]->stack[rvm[thd]->dsp]),sizeof(rdata_t));
	if(d->type!=rSINT32 || (d->data.sint64 & REFMASK)!=REFVAL) xv=-17;
}


// non-invasive access to data element in the stack
void get(rdata_t *d, unsigned short depth) {
	unsigned short dsp0=rvm[thd]->dsp;
	if(depth<=rvm[thd]->dsp) rvm[thd]->dsp-=depth; else rvm[thd]->dsp=0;
	pull(d);
	rvm[thd]->dsp=dsp0;	// restore the original stack pointer
}


// pull signed integer from the stack
void pulls(rdata_t *d) {
	pull(d);
	convert(d,rSINT64);
}


// pull unsigned integer from the stack
void pullu(rdata_t *d) {
	pulls(d);
	if(!xv && d->data.sint64<0) xv=-3;
}


// pull real number from the stack
void pullr(rdata_t *d) {
	pull(d);
	convert(d,rREAL);
}


// pull text from the stack
void pullt(rdata_t *d) {
	pull(d);
	if(!xv && d->type!=rTEXT) xv=-4;
}


// TOKEN LIBRARY ==============================================================

void _invalid_(void) {
	xv=-1;
}


void _unsupported_(void) {
	xv=-21;
}


void __nop(void) {
}


void __break(void) {
	if(xmode) {
		if(dbgfunc) dbgfunc();
		if(!xv) xv=2;
	}
}


void __clear(void) {
	while(rvm[thd]->dsp) {
		rvm[thd]->dsp--;
		if(rvm[thd]->stack[rvm[thd]->dsp].type==rTEXT &&
			(unsigned char *)rvm[thd]->stack[rvm[thd]->dsp].data.text>=memory &&
			(unsigned char *)rvm[thd]->stack[rvm[thd]->dsp].data.text<(memory+XMEM_SIZE))
			xfree((unsigned char **)rvm[thd]->stack[rvm[thd]->dsp].data.text);
	}
}


void __dup(void) {
	rdata_t d;
	get(&d,0);
	push(&d);
}


void __drop(void) {
	rdata_t d;
	pull(&d);
}


void __reset(void) {
    for(thd=0; thd<MAX_FILES; thd++) {
        if(files[thd]) xfree((unsigned char **)files[thd]);
    }
    for(thd=0; thd<=MAX_THREADS; thd++) {
        if(rvm[thd]) xfree((unsigned char **)rvm[thd]);
    }
    threads=0;
    thd=0;
    xalloc((unsigned char **)&rvm[thd],sizeof(rvm_t));
    if(!rvm[thd]) {
        xv=-7;
        return;
    }
    rvm[thd]->vmv=NULL;
    rvm[thd]->dsp=0;
	rvm[thd]->plp=0;
    rvm[thd]->csp=0;
    rvm[thd]->entry=0;
    rvm[thd]->pc=0;
    rvm[thd]->lvidx=0;
    rvm[thd]->ix=0;
	for(xv=0; xv<MAX_STACK; xv++) rvm[thd]->stack[xv].type=rINVALID;
	xv=0;
	if(*(mem+rvm[thd]->pc)!=token(".reset") || *(mem+rvm[thd]->pc+1)!=0x4b || *(mem+rvm[thd]->pc+2)!=0xb8 || *(mem+rvm[thd]->pc+3)!=0x44) xv=-2;
	rvm[thd]->pc+=4;
}


void __comment(void) {
    while(*(mem+rvm[thd]->pc)) rvm[thd]->pc++;
    rvm[thd]->pc++;  // skip the trailing NUL character
}


void __sint8(void) {
	unsigned char r=sizeof(signed char);
	rdata_t d;
	d.type=rSINT8;
	d.data.sint64=0;
	memcpy((unsigned char *)&d.data.sint64,(unsigned char *)(mem+rvm[thd]->pc),r);
	d.data.sint64=(signed char)d.data.sint64;
	push(&d);
	rvm[thd]->pc+=r;
}


void __sint16(void) {
	unsigned char r=sizeof(signed short);
	rdata_t d;
	d.type=rSINT16;
	d.data.sint64=0;
	memcpy((unsigned char *)&d.data.sint64,(unsigned char *)(mem+rvm[thd]->pc),r);
	d.data.sint64=(signed short)d.data.sint64;
	push(&d);
	rvm[thd]->pc+=r;
}


void __sint32(void) {
	unsigned char r=sizeof(signed long);
	rdata_t d;
	d.type=rSINT32;
	d.data.sint64=0;
	memcpy((unsigned char *)&d.data.sint64,(unsigned char *)(mem+rvm[thd]->pc),r);
	if((d.data.sint64 & REFMASK)!=REFVAL) d.data.sint64=(signed long)d.data.sint64;	// references are not cast
	push(&d);
	rvm[thd]->pc+=r;
}


void __sint64(void) {
	unsigned char r=sizeof(signed long long);
	rdata_t d;
	d.type=rSINT64;
	d.data.sint64=0;
	memcpy((unsigned char *)&d.data.sint64,(unsigned char *)(mem+rvm[thd]->pc),r);
	push(&d);
	rvm[thd]->pc+=r;
}


void __creal(void) {
	double v=0.0;
	rdata_t d;
	d.type=rREAL;
	memcpy((unsigned char *)&v,(unsigned char *)(mem+rvm[thd]->pc),sizeof(double)); 
	d.data.real=v;
	push(&d);
	rvm[thd]->pc+=sizeof(double);
}


void __text(void) {
	rdata_t d;
	d.data.text=(char *)(mem+rvm[thd]->pc);
	while(*(mem+rvm[thd]->pc)) rvm[thd]->pc++;
	while(*(mem+rvm[thd]->pc)==0) rvm[thd]->pc++;
	d.type=rTEXT;
	push(&d);
}


void __reffn(void) {
	unsigned char r=sizeof(signed long);
	rdata_t d;
	d.type=rFUNC;
	d.data.sint64=0;
	memcpy((unsigned char *)&d.data.sint64,(unsigned char *)(mem+rvm[thd]->pc),r);
	d.data.sint64=(unsigned long)d.data.sint64;
	push(&d);
	rvm[thd]->pc+=r;
}


void __exec(void) {
	unsigned char r=sizeof(unsigned long);
	rdata_t d;
	d.data.sint64=0;
	memcpy((unsigned char *)&d.data.sint64,(unsigned char *)(mem+rvm[thd]->pc),r);
	d.type=rFUNC;
	push(&d);
	rvm[thd]->pc+=r;
}


void __exit(void) {
	if(rvm[thd]->lvidx) {	// free all local variables
		rvm[thd]->lvidx--;
		if(rvm[thd]->local[rvm[thd]->lvidx]) {
			if(rvm[thd]->local[rvm[thd]->lvidx]->next) freevars((vmvar_t *)rvm[thd]->local[rvm[thd]->lvidx]->next);
			rvm[thd]->local[rvm[thd]->lvidx]->next=NULL;
		} else {
			freevars(rvm[thd]->vmv);	// there haven't been any variables prior to the current local bunch
			rvm[thd]->vmv=NULL;
		}
	}
	if(rvm[thd]->csp) {
		rvm[thd]->pc=rvm[thd]->callst[--(rvm[thd]->csp)];
		if(rvm[thd]->csp==0 && thd) {
			freevars(rvm[thd]->vmv);
			rvm[thd]->vmv=NULL;
			if(thd<threads) memcpy((unsigned char *)rvm[thd],(unsigned char *)rvm[threads],sizeof(rvm_t));
			if(threads) {
                xfree((unsigned char **)rvm[threads]);
                threads--;
            }
			thd=0;
		}
	} else {
		freevars(rvm[thd]->vmv);
        xdefrag();
		rvm[thd]->vmv=NULL;
		xv=1;	// end of main program body
	}
}


void __ppterm(void) {
	if(!thd) {	// only thread 0 can terminate processes
		unsigned long e=(unsigned long)*(mem+rvm[thd]->pc)+((unsigned long)*(mem+rvm[thd]->pc+1)<<8)+((unsigned long)*(mem+rvm[thd]->pc+2)<<16)+((unsigned long)*(mem+rvm[thd]->pc+3)<<24);
		rvm[thd]->pc+=4;
		unsigned char t;
		for(t=1; t<=threads; t++) {
			if(e==rvm[t]->entry) {
				thd=t;
				while(rvm[thd]->csp>1) __exit();	// first exit from all nested functions
				__exit();
				break;
			}
		}
	} else xv=-19;
}


void __goto(void) {
	rvm[thd]->pc=(unsigned long)*(mem+rvm[thd]->pc)+((unsigned long)*(mem+rvm[thd]->pc+1)<<8)+((unsigned long)*(mem+rvm[thd]->pc+2)<<16)+((unsigned long)*(mem+rvm[thd]->pc+3)<<24);
}


// call RVM function at given address
void call_addr(unsigned long addr, unsigned long rtnaddr) {
	rvm[thd]->callst[rvm[thd]->csp]=rtnaddr;
	if(++(rvm[thd]->csp)>=MAX_NESTED) xv=-9;
	if(rvm[thd]->dsp) memcpy((unsigned char *)rvm[thd]->params,(unsigned char *)rvm[thd]->stack,(rvm[thd]->dsp*sizeof(rdata_t)));
	for(rvm[thd]->plp=rvm[thd]->dsp; rvm[thd]->plp<MAX_STACK; rvm[thd]->plp++) rvm[thd]->params[rvm[thd]->plp].type=rINVALID;
	rvm[thd]->dsp=0;
	rvm[thd]->plp=0;
	rvm[thd]->local[rvm[thd]->lvidx]=rvm[thd]->vmv;
	while(rvm[thd]->local[rvm[thd]->lvidx] && rvm[thd]->local[rvm[thd]->lvidx]->next) rvm[thd]->local[rvm[thd]->lvidx]=(vmvar_t *)rvm[thd]->local[rvm[thd]->lvidx]->next;	// store the last non-local variable address
	if(++(rvm[thd]->lvidx)>=MAX_NESTED) xv=-9;
	else rvm[thd]->pc=addr;
}


void __call(void) {
	call_addr((unsigned long)*(mem+rvm[thd]->pc)+((unsigned long)*(mem+rvm[thd]->pc+1)<<8)+((unsigned long)*(mem+rvm[thd]->pc+2)<<16)+((unsigned long)*(mem+rvm[thd]->pc+3)<<24),
				(rvm[thd]->pc+4));
}


void __callnt(void) {
	if(threads<MAX_THREADS) {
        xalloc((unsigned char **)&rvm[threads+1],sizeof(rvm_t));
        if(!rvm[threads]) {
            xv=-7;
            return;
        }
        threads++;
		memcpy((unsigned char *)rvm[threads],(unsigned char *)rvm[thd],sizeof(rvm_t));
		rvm[thd]->pc+=4;	// the current thread will continue with the instruction after the token
		thd=threads;
		rvm[thd]->csp=0;	// make the new thread root
		__call();
		rvm[thd]->entry=rvm[thd]->pc;	// record the entry point
	} else xv=-18;
}


void __ifnot(void) {
	rdata_t d;
	pull(&d);
	if(d.type!=rINVALID) {
		unsigned long a=(unsigned long)*(mem+rvm[thd]->pc)+((unsigned long)*(mem+rvm[thd]->pc+1)<<8)+
							((unsigned long)*(mem+rvm[thd]->pc+2)<<16)+((unsigned long)*(mem+rvm[thd]->pc+3)<<24);
		rvm[thd]->pc+=4;
		if(d.type==rSINT64) {
			if(!d.data.sint64) rvm[thd]->pc=a;
		}
		else if(d.type==rREAL) {
			if(d.data.real==0.0) rvm[thd]->pc=a;
		}
		else if(d.type==rTEXT) {
			if(!d.data.text || *(d.data.text)==0) rvm[thd]->pc=a;
		}
		else xv=-4;
	} else xv=-4;
}


void __data(void) {
	rtype_t t=(rtype_t)*(mem+(rvm[thd]->pc++));	// get the data type
	unsigned short vid=((unsigned short)*(mem+rvm[thd]->pc)+((unsigned short)*(mem+rvm[thd]->pc+1)<<8));
	rvm[thd]->pc+=2;
	unsigned long cnt=(unsigned long)*(mem+rvm[thd]->pc)+((unsigned long)*(mem+rvm[thd]->pc+1)<<8)+
							((unsigned long)*(mem+rvm[thd]->pc+2)<<16)+((unsigned long)*(mem+rvm[thd]->pc+3)<<24);
	rvm[thd]->pc+=4;
	signed char dl=tokens[t].inc;
	if(dl<0) dl=ptrlen;	// TEXT
	vmvar_t *v=rvm[thd]->vmv;
	while(v && v->next) v=(vmvar_t *)v->next;
	vmvar_t *n=NULL;
    xalloc((unsigned char **)&n,sizeof(vmvar_t)+dl);
	if(n) {
		if(v) v->next=n; else rvm[thd]->vmv=n;
		n->next=NULL;
		n->type=t;
		n->flags=vfCONST;	// constant
		if(t==rANY) {
			n->flags|=vfMORPH;	// can morph
			((rdata_t *)((unsigned char *)n+sizeof(vmvar_t)))->type=rANY;
		}
		if(n->type!=rTEXT) n->len=dl; else n->len=ULONG_MAX;
		n->dim[0]=cnt;
		n->addr=(unsigned char *)(mem+rvm[thd]->pc);
		n->vid=vid;
		if((n->flags & vfMORPH)==0) {	// defined data types
			if(tokens[t].inc<0) {	// TEXT
				unsigned char *w=(unsigned char *)((unsigned char *)n+sizeof(vmvar_t));
				memcpy((unsigned char *)w,BLANK,ptrlen);
				while(cnt--) {	// skip (cnt) number of ASCIIZ strings
					while(*(mem+rvm[thd]->pc)) rvm[thd]->pc++;
					rvm[thd]->pc++;
				}
			}
			else rvm[thd]->pc+=(cnt*dl);	// ordinal types
		} else {	// data type "any"
			while(cnt--) {
				t=(rtype_t)*(mem+(rvm[thd]->pc++));	// get the data type for the single element
				dl=tokens[t].inc;
				if(dl>0) rvm[thd]->pc+=dl;	// ordinal types
				else if(dl<0) {	// TEXT
					while(*(mem+rvm[thd]->pc)) rvm[thd]->pc++;
					rvm[thd]->pc++;
				}
				else xv=-4;
			}
		}
	} else xv=-7;
	v=n;
	rvm[thd]->ix=0;
}


// declare a new variable
void initvar(rrole_t role) {
	rtype_t t=(rtype_t)*(mem+(rvm[thd]->pc++));	// get the data type
	signed char dl=tokens[t].inc;
	if(dl<0) dl=ptrlen;	// TEXT
	vmvar_t *v=rvm[thd]->vmv;
	while(v && v->next) v=(vmvar_t *)v->next;
	while(!xv && (*(mem+rvm[thd]->pc) || *(mem+rvm[thd]->pc+1))) {
		unsigned short vid=(*(mem+rvm[thd]->pc)+(unsigned short)(*(mem+rvm[thd]->pc+1)<<8));
		vmvar_t *n=NULL;
        xalloc((unsigned char **)&n,sizeof(vmvar_t)+dl);
		if(n) {
			if(v) v->next=n; else rvm[thd]->vmv=n;
			n->next=NULL;
			n->vid=vid;
			n->type=t;
			n->flags=0;
			if(t==rANY) {
				n->flags|=vfMORPH;	// can morph
				((rdata_t *)((unsigned char *)n+sizeof(vmvar_t)))->type=rANY;
			}
			if(n->type!=rTEXT) n->len=dl; else n->len=ULONG_MAX;
			n->addr=NULL;
			if(tokens[t].inc<0) {	// TEXT
				unsigned char *w=(unsigned char *)((unsigned char *)n+sizeof(vmvar_t));
				memcpy((unsigned char *)w,BLANK,ptrlen);
			}
			if(role==rrINPUT || role==rrREFER) {
				rdata_t d;
				memcpy((unsigned char *)&d,(unsigned char *)&(rvm[thd]->params[rvm[thd]->plp]),sizeof(rdata_t));
				if(role==rrINPUT) {
					convert(&d,t);
					memcpy((unsigned char *)&(rvm[thd]->params[rvm[thd]->plp]),(unsigned char *)&d,sizeof(rdata_t));
					if(t!=rANY)
						memcpy(((unsigned char *)n+sizeof(vmvar_t)),(unsigned char *)&d.data,dl);
					else
						memcpy(((unsigned char *)n+sizeof(vmvar_t)),(unsigned char *)&d,dl);
				}
				else {	// rrREFER type
					if(d.type==rSINT32) {
						if((d.data.sint64 & REFMASK)!=REFVAL) xv=-17;	// check whether the parameter is a variable id
						unsigned short vid1=(unsigned short)(d.data.sint64 & USHRT_MAX);
						vmvar_t *x1=rvm[thd]->vmv;
						while(x1 && x1->vid!=vid1) x1=(vmvar_t *)x1->next;
						if(!xv && x1) {
							n->flags|=vfREFER;
							n->addr=x1;
							if(x1) d.type=x1->type; else xv=-5;
							if(dl>=tokens[d.type].inc) {
								if(t!=rANY && d.type!=rANY && t!=d.type && (t==rTEXT || d.type==rTEXT)) xv=-4;
							} else xv=-4;
						} else if(!xv) xv=-17;
					} else xv=-17;
				}
				rvm[thd]->plp++;
			}
		} else xv=-7;
		v=n;
		rvm[thd]->pc+=2;
	}
	rvm[thd]->pc+=2;	// skip the 0000
	rvm[thd]->ix=0;
}


void __var(void) {
	initvar(rrNONE);
}


void __varin(void) {
	initvar(rrINPUT);
}


void __varout(void) {
	initvar(rrOUTPUT);
}


void __varref(void) {
	initvar(rrREFER);
}


void __vafix(void) {
	unsigned short vid=((unsigned short)*(mem+rvm[thd]->pc)+((unsigned short)*(mem+rvm[thd]->pc+1)<<8));
	rvm[thd]->pc+=2;
	rvm[thd]->ix=0;
	vmvar_t *x=rvm[thd]->vmv;
	while(x && x->vid!=vid) x=(vmvar_t *)x->next;
	if(x && (x->flags & vfCONST)==0) {
		if(x->flags & vfREFER) x=(vmvar_t *)x->addr;
		if(!x) {
			xv=-5;
			return;
		}
		rdata_t d;
		pullu(&d);
		if(!xv) {
            signed char dl=tokens[x->type].inc;
			if(dl>1) d.data.sint64&=~(dl-1);	// address alignment
			x->addr=(void *)((unsigned long)d.data.sint64);
		}
		else xv=-4;
	} else xv=-5;
}


void __varmaxl(void) {
	unsigned short vid=((unsigned short)*(mem+rvm[thd]->pc)+((unsigned short)*(mem+rvm[thd]->pc+1)<<8));
	rvm[thd]->pc+=2;
	rvm[thd]->ix=0;
	vmvar_t *x=rvm[thd]->vmv;
	while(x && x->vid!=vid) x=(vmvar_t *)x->next;
	if(x && (x->flags & vfCONST)==0) {
		if(x->flags & vfREFER) x=(vmvar_t *)x->addr;
		if(!x) {
			xv=-5;
			return;
		}
		rdata_t d;
		pullu(&d);
		if(!xv && (x->type==rTEXT)) x->len=(unsigned long)d.data.sint64; else xv=-4;
	} else xv=-5;
}


void __varidx(void) {
	unsigned short vid=((unsigned short)*(mem+rvm[thd]->pc)+((unsigned short)*(mem+rvm[thd]->pc+1)<<8));
	rvm[thd]->pc+=2;
	vmvar_t *x=rvm[thd]->vmv;
	while(x && x->vid!=vid) x=(vmvar_t *)x->next;
	if(!x) {
		xv=-17;
		return;
	}
	if(x->flags & vfREFER) x=(vmvar_t *)x->addr;
	if(!x) {
		xv=-5;
		return;
	}
	rvm[thd]->ix=0;
	if(!xv && x) {
		rdata_t d;
		unsigned long v;
		signed char t,r;
		for(t=MAX_DIMENSIONS-1; t>=0 && !xv; t--) {
			if(x->dim[t]>0) {
				pullu(&d);
				if(!xv && d.data.sint64<x->dim[t]) {
					for(v=1, r=t+1; r<MAX_DIMENSIONS; r++) if(x->dim[r]>0) v*=x->dim[r];
					rvm[thd]->ix+=(d.data.sint64*v);
				}
				else if(!xv) xv=-8;
			}
		}
	}
}


void __vardim(void) {
	unsigned short vid=((unsigned short)*(mem+rvm[thd]->pc)+((unsigned short)*(mem+rvm[thd]->pc+1)<<8));
	rvm[thd]->pc+=2;
	unsigned char dims=*(mem+rvm[thd]->pc++);	// take the needed number of dimensions
	if(dims>MAX_DIMENSIONS) xv=-8;
	rvm[thd]->ix=0;
	vmvar_t *xp=NULL;
	vmvar_t *x=rvm[thd]->vmv;
	while(x && x->vid!=vid) {
		xp=x;
		x=(vmvar_t *)x->next;
	}
	if(!x || (x->flags & vfCONST)==1) {
		xv=-17;
		return;
	}
	if(x->flags & vfREFER) x=(vmvar_t *)x->addr;
	if(!x) {
		xv=-5;
		return;
	}
	if((x->flags & vfCONST)==0) {
		// memset((unsigned char *)(x->dim),0,sizeof(x->dim));
		rdata_t d;
		unsigned long oldy=1;
		unsigned long y=1;
		unsigned char ixx;
		for(ixx=0; ixx<dims && !xv; ixx++) {
			if(x->dim[ixx]) oldy*=x->dim[ixx];
			pullu(&d);
			x->dim[ixx]=d.data.sint64;
			y*=d.data.sint64;
		}
		if(!xv) {
			if(x->type!=rTEXT) {
				y*=x->len;
				oldy*=x->len;
			} else {
				y*=sizeof(char *);
				oldy*=sizeof(char *);
			}
			if(oldy>y) {
				unsigned long r=y;
				while(r<oldy) {
					if(x->type==rTEXT) {
						unsigned char *xc=(unsigned char *)(x+r);
						if(*xc) xfree((unsigned char **)&xc);
					}
					else if(x->type==rANY) {
						rdata_t *xx=(rdata_t *)(x+r);
						if(xx->data.text) xfree((unsigned char **)&xx->data.text);
					}
					if(x->type!=rTEXT) r+=x->len; else r+=sizeof(char *);
				}
				oldy=y;
			}
			vmvar_t *n=NULL;
            xalloc((unsigned char **)&n,sizeof(vmvar_t)+y);
			if(n) {
				memcpy((unsigned char *)n,(unsigned char *)x,sizeof(vmvar_t)+oldy);
				if(xp) xp->next=n; else rvm[thd]->vmv=n;
				xfree((unsigned char **)&x);
			} else xv=-7;
		}
	} else xv=-19;
}


void __set(void) {
	unsigned short vid=((unsigned short)*(mem+rvm[thd]->pc)+((unsigned short)*(mem+rvm[thd]->pc+1)<<8));	// get the var id
	rvm[thd]->pc+=2;
	vmvar_t *x=rvm[thd]->vmv;
	while(x && x->vid!=vid) x=(vmvar_t *)x->next;	// (x) is the variable we need
	if(!x || (x->flags & vfCONST)==1) {
		xv=-17;
		return;
	}
	if(x->flags & vfREFER) x=(vmvar_t *)x->addr;
	if(!x) {
		xv=-5;
		return;
	}
	if((x->flags & vfCONST)==0) {
		rdata_t d;
		pull(&d);
		if(d.type!=rINVALID) {
			convert(&d,x->type);
			unsigned long y;
			if(x->type!=rTEXT) y=(rvm[thd]->ix*(x->len)); else y=(rvm[thd]->ix*sizeof(char *));	// calculate array index
			unsigned char *w;	// (w) points to the data
			if(x->addr) {
				w=(unsigned char *)((unsigned char *)(x->addr)+y);
			} else {
				w=(unsigned char *)((unsigned char *)x+sizeof(vmvar_t)+y);
			}
			if(!w) {
				xv=-20;
				return;
			}
			if(x->type==rANY) {
				memcpy((unsigned char *)w,(unsigned char *)&d,sizeof(rdata_t));
				rdata_t *wd=(rdata_t *)w;
				if(wd->type==rTEXT) {
					char *c=NULL;
					if(d.data.text) {
						if(x->len>0 && x->len<d.data.tlen) {
							xalloc((unsigned char **)&c,(x->len+1));
							if(c) {
								strncpy(c,d.data.text,x->len);
								*(c+x->len)=0;
								xfree((unsigned char **)&d.data.text);
								d.data.text=c;
							} else xv=-7;
						} else c=d.data.text;
					}
					xfree((unsigned char **)wd->data.text);
					memcpy((unsigned char *)&wd->data.text,(unsigned char *)&c,ptrlen);
				}
			}
			else if(x->type==rSINT8) {
				unsigned char j=(unsigned char)(d.data.sint64 & UCHAR_MAX);
				memcpy((unsigned char *)w,(unsigned char *)&j,sizeof(signed char));
			}
			else if(x->type==rSINT16) {
				unsigned short j=(unsigned short)(d.data.sint64 & USHRT_MAX);
				memcpy((unsigned char *)w,(unsigned char *)&j,sizeof(signed short));
			}
			else if(x->type==rSINT32) {
				unsigned long j=(unsigned long)(d.data.sint64 & ULONG_MAX);
				memcpy((unsigned char *)w,(unsigned char *)&j,sizeof(signed long));
			}
			else if(x->type==rSINT64) {
				unsigned long long j=(unsigned long long)(d.data.sint64 & ULONG_LONG_MAX);
				memcpy((unsigned char *)w,(unsigned char *)&j,sizeof(signed long long));
			}
			else if(x->type==rREAL) {
                memcpy((unsigned char *)w,(unsigned char *)&d.data.real,sizeof(double));
			}
			else if(x->type==rTEXT) {
				if(d.type==rTEXT) {
					char *c=NULL;
					if(d.data.text) {
						if(x->len>0 && x->len<d.data.tlen) {
							xalloc((unsigned char **)&c,(x->len+1));
							if(c) {
								strncpy(c,d.data.text,x->len);
								*(c+x->len)=0;
								xfree((unsigned char **)&d.data.text);
								d.data.text=c;
							} else xv=-7;
						} else c=d.data.text;
					}
					xfree((unsigned char **)w);
					memcpy((unsigned char *)w,(unsigned char *)&c,ptrlen);
				} else xv=-4;
			}
			else if(x->type==rFUNC) {
				memcpy((unsigned char *)w,(unsigned char *)&d.data.sint64,sizeof(unsigned long));	// reference to the address
			}
			rvm[thd]->ix=0;
		} else xv=-4;
	} else xv=-19;
}


void __get(void) {
	unsigned short vid=((unsigned short)*(mem+rvm[thd]->pc)+((unsigned short)*(mem+rvm[thd]->pc+1)<<8));	// get the var id
	rvm[thd]->pc+=2;
	vmvar_t *x=rvm[thd]->vmv;
	while(x && x->vid!=vid) x=(vmvar_t *)x->next;	// (x) is the variable we need
	if(!x) {
		xv=-17;
		return;
	}
	if(x && (x->flags & vfREFER)) x=(vmvar_t *)x->addr;
	rdata_t d;
	d.type=x->type;
	unsigned long y;
	if((x->flags & vfCONST)==1 && (x->type==rTEXT || x->type==rANY)) y=0;	// calculate array index (doesn't have meaning for TEXT or ANY type constants)
	else if(x->type!=rTEXT) y=(rvm[thd]->ix*(x->len));
	else y=(rvm[thd]->ix*sizeof(char *));
	unsigned char *w;	// (w) points to the data
	if(x->addr) {
		w=(unsigned char *)((unsigned char *)(x->addr)+y);
	} else {
		w=(unsigned char *)((unsigned char *)x+sizeof(vmvar_t)+y);
	}
	if(!w) {
		xv=-20;
		return;
	}
	rtype_t xt=x->type;
	d.data.sint64=0;
	if(xt==rANY) {
		if((x->flags & vfCONST)==0) {	// variables
			memcpy((unsigned char *)&d,(unsigned char *)w,sizeof(rdata_t));
			w=(unsigned char *)&d.data;
			if(d.type==0) d.type=rANY;	// fixing initialised but previously never accessed arrays
			xt=d.type;
		}
		else {	// constants (will have to count them)
			while(rvm[thd]->ix--) {
				rtype_t t=(rtype_t)*(w++);	// get the data type for the single element to skip
				signed char dl=tokens[t].inc;
				if(dl>0) w+=dl;		// ordinal types
				else if(dl<0) {		// TEXT
					while((char)*w) w++;
					w++;
				}
				else xv=-4;
			}
			rvm[thd]->ix=0;	// we already counted them here
			xt=(rtype_t)*(w++);	// get the data type for the single indexed element
			d.type=xt;
		}
	}
	if(xt==rSINT8) {
		memcpy((unsigned char *)&d.data.sint64,(unsigned char *)w,sizeof(signed char));
		d.data.sint64=(signed char)(d.data.sint64 & UCHAR_MAX);
	}
	else if(xt==rSINT16) {
		memcpy((unsigned char *)&d.data.sint64,(unsigned char *)w,sizeof(signed short));
		d.data.sint64=(signed short)(d.data.sint64 & USHRT_MAX);
	}
	else if(xt==rSINT32) {
		memcpy((unsigned char *)&d.data.sint64,(unsigned char *)w,sizeof(signed long));
		d.data.sint64=(signed long)(d.data.sint64 & ULONG_MAX);
	}
	else if(xt==rSINT64) {
		memcpy((unsigned char *)&d.data.sint64,(unsigned char *)w,sizeof(signed long long));
	}
	else if(xt==rREAL) {
        memcpy((unsigned char *)&d.data.real,(unsigned char *)w,sizeof(double));
	}
	else if(xt==rTEXT) {
		if((x->flags & vfCONST)==1) {
			while(rvm[thd]->ix--) {	// skip (rvm[thd]->ix) number of ASCIIZ strings
				while(*w) w++;
				w++;
			}
			memcpy((unsigned char *)&d.data.text,(unsigned char *)&w,ptrlen);
		}
		else memcpy((unsigned char *)&d.data.text,(unsigned char *)w,ptrlen);
	}
	else if(xt==rFUNC) {	// "reading" a function variable causes execution of the function
		rvm[thd]->callst[rvm[thd]->csp]=rvm[thd]->pc;
		if(++(rvm[thd]->csp)>=MAX_NESTED) xv=-9;
		if(rvm[thd]->dsp) memcpy((unsigned char *)rvm[thd]->params,(unsigned char *)rvm[thd]->stack,(rvm[thd]->dsp*sizeof(rdata_t)));
		for(rvm[thd]->plp=rvm[thd]->dsp; rvm[thd]->plp<MAX_STACK; rvm[thd]->plp++) rvm[thd]->params[rvm[thd]->plp].type=rINVALID;
		rvm[thd]->dsp=0;
		rvm[thd]->plp=0;
		rvm[thd]->local[rvm[thd]->lvidx]=rvm[thd]->vmv;
		while(rvm[thd]->local[rvm[thd]->lvidx] && rvm[thd]->local[rvm[thd]->lvidx]->next) rvm[thd]->local[rvm[thd]->lvidx]=(vmvar_t *)rvm[thd]->local[rvm[thd]->lvidx]->next;	// store the last non-local variable address
		if(++(rvm[thd]->lvidx)>=MAX_NESTED) xv=-9;
		else memcpy((unsigned char *)&rvm[thd]->pc,(unsigned char *)w,sizeof(signed long));
	}
	else if(xt!=rANY) xv=-4;
	if(!xv && xt!=rINVALID && xt!=rFUNC) push(&d);
	rvm[thd]->ix=0;
}


void __type(void) {
	rdata_t d;
	pull(&d);
	d.data.sint64=(signed long long)d.type;
	d.type=rSINT8;
	push(&d);
}


void __count(void) {
	rdata_t d;
	pullref(&d);
	if(!xv) {
		unsigned short vid=(unsigned short)(d.data.sint64 & USHRT_MAX);
		vmvar_t *x=rvm[thd]->vmv;
		while(x && x->vid!=vid) x=(vmvar_t *)x->next;
		if(!xv && x) {
			d.type=rSINT64;
			d.data.sint64=1;
			unsigned char t;
			for(t=0; t<MAX_DIMENSIONS; t++) if(x->dim[t]) d.data.sint64*=x->dim[t];
			push(&d);
		}  else xv=-17;
	}
}


void __equal(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv) {
		if((d1.type==rSINT64 && d1.data.sint64==d2.data.sint64) ||
		(d1.type==rREAL && d1.data.real==d2.data.real) ||
		(d1.type==rTEXT && !strcmp(d1.data.text,d2.data.text))) d1.data.sint64=1;
		else d1.data.sint64=0;
		d1.type=rSINT8;
		push(&d1);
	}
}


void __nequal(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv) {
		if((d1.type==rSINT64 && d1.data.sint64==d2.data.sint64) ||
		(d1.type==rREAL && d1.data.real==d2.data.real) ||
		(d1.type==rTEXT && !strcmp(d1.data.text,d2.data.text))) d1.data.sint64=0;
		else d1.data.sint64=1;
		d1.type=rSINT8;
		push(&d1);
	}
}


void __smaller(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if((d1.type==rSINT64 && d1.data.sint64<d2.data.sint64) ||
		(d1.type==rREAL && d1.data.real<d2.data.real) ||
		(d1.type==rTEXT && strcmp(d1.data.text,d2.data.text)<0)) d1.data.sint64=1;
		else d1.data.sint64=0;
		d1.type=rSINT8;
		push(&d1);
	} else xv=-4;
}


void __smequal(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if((d1.type==rSINT64 && d1.data.sint64<=d2.data.sint64) ||
		(d1.type==rREAL && d1.data.real<=d2.data.real) ||
		(d1.type==rTEXT && strcmp(d1.data.text,d2.data.text)<=0)) d1.data.sint64=1;
		else d1.data.sint64=0;
		d1.type=rSINT8;
		push(&d1);
	} else xv=-4;
}


void __grequal(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if((d1.type==rSINT64 && d1.data.sint64>=d2.data.sint64) ||
		(d1.type==rREAL && d1.data.real>=d2.data.real) ||
		(d1.type==rTEXT && strcmp(d1.data.text,d2.data.text)>=0)) d1.data.sint64=1;
		else d1.data.sint64=0;
		d1.type=rSINT8;
		push(&d1);
	} else xv=-4;
}


void __greater(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if((d1.type==rSINT64 && d1.data.sint64>d2.data.sint64) ||
		(d1.type==rREAL && d1.data.real>d2.data.real) ||
		(d1.type==rTEXT && strcmp(d1.data.text,d2.data.text)>0)) d1.data.sint64=1;
		else d1.data.sint64=0;
		d1.type=rSINT8;
		push(&d1);
	} else xv=-4;
}


void __add(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			d1.data.sint64+=d2.data.sint64;
			cast(&d1.data.sint64,&d1);
		}
		else if(d1.type==rREAL) d1.data.real+=d2.data.real;
		else if(d1.type==rTEXT) {
			char *c=NULL;
			if(d1.data.text && d2.data.text) {
				xalloc((unsigned char **)&c,(d1.data.tlen+d2.data.tlen+1));
				if(c) {
					strcpy(c,d1.data.text);
					strcpy(&c[d1.data.tlen],d2.data.text);
				} else xv=-7;
			}
			else if(d1.data.text==NULL && d2.data.text) {
				xalloc((unsigned char **)&c,(d2.data.tlen+1));
				if(c) strcpy(c,d2.data.text); else xv=-7;
			}
			else if(d1.data.text && d2.data.text==NULL) {
				xalloc((unsigned char **)&c,(d1.data.tlen+1));
				if(c) strcpy(c,d1.data.text); else xv=-7;
			}
			d1.data.text=c;
		}
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __sub(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			d1.data.sint64-=d2.data.sint64;
			cast(&d1.data.sint64,&d1);
		}
		else if(d1.type==rREAL) d1.data.real-=d2.data.real;
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __mul(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			d1.data.sint64*=d2.data.sint64;
			cast(&d1.data.sint64,&d1);
		}
		else if(d1.type==rREAL) d1.data.real*=d2.data.real;
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __div(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			if(d2.data.sint64) {
				if(d1.data.sint64%d2.data.sint64) {
					d1.data.real=(((double)d1.data.sint64*1.0)/((double)d2.data.sint64*1.0));
					d1.type=rREAL;
				}
				else {
					d1.data.sint64/=d2.data.sint64;
					cast(&d1.data.sint64,&d1);
				}
			} else xv=-6;
		}
		else if(d1.type==rREAL) if(d2.data.real) d1.data.real/=d2.data.real; else xv=-6;
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __idiv(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			if(d2.data.sint64) d1.data.sint64/=d2.data.sint64; else xv=-6;
			cast(&d1.data.sint64,&d1);
		}
		else if(d1.type==rREAL) if(d2.data.real) d1.data.real=xround(d1.data.real/d2.data.real); else xv=-6;
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __mod(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			if(d2.data.sint64) d1.data.sint64%=d2.data.sint64; else xv=-6;
			cast(&d1.data.sint64,&d1);
		}
		else if(d1.type==rREAL) {
			if(d2.data.real) d1.data.real=(d1.data.real/d2.data.real)-floor(d1.data.real/d2.data.real); else xv=-6;
		}
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __and(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			d1.data.sint64&=d2.data.sint64;
			cast(&d1.data.sint64,&d1);
		}
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __or(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			d1.data.sint64|=d2.data.sint64;
			cast(&d1.data.sint64,&d1);
		}
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __xor(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			d1.data.sint64^=d2.data.sint64;
			cast(&d1.data.sint64,&d1);
		}
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __bit(void) {
	rdata_t d1;
	pull(&d1);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64 && d1.data.sint64>=0) {
			unsigned char b=(unsigned char)(d1.data.sint64 & 63);
			d1.data.sint64=1;
			while(b--) d1.data.sint64*=2;	// all this is needed only because the << operator only works with 32 bits
			cast(&d1.data.sint64,&d1);
		}
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __shiftl(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64 && d2.data.sint64>=0) {
			d2.data.sint64&=63;
			while(d2.data.sint64--) d1.data.sint64*=2;
			cast(&d1.data.sint64,&d1);
		}
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __shiftr(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64 && d2.data.sint64>=0) {
			d2.data.sint64&=63;
			while(d2.data.sint64--) d1.data.sint64/=2;
			cast(&d1.data.sint64,&d1);
		}
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __not(void) {
	rdata_t d1;
	pull(&d1);
	if(d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			d1.data.sint64=!d1.data.sint64;
			cast(&d1.data.sint64,&d1);
		}
		else if(d1.type==rREAL) {
			if(d1.data.real!=0.0) d1.data.real=0.0; else d1.data.real=1.0;
		}
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __neg(void) {
	rdata_t d1;
	pull(&d1);
	if(d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			d1.data.sint64=~d1.data.sint64;
			cast(&d1.data.sint64,&d1);
		}
		else if(d1.type==rREAL) d1.data.real=-d1.data.real;
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __power(void) {
	rdata_t d1,d2;
	pull(&d2);
	pull(&d1);
	common(&d1,&d2);
	if(!xv && d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			if(d1.data.sint64==0 && d2.data.sint64<=0) xv=-3;
			else {
				d1.data.sint64=pow(d1.data.sint64,d2.data.sint64);
				cast(&d1.data.sint64,&d1);
			}
		}
		else if(d1.type==rREAL) {
			if(d1.data.real<=0 && d2.data.real!=floor(d2.data.real)) xv=-3;
			else d1.data.real=pow(d1.data.real,d2.data.real);
		}
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __inc(void) {
	rdata_t d1;
	pull(&d1);
	if(d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			d1.data.sint64++;
			cast(&d1.data.sint64,&d1);
		}
		else if(d1.type==rREAL) d1.data.real++;
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __dec(void) {
	rdata_t d1;
	pull(&d1);
	if(d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			d1.data.sint64--;
			cast(&d1.data.sint64,&d1);
		}
		else if(d1.type==rREAL) d1.data.real--;
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __abs(void) {
	rdata_t d1;
	pull(&d1);
	if(d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			d1.data.sint64=abs(d1.data.sint64);
			cast(&d1.data.sint64,&d1);
		}
		else if(d1.type==rREAL) d1.data.real=fabs(d1.data.real);
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __sign(void) {
	rdata_t d1;
	pull(&d1);
	if(d1.type!=rINVALID) {
		if(d1.type==rSINT64) {
			if(d1.data.sint64<0) d1.data.sint64=-1;
			else if(d1.data.sint64>0) d1.data.sint64=1;
			else d1.data.sint64=0;
			cast(&d1.data.sint64,&d1);
		}
		else if(d1.type==rREAL) {
			if(d1.data.real<0.0) d1.data.sint64=-1;
			else if(d1.data.real>0.0) d1.data.sint64=1;
			else d1.data.sint64=0;
			cast(&d1.data.sint64,&d1);
		}
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __trim(void) {
	rdata_t d1;
	pull(&d1);
	if(d1.type!=rINVALID) {
		if(d1.type==rSINT64) cast(&d1.data.sint64,&d1);
		else if(d1.type==rREAL) {
			if(d1.data.real<LONG_LONG_MIN || d1.data.real>LONG_LONG_MAX) xv=-10;
			else {
				d1.data.sint64=(signed long long)xround(d1.data.real);
				cast(&d1.data.sint64,&d1);
			}
		}
		else xv=-4;
		push(&d1);
	} else xv=-4;
}


void __size(void) {
	rdata_t d;
	pullref(&d);
	if(xv) {	// the argument is not reference
		xv=0;

		// emulate the rest of the pull() function here
		if(d.type==rSINT8 || d.type==rSINT16 || d.type==rSINT32) convert(&d,rSINT64);
		else if(d.type==rTEXT) {
			if(d.data.text) d.data.tlen=strlen(d.data.text);
			else {
				d.data.text=(char *)&BLANK;	// making sure invalid texts are properly blanked
				d.data.tlen=0;
			}
		}

		if(d.type!=rINVALID) {
			if(d.type==rTEXT) d.data.sint64=(signed long long)d.data.tlen;
			else {
				if(d.type==rSINT64) {
					cast(&d.data.sint64,&d);
					if(d.type==rSINT8) d.data.sint64=sizeof(signed char);
					else if(d.type==rSINT16) d.data.sint64=sizeof(signed short);
					else if(d.type==rSINT32) d.data.sint64=sizeof(signed long);
					else d.data.sint64=sizeof(signed long long);
				}
				else if(d.type==rREAL) d.data.sint64=sizeof(double);
				else d.data.sint64=0;
			}
		} else xv=-4;
	}

	else {	// the argument is reference
		unsigned short vid=(unsigned short)(d.data.sint64 & USHRT_MAX);
		vmvar_t *x=rvm[thd]->vmv;
		while(x && x->vid!=vid) x=(vmvar_t *)x->next;
		if(!x) {
			xv=-5;
			return;
		}
		if(x->type!=rINVALID) {
			unsigned long t,z=1;
			for(t=0; t<MAX_DIMENSIONS; t++) if(x->dim[t]>0) z*=x->dim[t];
			if(x->type==rTEXT) {
				d.data.sint64=0;
				for(t=0; t<z; t++) {
					char *w=(char *)x+sizeof(vmvar_t);
					d.data.sint64+=strlen(w);
					w+=sizeof(char *);
				}
			}
			else d.data.sint64=z*x->len;
		} else xv=-4;
	}

	d.type=rSINT64;
	push(&d);
}


void __sin(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=sin(d1.data.real);
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __arcsin(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=asin(d1.data.real);
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __cos(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=cos(d1.data.real);
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __arccos(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=acos(d1.data.real);
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __sinh(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=sinh(d1.data.real);
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __tan(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=tan(d1.data.real);
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __arctan(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=atan(d1.data.real);
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __tanh(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=tanh(d1.data.real);
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __ln(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=log(d1.data.real);
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __log(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=log10(d1.data.real);
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __exp(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=exp(d1.data.real);
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __E(void) {
	rdata_t d1;
	d1.type=rREAL;
	d1.data.real=E;
	push(&d1);
}


void __PI(void) {
	rdata_t d1;
	d1.type=rREAL;
	d1.data.real=PI;
	push(&d1);
}


void __deg(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=log(d1.data.real*(180.0/PI));
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __rad(void) {
	rdata_t d1;
	pullr(&d1);
	if(!xv) {
		d1.data.real=log(d1.data.real*(PI/180.0));
		if(!xv && (d1.data.real==HUGE_VAL || d1.data.real==-HUGE_VAL)) xv=-10; else push(&d1);
	}
}


void __cut(void) {
	rdata_t d1,d2,d3;
	pullu(&d3);
	pullu(&d2);
	pullt(&d1);
	if(!xv) {
		unsigned long l=d1.data.tlen;
		if((d2.data.sint64+d3.data.sint64)>=l) {
			if(d2.data.sint64>=l) d2.data.sint64=d3.data.sint64=0;
			else d3.data.sint64=l-d2.data.sint64;
		}
		char *c=NULL, *r=NULL;
        xalloc((unsigned char **)&c,(d3.data.sint64+1));		// cut
		xalloc((unsigned char **)&r,((l-d3.data.sint64)+1));	// remainder
		if(c && r) {
			memcpy((unsigned char *)c,(unsigned char *)(d1.data.text+d2.data.sint64),d3.data.sint64);
			*(c+d3.data.sint64)=0;
			memcpy((unsigned char *)r,(unsigned char *)d1.data.text,d2.data.sint64);
			memcpy((unsigned char *)(r+d2.data.sint64),(unsigned char *)(d1.data.text+d2.data.sint64+d3.data.sint64),(l-d2.data.sint64-d3.data.sint64));
			*(r+l-d3.data.sint64)=0;
		} else {
			xfree((unsigned char **)&c);
			xfree((unsigned char **)&r);
			xv=-7;
		}
		d1.data.text=c;
		d2.data.text=r;
		d1.type=d2.type=rTEXT;
		push(&d2);
		push(&d1);
	}
}


void __asnum(void) {
	rdata_t d1,d2;
	pullt(&d2);
	if(!xv) {
		d1.data.real=strtod(d2.data.text,NULL);
		if(d1.data.real==floor(d1.data.real)) {
			if(d1.data.real>=LONG_LONG_MIN && d1.data.real<=LONG_LONG_MAX) {
				d1.data.sint64=(signed long long)xround(d1.data.real);
				signed long long a=d1.data.sint64;
				if(a<LONG_MIN || a>LONG_MAX) d1.type=rSINT64;
				else if(a<SHRT_MIN || a>SHRT_MAX) d1.type=rSINT32;
				else if(a<SCHAR_MIN || a>SCHAR_MAX) d1.type=rSINT16;
				else d1.type=rSINT8;
			} else d1.type=rREAL;
		} else d1.type=rREAL;
		push(&d1);
	}
}


void __isnum(void) {
	rdata_t d1;
	pull(&d1);
	if(d1.type==rTEXT) {
		char *xp;
		strtod(d1.data.text,&xp);
		if((xp-d1.data.text)==d1.data.tlen) d1.data.sint64=1; else d1.data.sint64=0;
	}
	else if(d1.type==rSINT8 || d1.type==rSINT16 || d1.type==rSINT32 || d1.type==rSINT64 || d1.type==rREAL) d1.data.sint64=1;
	else d1.data.sint64=0;
	d1.type=rSINT8;
	push(&d1);
}


// return 1 if the parameter is a valid Rittle word
void __isword(void) {
	rdata_t d1;
	pullt(&d1);
	if(xv) return;
	if(token(d1.data.text)) d1.data.sint64=1;
	else d1.data.sint64=0;
	d1.type=rSINT8;
	push(&d1);
}


// returns the similarity of two strings as a number between 0 and 1
float similarity0(char *s1, char *s2) {
    int checks=0;
    int matches1=0;
    int x1,x2;
    for (x1=1; x1<=*s1; x1++)
        for (x2=1; x2<=*s2; x2++) {
            if ((s1[x1] & 0xdf)==(s2[x2] & 0xdf)) {
                matches1++;
                x1++;
            }
            if (matches1 || checks==0) checks++;
            if (x1>*s1) break;
        }
    int matches2=0;
    for (x1=*s1; x1>=1; x1--)
        for (x2=*s2; x2>=1; x2--) {
            if ((s1[x1] & 0xdf)==(s2[x2] & 0xdf)) {
                matches2++;
                x1--;
            }
            if (matches2 || checks==0) checks++;
            if (x1<1) break;
        }
    float sim;
    if (checks) sim=((float)(matches1+matches2)/checks); else sim=0;
    return sim;
}


void __sim(void) {
	rdata_t d1,d2;
	pullt(&d2);
	pullt(&d1);
	if(!xv) {
		double fret=0.0;
		char *s1=d1.data.text;
		char *s2=d2.data.text;
		if(s1 && s2) {
			float sim1=similarity0(s1,s2);
			float sim2=similarity0(s2,s1);
			fret=(sim1+sim2)/2;
			if (fret<0) fret=0;
			if (fret>1) fret=1;
			float rl;
			if (*s1>*s2) rl=(float)(*s2)/(*s1); else rl=(float)(*s1)/(*s2);
			fret*=rl;
		}
		else if(!s1 && !s2) fret=1.0;
		d1.type=rREAL;
		d1.data.real=fret;
		push(&d1);
	}
}


void __code(void) {
	rdata_t d1,d2;
	pullu(&d2);
	pullt(&d1);
	if(!xv) {
		if(d2.data.sint64>=d1.data.tlen) xv=-3;
		d1.data.sint64=*(d1.data.text+d2.data.sint64);
		d1.type=rSINT64;
		push(&d1);
	}
}


void __char(void) {
	rdata_t d1;
	pullu(&d1);
	if(!xv) {
		char r=d1.data.sint64;
		d1.type=rTEXT;
        d1.data.text=(char *)&char_buf;
		char_buf=0;
		*d1.data.text=r;
		push(&d1);
	}
}


void __search(void) {
	rdata_t d1,d2,d3;
	pullu(&d3);
	pullt(&d2);
	pullt(&d1);
	if(!xv) {
		char *x=(d2.data.text+d3.data.sint64);
		char *c=strstr(x,d1.data.text);
		if(c) {
			d1.data.sint64=(c-x);
			cast(&d1.data.sint64,&d1);
		} else {
			d1.data.sint64=-1;
			d1.type=rSINT8;
		}
		push(&d1);
	}
}


void __insert(void) {
	rdata_t d1,d2,d3;
	pullu(&d3);
	pullt(&d2);
	pullt(&d1);
	if(!xv) {
		unsigned long long x=d3.data.sint64;
		unsigned long long l=d1.data.tlen+d2.data.tlen;
        char *c=NULL;
		xalloc((unsigned char **)&c,(l+1));
		if(c) {
			if(x>d2.data.tlen) x=d2.data.tlen;
			if(x) strncpy(c,d2.data.text,x);
			strcpy((c+x),d1.data.text);
			strcpy((c+x+d1.data.tlen),(d2.data.text+x));
			d1.data.text=c;
			push(&d1);
		} else xv=-7;
	}
}


void __print(void) {
	rdata_t d;
	while(rvm[thd]->dsp) {
		pullfifo(&d);
		memcpy((unsigned char *)&xres,(unsigned char *)&d,sizeof(rdata_t));
		if(encout) {
			xres.role=rrNONE;
			if(d.type==rSINT64 || d.type==rSINT32 || d.type==rSINT16 || d.type==rSINT8) printf("%lli",d.data.sint64);
			else if(d.type==rREAL) printf("%G",(double)d.data.real);
			else if(d.type==rTEXT && d.data.text) printf("%s",d.data.text);
		} else xres.role=rrOUTPUT;
	}
	__clear();
}


void __conrd(void) {
	rdata_t d;
	d.type=rTEXT;
	d.data.text=NULL;
	char *s=NULL;
    unsigned long maxlen=256;
	xalloc((unsigned char **)&s,maxlen);
	if(s) {
        line_edit("", s, (maxlen-1), 0);
		if(*s) xalloc((unsigned char **)&s,strlen(s)+1);	// trim the size only to used characters
		else xalloc((unsigned char **)&s,1);
		d.data.text=s;
		push(&d);
	} else xv=-7;
}


void __conch(void) {
	rdata_t d;
	d.type=rTEXT;
	d.data.text=(char *)&conch_buf;
	if(kbhit()) conch_buf=getch();
	else conch_buf=EOF;
	push(&d);
}


// get a positive integer number from the source or return -1 if can't get any number
signed long long getnum(char **c) {
	char *s=*c;
	signed long long v;
	if(*s>='0' && *s<='9') {
		v=0;
		while(*s>='0' && *s<='9') {
			v=(10*v)+(*s-'0');
			s++;
		}
	} else v=-1;
	*c=s;
	return v;
}


// parse substring with format flags and length
// [0.1]: 00:none, 01:'>', 10:'<', 11:'^'
// [2.3]: 00:none, 01:'-', 10:'+', 11:reserved
// [4]: pre-fill '#'
// [5]: post-fill '#'
void fmtspecs(char **c) {
	char *s=(*c)+1;
	fmtflags=0;
	fmtlen=-1;
	fmtprec=-1;
	prefill=postfill=' ';
	char hasdp=0;
	char pref=0;
	while(*s && !xv) {
		if(*s=='*') {					// pre-fill and post-fill enabling (requires specified length)
			fmtflags|=(1<<(4+pref));	// '*' must be followed by a mandatory character defining the fill character
			if(!pref) prefill=*(++s); else postfill=*(++s);
			pref=1;
		}
		else if(*s=='>') fmtflags|=1;	// right alignment (requires specified length)
		else if(*s=='<') fmtflags|=2;	// left alignment (requires specified length)
		else if(*s=='^') fmtflags|=3;	// center alignment (requires specified length)
		else if(*s=='-') fmtflags|=4;	// reserved space for sign (only with decimal number)
		else if(*s=='+') fmtflags|=8;	// forced sign (only with decimal number)
		else if(*s=='.') {		// decimal point to specify length of the fraction (only with decimal number)
			if(!hasdp) hasdp=1; else xv=-11;
		}
		else if((*s>='0') && (*s<='9')) {
			if(!hasdp) fmtlen=getnum(&s); else fmtprec=getnum(&s);
			if(!xv) s--;
		}
		else break;
		if(*s && !xv) s++;
	}
	*c=s;
}


void dofmt(rdata_t d, char **output, unsigned long long *blen) {
	char *dst=*output;
	*blen+=fmtlen;
	if(dst) {
		unsigned long long len=fmtlen;
		if(prefill!=' ') {
			if((fmtflags & 0b1100)==0b1000) {
				if(d.data.real<0) *(dst++)='-';
				else if(d.data.real>0) *(dst++)='+';
				else *(dst++)=' ';
				len--;
			}
			else if((fmtflags & 0b1100)==0b0100) {
				if(d.data.real<0) *(dst++)='-'; else *(dst++)=' ';
				len--;
			}
		}
		if(len>=strlen(d.data.text)) {
			unsigned long long b=len-strlen(d.data.text);
			if((fmtflags & 0b0011)==0b0011) {		// centred
				b/=2;
				memset(dst,prefill,b);
				dst+=b;
				strcpy(dst,d.data.text);
				dst+=strlen(d.data.text);
				b=len-(b+strlen(d.data.text));
				memset(dst,postfill,b);
				dst+=b;
			}
			else if((fmtflags & 0b0011)==0b0001) {	// right aligned
				memset(dst,prefill,b);
				dst+=b;
				strcpy(dst,d.data.text);
				dst+=strlen(d.data.text);
			}
			else {	// left aligned
				strcpy(dst,d.data.text);
				dst+=strlen(d.data.text);
				memset(dst,postfill,b);
				dst+=b;
			}
		} else {	// trim to the specified length
			strncpy(dst,d.data.text,len);
			dst+=len;
		}
	}
	*output=dst;
}


// parse format string
// returns the size of the needed output buffer
unsigned long long parsefmt(char *c, char *dst) {
	unsigned long long blen=0;
	signed short params=rvm[thd]->dsp-2;
	rdata_t d;
	while(!xv && *c /*&& rvm[thd]->params[params].type>rINVALID*/) {
		if(*c=='|') {
			c++;
			if(*c=='|') blen++;

			else if(*c=='d' || *c=='D' || *c=='e' || *c=='E') {	// decimal number
				char upf=((*c=='D' || *c=='E')?1:0);
				char epf=((*c=='e' || *c=='E')?1:0);
				fmtspecs(&c);
				get(&d,params);
				if(d.type==rSINT64 || d.type==rREAL) {
					char *bf=(fmtbuf+50);
					*bf=0;
					if(fmtprec>0) {
						if(fmtprec>24) fmtprec=24;	// limit the precision to 24 digits after the decimal point
						if(epf) sprintf(bf,"%%1.%de",fmtprec); else sprintf(bf,"%%1.%df",fmtprec);
					} else {
						if(epf) {
							if(upf) sprintf(bf,"%%E"); else sprintf(bf,"%%e");
						} else {
							if(d.type==rSINT64) sprintf(bf,"%%lli");
							else {
								if(upf) sprintf(bf,"%%F"); else sprintf(bf,"%%f");
							}
						}
					}
					double v;
					if(d.type==rSINT64) v=(double)(1.0*d.data.sint64); else v=d.data.real;
					char *b=fmtbuf;
					if(prefill!=' ') {
						if((fmtflags & 0b1100)==0b1000) {
							if(v<0) *fmtbuf='-';
							else if(v>0) *fmtbuf='+';
							else *fmtbuf=' ';
							b++;
						}
						else if((fmtflags & 0b1100)==0b0100) {
							if(v<0) *fmtbuf='-'; else *fmtbuf=' ';
							b++;
						}
					}
					if(d.type==rSINT64) sprintf(b,bf,d.data.sint64);
					else {
						sprintf(b,bf,(double)v);	// FYI: for some reason the compiler can't do long double...
						if(v!=floor(v)) {
							char *j=fmtbuf+strlen(fmtbuf)-1;
							while(j>fmtbuf && *j=='0') *(j--)=0;	// remove trailing zeros
							if(*j=='.') {
								if(j>fmtbuf) *j=0; else *j='0';
							}
						}
					}
					if(fmtlen<=0 || fmtlen<strlen(fmtbuf)) fmtlen=strlen(fmtbuf);	// the length needs to be adjusted
					d.data.text=fmtbuf;
					dofmt(d,&dst,&blen);
				}
			}

			else if(*c=='x' || *c=='X') {	// hexadecimal number
				char upf=(*c=='X'?1:0);
				fmtspecs(&c);
				if(fmtprec<0 && (fmtflags & 0b1100)==0) {
					get(&d,params--);
					if(d.type==rSINT64) {
						char *b=fmtbuf+sizeof(fmtbuf);
						*(--b)=0;
						while((unsigned long long)d.data.sint64) {
							blen++;
							unsigned char x=((unsigned long long)d.data.sint64 & 0x0f);
							d.data.sint64/=16;
							if(x>9) {
								if(upf) *(--b)=(x-10)+'A'; else *(--b)=(x-10)+'a';
							} else *(--b)=x+'0';
						}
						if(fmtlen<=0 || fmtlen<strlen(b)) fmtlen=strlen(b);	// the length needs to be adjusted
						d.data.text=b;
						dofmt(d,&dst,&blen);
						dst=d.data.text;
					} else xv=-4;
				} else xv=-11;
			}

			else if(*c=='b' || *c=='B') {	// binary number
				fmtspecs(&c);
				if(fmtprec<0 && (fmtflags & 0b1100)==0) {
					get(&d,params--);
					if(d.type==rSINT64) {
						char *b=fmtbuf+sizeof(fmtbuf);
						*(--b)=0;
						while((unsigned long long)d.data.sint64) {
							blen++;
							if((unsigned long long)d.data.sint64 & 1) *(--b)='1'; else *(--b)='0';
							d.data.sint64/=2;
						}
						if(fmtlen<=0 || fmtlen<strlen(b)) fmtlen=strlen(b);	// the length needs to be adjusted
						d.data.text=b;
						dofmt(d,&dst,&blen);
						dst=d.data.text;
					} else xv=-4;
				} else xv=-11;
			}

			else if(*c=='t' || *c=='T') {	// text
				fmtspecs(&c);
				if(fmtprec<0 && (fmtflags & 0b1100)==0) {
					get(&d,params--);
					if(fmtlen<=0 || fmtlen<strlen(d.data.text)) fmtlen=strlen(d.data.text);	// the length needs to be adjusted
					if(d.type==rTEXT) dofmt(d,&dst,&blen); else xv=-4;
				} else xv=-11;
			}

			else xv=-11;

		} else {
			blen++;
			if(dst) *(dst++)=*(c++); else c++;
		}
	}
	if(dst) *dst=0;
	return blen;
}


void __format(void) {
	fmtflags=0;
	fmtlen=-1;
	fmtprec=-1;
	prefill=postfill=' ';
	rdata_t ds,d1;
	pullfifo(&ds);	// get the deepest element from the stack (supposed to be the format string)
	if(ds.type==rTEXT) {
		unsigned long long blen=parsefmt(ds.data.text,NULL);
		if(blen) {
            char *c=NULL;
			xalloc((unsigned char **)&c,(blen+1));
			if(c) {
				parsefmt(ds.data.text,c);
				d1.data.text=c;
				d1.type=rTEXT;
				__clear();	// clear the stack;
				push(&d1);	// return the result
			} else xv=-7;
		}
	} else xv=-11;
}


void __crlf(void) {
	rdata_t d;
	d.type=rTEXT;
    d.data.text=CRLF;
	push(&d);
}


void __freemem(void) {
    rdata_t d;
	d.type=rSINT64;
	d.data.sint64=xtotal();
	push(&d);
}


void __random(void) {
	int z=(rnd() % RAND_MAX);
	rdata_t d1;
	d1.type=rREAL;
	d1.data.real=(1.0*z)/RAND_MAX;
	push(&d1);
}


void __wait(void) {
    if(rvm[thd]->dlycntr>0) {
        if(uptcounter<rvm[thd]->dlycntr) rvm[thd]->pc--;	// stay on the same instruction
        else rvm[thd]->dlycntr=0;
    }
    else {
        rdata_t d;
        pulls(&d);
        if(xv || d.data.sint64==0) return;
        if(d.data.sint64<0) dlyus((unsigned long)(-d.data.sint64));
        else {
            rvm[thd]->dlycntr=uptcounter+d.data.sint64;
            rvm[thd]->pc--; // stay on the same instruction
			dlyus(0);		// this will trigger an error if the dlyus() function is unsupported
        }
    }
}


// return number of microseconds since the start of execution
void __uptime(void) {
	cclkss_old=cclkss;
    cclkss=cclk();
	// increase the uptime counter with the microseconds passed since the last cycle
	uptcounter+=((cclkss>=cclkss_old)? (cclkss-cclkss_old) : (cclkss+(~cclkss_old)+1));
	rdata_t d;
	d.data.sint64=(signed long long)uptcounter;
	d.type=rSINT64;
	push(&d);
}


// platform and version identifiers
void __platform(void) {
    rdata_t d1, d2;
	d1.type=rTEXT;
	d1.data.text=NULL;
	xalloc((unsigned char **)&d1.data.text, strlen(HW_PLATFORM)+1);
	d2.type=rTEXT;
	d2.data.text=NULL;
	xalloc((unsigned char **)&d2.data.text, strlen(SW_VERSION)+1);
	if(d1.data.text==NULL || d2.data.text==NULL) {
		xfree((unsigned char **)&d1.data.text);
		xfree((unsigned char **)&d2.data.text);
		xv=-7;
		return;
	}
	strcpy(d1.data.text,HW_PLATFORM);
	push(&d1);
	strcpy(d2.data.text,SW_VERSION);
	push(&d2);
}


// enable/disable Ctrl-C user break
void __ctrlbrk(void) {
	rdata_t d;
	pull(&d);
	if(d.type>=rSINT8 && d.type<=rSINT64) enbrk=((d.data.sint64!=0)? 1 : 0);
	else if(d.type==rREAL) enbrk=((d.data.real!=0.0)? 1 : 0);
	else if(d.type==rTEXT) enbrk=((d.data.text && *d.data.text)? 1 : 0);
	else xv=-23;
}


// execute function at specified microsecond intervals
void __tick(void) {
	rdata_t dt,df;
	pull(&df);
	if(df.type!=rFUNC) xv=-5;
	pullu(&dt);
	if(xv) return;
	rvm[thd]->tickcall=(unsigned long)df.data.sint64;
	rvm[thd]->tickval=dt.data.sint64;
}


// clear variable or array
void __clrvar(void) {
	rdata_t d;
	pullref(&d);
	if(xv) return;
	unsigned short vid=(unsigned short)(d.data.sint64 & USHRT_MAX);
	vmvar_t *x=rvm[thd]->vmv;
	while(x && x->vid!=vid) x=(vmvar_t *)x->next;
	if(!xv && x) {
		unsigned char *w=(unsigned char *)((unsigned char *)x+sizeof(vmvar_t));
		signed int dl;
		unsigned long t,z=1;
		for(t=0; t<MAX_DIMENSIONS; t++) if(x->dim[t]>0) z*=x->dim[t];
		if(x->type==rTEXT) {
			for(t=0; t<z; t++) {
                if(w>=memory && w<(memory+XMEM_SIZE)) xfree((unsigned char **)w);
				dl=sizeof(char *);
				memcpy(w,BLANK,dl);
				w+=sizeof(char *);
			}
		}
		else {
			dl=tokens[x->type].inc;
			if(dl>0) memset(w,0,(z*dl));
		}
	}
}


void __run(void) {
	rdata_t d;
	pullt(&d);
	if(xv) return;
	if(strlen(d.data.text)>(sizeof(conbuf)-5)) {
		xv=-3;
		return;
	}
	memset(conbuf,0,sizeof(conbuf));
	strcpy(conbuf,"run ");
	strcpy(&conbuf[4],d.data.text);
	xv=3;	// exit and return the line in conbuf[]
}


// FILES ======================================================================

void __mkdir(void) {
	rdata_t d;
	pullt(&d);
	if(!xv) {
		d.data.sint64=f_mkdir(d.data.text);
		d.type=rSINT8;
		push(&d);
	}
}


void __rmdir(void) {
	rdata_t d;
	pullt(&d);
	if(!xv) {
		d.data.sint64=f_rmdir(d.data.text);
		d.type=rSINT8;
		push(&d);
	}
}


void __chdir(void) {
	rdata_t d;
	pullt(&d);
	if(!xv) {
		d.data.sint64=f_chdir(d.data.text);
		d.type=rSINT8;
		push(&d);
	}
}


void __open(void) {
	rdata_t d;
	pullt(&d);	// file name
	if(!xv) {
		unsigned char t;
		for(t=0; t<MAX_FILES && files[t]; t++); // find unused file slot
		if(t>=MAX_FILES) {
			xv=-15;	// no free handlers
			return;
		}
		xalloc((unsigned char **)&files[t],sizeof(FIL));
		if(!files[t]) {
			xv=-7;
			return;
		}
		FRESULT r=f_open(files[t], d.data.text, (FA_READ | FA_WRITE | FA_OPEN_APPEND));
		if(r==FR_OK) d.data.sint64=(signed long long)t;
		else {
			d.data.sint64=-((signed long long)r);
			// xfree((unsigned char **)&files[t]);	// release the file handler if opening was unsuccessful
		}
		d.type=rSINT8;
		push(&d);
	}
}


void __close(void) {
	rdata_t d;
	pullu(&d);	// file handler id
	if(!xv) {
		if(d.data.sint64<0 || d.data.sint64>=MAX_FILES /*|| !files[d.data.sint64]*/) {
			xv=-16;	// invalid handler
			return;
		}
        FRESULT r=FR_OK;
        if(files[d.data.sint64]) r=f_close(files[d.data.sint64]);
		if(r==FR_OK) d.data.sint64=0;
		else d.data.sint64=-((signed long long)r);
		xfree((unsigned char **)&files[d.data.sint64]);
		d.type=rSINT8;
		push(&d);
	}
}


void __isopen(void) {
	rdata_t d;
	pullu(&d);	// file handler id
	if(!xv) {
		if(d.data.sint64<0 || d.data.sint64>=MAX_FILES) {
			xv=-16;	// invalid handler
			return;
		}
		d.data.sint64=!!files[d.data.sint64];
		d.type=rSINT8;
		push(&d);
	}
}


void __eof(void) {
	rdata_t d;
	pullu(&d);	// file handler id
	if(!xv) {
		if(d.data.sint64<0 || d.data.sint64>=MAX_FILES) {
			xv=-16;	// invalid handler
			return;
		}
		if(files[d.data.sint64]) d.data.sint64=(signed long long)f_eof(files[d.data.sint64]);
		else d.data.sint64=-1;
		d.type=rSINT64;
		push(&d);
	}
}


void __fpos(void) {
	rdata_t d;
	pullu(&d);	// file handler id
	if(!xv) {
		if(d.data.sint64<0 || d.data.sint64>=MAX_FILES) {
			xv=-16;	// invalid handler
			return;
		}
		if(files[d.data.sint64]) d.data.sint64=(signed long long)f_tell(files[d.data.sint64]);
		else d.data.sint64=-1;
		d.type=rSINT64;
		push(&d);
	}
}


void __seek(void) {
	rdata_t d,dp;
	pullu(&dp);	// pile pointer position
	pullu(&d);	// file handler id
	if(!xv) {
		if(d.data.sint64<0 || d.data.sint64>=MAX_FILES) {
			xv=-16;	// invalid handler
			return;
		}
		if(files[d.data.sint64]) {
			FRESULT r=f_lseek(files[d.data.sint64], dp.data.sint64);
			if(r==FR_OK) d.data.sint64=0;
			else d.data.sint64=-((signed long long)r);
		}
		else d.data.sint64=-1;
		d.type=rSINT8;
		push(&d);
	}
}


void __ioerr(void) {
	rdata_t d;
	pullu(&d);	// file handler id
	if(!xv) {
		if(d.data.sint64<0 || d.data.sint64>=MAX_FILES) {
			xv=-16;	// invalid handler
			return;
		}
		if(files[d.data.sint64]) d.data.sint64=(signed long long)f_error(files[d.data.sint64]);
		else d.data.sint64=-1;
		d.type=rSINT64;
		push(&d);
	}
}


void __fsize(void) {
	rdata_t d;
	pullu(&d);	// file handler id
	if(!xv) {
		if(d.data.sint64<0 || d.data.sint64>=MAX_FILES) {
			xv=-16;	// invalid handler
			return;
		}
		if(files[d.data.sint64]) {
            d.data.sint64=(signed long long)f_size(files[d.data.sint64]);
            f_rewind(files[d.data.sint64]);
        }
		else d.data.sint64=-1;
		d.type=rSINT64;
		push(&d);
	}
}


void __delete(void) {
	rdata_t d;
	pullt(&d);	// file name
	if(!xv) {
		FRESULT r=f_unlink(d.data.text);
		if(r==FR_OK) d.data.sint64=0;
		else d.data.sint64=-((signed long long)r);
		d.type=rSINT8;
		push(&d);
	}
}


void __rename(void) {
	rdata_t d, dn;
	pullt(&dn);	// file name (new)
	pullt(&d);	// file name (old)
	if(!xv) {
		FRESULT r=f_rename(d.data.text, dn.data.text);
		if(r==FR_OK) d.data.sint64=0;
		else d.data.sint64=-((signed long long)r);
		d.type=rSINT8;
		push(&d);
	}
}


signed long long fcopy(char *cur, char *path) {
	#ifndef COPYBUF_SIZE
		#define COPYBUF_SIZE 4096
	#endif
	if(!cur || !path || *cur==0 || *path==0 || !strcmp(cur,path)) return 0;
	unsigned char *buf=NULL;
	char *cwd=NULL;
	FIL *ff=NULL;
	xalloc((unsigned char **)&buf,COPYBUF_SIZE);	// 4k buffer
    xalloc((unsigned char **)&ff,sizeof(FIL));
	xalloc((unsigned char **)&cwd,sizeof(conbuf));
	if(!buf || !ff || !cwd) {
		xfree((unsigned char **)&buf);
		xfree((unsigned char **)&ff);
		xv=-7;
		return 0;
	}
	FRESULT r=FR_OK;

	if(cur[0] && cur[1] && cur[2] && cur[3]==':') {
		if(r==FR_OK) r=f_chdrive((char *)cur);
		if(r==FR_OK) r=f_mount(&FatFs, "", 1);
	}
	f_getcwd(cwd, (sizeof(conbuf)-1));
	if(r==FR_OK) r=f_open(ff, cur, (FA_READ | FA_OPEN_EXISTING));
	f_close(ff);

	if(path[0] && path[1] && path[2] && path[3]==':') {
		if(r==FR_OK) r=f_chdrive((char *)path);
		if(r==FR_OK) r=f_mount(&FatFs, "", 1);
	}
	if(r==FR_OK) r=f_open(ff, path, (FA_WRITE | FA_CREATE_ALWAYS));
	f_close(ff);

	UINT read=COPYBUF_SIZE, written=0;
	unsigned long cpos=0;
	while(r==FR_OK && read==COPYBUF_SIZE) {

		if(cwd[0] && cwd[1] && cwd[2] && cwd[3]==':') {
			if(r==FR_OK) r=f_chdrive(cwd);
			if(r==FR_OK) r=f_mount(&FatFs, "", 1);
		}
		if(r==FR_OK) r=f_open(ff, cur, (FA_READ | FA_OPEN_EXISTING));
		if(r==FR_OK) r=f_lseek(ff, cpos);
		if(r==FR_OK) r=f_read(ff, buf, COPYBUF_SIZE, &read);
		f_close(ff);
		if(r!=FR_OK || read==0) break;
		cpos+=read;

		if(path[0] && path[1] && path[2] && path[3]==':') {
			if(r==FR_OK) r=f_chdrive((char *)path);
			if(r==FR_OK) r=f_mount(&FatFs, "", 1);
		}
		if(r==FR_OK) r=f_open(ff, path, (FA_WRITE | FA_OPEN_APPEND));
		if(r==FR_OK) r=f_write(ff, buf, read, &written);
		f_close(ff);
		if(r!=FR_OK || written!=read) break;

	}

	xfree((unsigned char **)&cwd);
	xfree((unsigned char **)&buf);
	xfree((unsigned char **)&ff);
	if(r==FR_OK || r==-FR_OK) r=0;
	return (signed long long)(-r);
}


void __copy(void) {
	rdata_t df, dp;
	pullt(&dp);	// path
	pullt(&df);	// file name (old)
	if(!xv) {
		df.data.sint64=(signed long long)fcopy(df.data.text, dp.data.text);
		df.type=rSINT8;
		push(&df);
	}
}


void __drive(void) {
	rdata_t d;
	pullt(&d);	// drive name
	if(!xv) {
		FRESULT r=f_chdrive(d.data.text);
		if(r==FR_OK) r=f_mount(&FatFs,d.data.text,1);
        if(r==FR_OK) d.data.sint64=0;
		else d.data.sint64=-((signed long long)r);
		d.type=rSINT8;
		push(&d);
	}
}


void __init(void) {
	rdata_t d;
	pullt(&d);	// drive
	if(!xv) {
		if(d.data.text && *(d.data.text)) d.data.sint64=ffinit(d.data.text);
        else xv=-14;
		d.type=rSINT64;
		push(&d);
	}
}


void __where(void) {
	rdata_t d;
	d.type=rTEXT;
	d.data.text=NULL;
	char *s=NULL;
	xalloc((unsigned char **)&s,256);
	if(s) {
		FRESULT r=f_getcwd(s,255);
		if(r==FR_OK) xalloc((unsigned char **)&s,strlen(s)+1);	// trim the size only to used characters
		else xalloc((unsigned char **)&s,1);
		d.data.text=s;
		push(&d);
	} else xv=-7;
}


void __write(void) {
	rdata_t ds, d, dh;
	pullu(&ds);		// data size (in bytes)
	pullref(&d);	// data reference
	pullu(&dh);		// file handler
	if(!xv && (dh.data.sint64<0 || dh.data.sint64>=MAX_FILES || !files[d.data.sint64])) xv=-16;	// invalid handler
	if(xv!=0) return;
	unsigned short vid=(unsigned short)(d.data.sint64 & USHRT_MAX);		// reference variable id
	vmvar_t *x=rvm[thd]->vmv;
	while(x && x->vid!=vid) x=(vmvar_t *)x->next;
	if(x) {
		unsigned char *buf=(unsigned char *)((unsigned char *)x+sizeof(vmvar_t));	// pointer to the data buffer
		FRESULT r=FR_OK;
		UINT w=0;
		r=f_write(files[dh.data.sint64], buf, ds.data.sint64, &w);
		d.data.sint64=(signed long long)w;
		if(r!=FR_OK) w=-w;	// negate the number in case of an error
		d.type=rSINT64;
		push(&d);
	}
	else xv=-5;
}


void __read(void) {
	rdata_t ds, d, dh;
	pullu(&ds);		// data size (in bytes)
	pullref(&d);	// data reference
	pullu(&dh);		// file handler
	if(!xv && (dh.data.sint64<0 || dh.data.sint64>=MAX_FILES || !files[d.data.sint64])) xv=-16;	// invalid handler
	if(xv!=0) return;
	unsigned short vid=(unsigned short)(d.data.sint64 & USHRT_MAX);		// reference variable id
	vmvar_t *x=rvm[thd]->vmv;
	while(x && x->vid!=vid) x=(vmvar_t *)x->next;
	if(x) {
		unsigned char *buf=(unsigned char *)((unsigned char *)x+sizeof(vmvar_t));	// pointer to the data buffer
		FRESULT r=FR_OK;
		UINT w=0;
		r=f_read(files[dh.data.sint64], buf, ds.data.sint64, &w);
		d.data.sint64=(signed long long)w;
		if(r!=FR_OK) w=-w;	// negate the number in case of an error
		d.type=rSINT64;
		push(&d);
	}
	else xv=-5;
}


void __ffirst(void) {
	rdata_t d;
	pullt(&d);
	if(xv!=0) return;
	FRESULT fe=f_findfirst(&(rvm[thd]->dir), &(rvm[thd]->finfo), "", d.data.text);
	xfree((unsigned char **)&d.data.text);
	if(fe==FR_OK && rvm[thd]->finfo.fname) {
		unsigned long l=(strlen(rvm[thd]->finfo.fname)+1);
		xalloc((unsigned char **)&d.data.text,l);
		if(d.data.text) strcpy(d.data.text,rvm[thd]->finfo.fname);
		else xv=-7;
	}
	else xalloc((unsigned char **)&d.data.text,1);
	d.type=rTEXT;
	if(d.data.text) push(&d);
	else xv=-7;
}


void __fnext(void) {
	rdata_t d;
	d.type=rTEXT;
	d.data.text=NULL;
	FRESULT fe=f_findnext(&(rvm[thd]->dir), &(rvm[thd]->finfo));
	if(fe==FR_OK && rvm[thd]->finfo.fname) {
		unsigned long l=(strlen(rvm[thd]->finfo.fname)+1);
		xalloc((unsigned char **)&d.data.text,l);
		if(d.data.text) strcpy(d.data.text,rvm[thd]->finfo.fname);
		else xv=-7;
	}
	else xalloc((unsigned char **)&d.data.text,1);
	if(d.data.text) push(&d);
	else xv=-7;
}


// TOUCH DRIVER ==========================================================

// top level touch interface primitive drivers
void (*touch_InitPort)(void) = null_InitPort;
void (*touch_Detach)(void) = null_Detach;
void (*touch_ReadTouch)(void) = null_ReadTouch;

void null_ReadTouch(void) { touch_points=0; }


// NULL DISPLAY DRIVER ========================================================

void null_InitPort(void) {}
void null_Detach(void) {}
void null_Cls(void) {}
void null_ScrollUp(void) {}
void null_DrawRect(int x1, int y1, int x2, int y2, long col) {}
void null_ReadRect(int x1, int y1, int x2, int y2, long *colarray) {}

// forward declaration needed for font smoothing (only)
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, long c);


// output with the system font
void gPutChr(int ch) {
	unsigned short c,th=(font->header.blankU+font->header.height+font->header.blankD);
	if(ch==0 && th>0) {	// character 0 is only displaying virtual cursor
		display_DrawRect(posX,posY,posX,(posY+(fontScale*th)-1),0xffffff);
		return;
	}
	if(fontBcol>=0) display_DrawRect(posX,posY,posX,(posY+(fontScale*th)-1),fontBcol);
	else display_DrawRect(posX,posY,posX,(posY+(fontScale*th)-1),0);
	if(ch<=0 || font->header.height==0 || ch<font->header.start || ch>=(font->header.start+font->header.characters)) return;
	if(!no_crtl_chars) {	// control characters are enabled only if (no_crtl_chars) is 0
		if(ch=='\r') {
			posX=0;
			return;
		}
		else if(ch=='\n') {
			posY+=(fontScale*th);
			if(posY+(fontScale*th)>resV) {
				display_ScrollUp();
				posY-=(fontScale*th);
			}
			return;
		}
		else if(ch=='\t') {
			for(c=0; c<strlen(TAB); c++) gPutChr(' ');
			return;
		}
		else if(ch=='\b') {
			if(posX>=(fontScale*lastW)) posX-=(fontScale*lastW); else posX=0;
			display_DrawRect(posX,posY,(posX+lastW-1),(posY+(fontScale*th)-1),fontBcol);
			return;
		}
	}
	int i,j,w,x,y,y0;
	const unsigned char *fcp, *fc=font->definitions;
	ch-=font->header.start;
	if(font->header.width==0) {		// variable width fonts
		while(ch--) fc+=(1+*fc);	// skip the preceding characters
		w=*(fc++);	// get the width for this character
	}
	else {	// fixed width fonts
		fc+=(ch*font->header.width);
		w=font->header.width;
	}
	if((posX+(fontScale*(font->header.blankL+w+font->header.blankR)))>resH) {
		posX=0;
		posY+=(fontScale*th);
	}
	if(posY+(fontScale*th)>resV) {
		display_ScrollUp();
		posY-=(fontScale*th);
	}
	x=posX;
	y0=posY;
	if(font->header.blankL) {
		if(fontBcol>=0) display_DrawRect(x,y0,(x+(fontScale*font->header.blankL)-1),(y0+(fontScale*th)-1),fontBcol);
		x+=(fontScale*font->header.blankL);	// adding blanks to the left
	}
	posX=x;	// this is needed in order to ignore the blank columns when considering smoothing
	for(i=0; i<w; i++, x+=fontScale) {	// columns
		y=posY;
		for(j=0; j<font->header.blankU; j++) {	// adding blanks to the top
			if(fontBcol>=0) display_DrawRect(x,y,(x+fontScale-1),(y+fontScale-1),fontBcol);
			y+=fontScale;
		}
		if(fontBcol>=0) display_DrawRect(x,y,(x+fontScale-1),(y+(font->header.height*fontScale)-1),fontBcol);
		unsigned long pz=0;
		unsigned long z=0;
		for(c=0, j=0; j<font->header.height; c++, j++, z>>=1, pz>>=1) {	// lines
			if((c & 7)==0) {
				z=((unsigned short)*(fc++))<<1;
				if(x>posX) {
					pz=2+((font->header.height-1)/8);
					fcp=(unsigned char *)(fc-pz);
					pz=(unsigned short)*fcp;
					if(font->header.height>8) pz+=((unsigned short)*(fcp+1)<<8);
					pz<<=1;
				}
			}
			if((z & BIT(1)) && fontFcol>=0) {
				display_DrawRect(x,y,(x+fontScale-1),(y+fontScale-1),fontFcol);
				if(fontScale>2) {	// font smoothing
					if((j>0) && (pz & !(pz & BIT(1)) && BIT(0)) && !(z & BIT(0))) {
						draw_triangle(x,y,(x+fontScale-1),y,x,(y-fontScale+1),fontFcol);
						draw_triangle(x,y,(x-fontScale+1),y,x,(y+fontScale-1),fontFcol);
					}
					if((j<(font->header.height-1)) && !(pz & BIT(1)) && (pz & BIT(2)) && !(z & BIT(2))) {
						draw_triangle(x,(y+fontScale-1),(x-fontScale+1),(y+fontScale-1),x,y,fontFcol);
						draw_triangle(x,(y+fontScale-1),(x+fontScale-1),(y+fontScale-1),x,(y+2*fontScale-2),fontFcol);
					}
				}
			}
			y+=fontScale;
		}
		for(j=0; j<font->header.blankD; j++) {	// adding blanks to the bottom
			if(fontBcol>=0) display_DrawRect(x,y,(x+fontScale-1),(y+fontScale-1),fontBcol);
			y+=fontScale;
		}
	}
	if(font->header.blankR) {
		if(fontBcol>=0) display_DrawRect(x,y0,(x+(fontScale*font->header.blankR)-1),(y0+(fontScale*th)-1),fontBcol);
		x+=(fontScale*font->header.blankR);	// adding blanks to the right
	}
	posX=x;
	lastW=font->header.blankL+w+font->header.blankR;
}


void null_PutChr(int ch) {
	font=&sysFont0508;	// forcing the system font to be used in the console
	gPutChr(ch);
}


void null_Attach(void) {
	display_InitPort=null_InitPort;
	display_Detach=null_Detach;
	display_Cls=null_Cls;
	display_PutChr=null_PutChr;
	display_ScrollUp=null_ScrollUp;
	display_DrawRect=null_DrawRect;
	display_ReadRect=null_ReadRect;
	resH=resV=1;
	posX=posY=0;
	orientation=LANDSCAPE;
	font=&sysFont0508;
	fontScale=1;
	fontFcol=LONG_MAX;
	fontBcol=0;
	lastW=0;
}


// GRAPHIC USER INTERFACE FUNCTIONS ===========================================

void __gget(void) {
	rdata_t dx1,dy1,dx2,dy2,dvar;
	pullref(&dvar);	// referred variable
	pulls(&dy2);
	pulls(&dx2);
	pulls(&dy1);
	pulls(&dx1);
	if(xv) return;
	unsigned short vid=(unsigned short)(dvar.data.sint64 & USHRT_MAX);
	vmvar_t *x=rvm[thd]->vmv;
	while(x && x->vid!=vid) x=(vmvar_t *)x->next;
	if(x && (x->type==rSINT32)) {
		int t,r;
		unsigned long ecc, ec=1;
		for(t=0; t<MAX_DIMENSIONS; t++) if(x->dim[t]) ec*=x->dim[t];	// count the elements
		unsigned char *w;
		if(x->addr) {	// pointer to the data block
			w=(unsigned char *)((unsigned char *)(x->addr));
		} else {
			w=(unsigned char *)((unsigned char *)x+sizeof(vmvar_t));
		}
		if(!w) return;

		// process the coordinates
		int x1=(int)dx1.data.sint64;
		int x2=(int)dx2.data.sint64;
		int y1=(int)dy1.data.sint64;
		int y2=(int)dy2.data.sint64;
		if(x1>x2) { t=x1; x1=x2; x2=t; }
		if(y1>y2) { t=y1; y1=y2; y2=t; }
		if(x1>=resH || x2<0 || y1>=resV || y2<0) return;	// completely outside the screen

		// read
		signed long c;
		for(ecc=0, r=y1; r<=y2 && ecc<ec; r++) {
			for(t=x1; t<=x2 && ecc<ec; t++) {
				display_ReadRect(t,r,t,r,&c);
				*(w+3)=0;
				*(w+2)=(unsigned char)c>>16;	// R
				*(w+1)=(unsigned char)c>>8;		// G
				*w=(unsigned char)c;			// B
				w+=sizeof(signed long);
				ecc++;
			}
		}
	}
	else xv=-17;
}


void __gput(void) {
	rdata_t dx1,dy1,dx2,dy2,dvar;
	pullref(&dvar);	// referred variable
	pulls(&dy2);
	pulls(&dx2);
	pulls(&dy1);
	pulls(&dx1);
	if(xv) return;
	unsigned short vid=(unsigned short)(dvar.data.sint64 & USHRT_MAX);
	vmvar_t *x=rvm[thd]->vmv;
	while(x && x->vid!=vid) x=(vmvar_t *)x->next;
	if(x && (x->type==rSINT32)) {
		int t,r;
		unsigned long ecc, ec=1;
		for(t=0; t<MAX_DIMENSIONS; t++) if(x->dim[t]) ec*=x->dim[t];	// count the elements
		unsigned char *w;
		if(x->addr) {	// pointer to the data block
			w=(unsigned char *)((unsigned char *)(x->addr));
		} else {
			w=(unsigned char *)((unsigned char *)x+sizeof(vmvar_t));
		}
		if(!w) return;

		// process the coordinates
		int x1=(int)dx1.data.sint64;
		int x2=(int)dx2.data.sint64;
		int y1=(int)dy1.data.sint64;
		int y2=(int)dy2.data.sint64;
		if(x1>x2) { t=x1; x1=x2; x2=t; }
		if(y1>y2) { t=y1; y1=y2; y2=t; }
		if(x1>=resH || x2<0 || y1>=resV || y2<0) return;	// completely outside the screen

		// draw
		signed long c;
		for(ecc=0, r=y1; r<=y2 && ecc<ec; r++) {
			for(t=x1; t<=x2 && ecc<ec; t++) {
				c=(signed long)((unsigned long)*w+((unsigned long)*(w+1)<<8)+
					((unsigned long)*(w+2)<<16)+((unsigned long)*(w+3)<<24));
				w+=sizeof(signed long);
				if(c>=0) display_Pixel(t,r,c);
				ecc++;
			}
		}
	}
	else xv=-17;
}


void __cls(void) {
	unsigned char e=enxcon;
	enxcon=0;	// temporarily disable the external console while sending blank lines
	int t;
	for(t=0; t<250; t++) printf("\r\n");
	enxcon=e;
	display_Cls();
}


void __gpattr(void) {
	rdata_t d1,d2,d3;
	pullu(&d3);
	pulls(&d2);
	pulls(&d1);
	if(xv) return;
	fontScale=(unsigned char)(d3.data.sint64 & 255);
	fontBcol=(signed long)d2.data.sint64;
	fontFcol=(signed long)d1.data.sint64;
}


void __pixel(void) {
	rdata_t x,y,c;
	pulls(&c);	// colour
	pulls(&y);	// y position
	pulls(&x);	// x position
	if(xv) return;
	display_Pixel(x.data.sint64, y.data.sint64, (signed long)abs(c.data.sint64));
}


// needed for the custom stack for flood fill
typedef struct { int x; int y; } fill_seed_t;
unsigned char *fstack;
unsigned short fsx, FILL_STACK;


// pixel setter and getter
#define setPixel(x, y, c) display_Pixel(x,y,c)
long getPixel(int x, int y) {
	long c;
	display_ReadRect(x,y,x,y,&c);
	return c;
}


// push coordinate seed
void pushSeed(int x, int y) {
	if(fsx>=FILL_STACK) return;
	fill_seed_t *p=(fill_seed_t *)(fstack+(fsx++)*sizeof(fill_seed_t));
	memcpy(&(p->x),&x,sizeof(int));
	memcpy(&(p->y),&y,sizeof(int));
}


// pop coordinate seed
char popSeed(int *x, int *y) {
	if(fsx<=0) return 0;
	fill_seed_t *p=(fill_seed_t *)(fstack+(--fsx)*sizeof(fill_seed_t));
	memcpy(x,&(p->x),sizeof(int));
	memcpy(y,&(p->y),sizeof(int));
	return 1;
}


// main flood fill function
void flood_fill(int x, int y, long col) {
	long bcol;
	display_ReadRect(x,y,x,y,&bcol);	// get the seed colour
	if(bcol==col) return;
	setPixel(x,y,~bcol);
	if(getPixel(x,y)==bcol) return;		// the display doesn't support reading
	fsx=0;
	xfree((unsigned char **)&fstack);	// make sure fstack[] is empty
	FILL_STACK=USHRT_MAX;
	while(FILL_STACK) {
		xalloc((unsigned char **)&fstack,(FILL_STACK*sizeof(fill_seed_t)));
		if(fstack) break;	// if successful stop the loop now
		if(FILL_STACK>64) FILL_STACK-=64; else FILL_STACK--;
	}
	if(!fstack) return;
	pushSeed(x,y);
	while(popSeed(&x,&y)) {
		setPixel(x,y,col);
		if(x>0 && getPixel(x-1,y)==bcol) pushSeed(x-1,y);
		if(x<resH && getPixel(x+1,y)==bcol) pushSeed(x+1,y);
		if(y>0 && getPixel(x,y-1)==bcol) pushSeed(x,y-1);
		if(y<resV && getPixel(x,y+1)==bcol) pushSeed(x,y+1);
	}
	xfree((unsigned char **)&fstack);
}


void __fill(void) {
	rdata_t dx,dy,dc;
	pulls(&dc);	// colour
	pulls(&dy);	// start y
	pulls(&dx);	// start x
	if(xv || dc.data.sint64<0) return;
	flood_fill(dx.data.sint64, dy.data.sint64, dc.data.sint64);
}


void draw_line(int x1, int y1, int x2, int y2, signed long c) {
	int t;
	if(x1>x2) { t=x1; x1=x2; x2=t; t=y1; y1=y2; y2=t; }
    if(x1>=resH || x2<0 || (y1<0 && y2<0) || (y1>=resV && y2>=resV)) return;	// completely outside the screen
	char stf=(abs(y2-y1) > abs(x2-x1));
    if(stf) {
		{ t=x1; x1=y1; y1=t; }
		{ t=x2; x2=y2; y2=t; }
    }
    if(x1>x2) {
		{ t=x1; x1=x2; x2=t; }
		{ t=y1; y1=y2; y2=t; }
    }
    int dx=x2-x1;
    int dy=abs(y2-y1);
    int err=dx/2;
    int ystep=((y1 < y2)? 1 : -1);
    for(; x1<=x2; x1++) {
        if(stf) display_Pixel(y1, x1, c);
        else display_Pixel(x1, y1, c);
        err-=dy;
        if(err<0) {
            y1+=ystep;
            err+=dx;
        }
    }
}


void __line(void) {
	rdata_t x1,y1,x2,y2,c;
	pulls(&c);		// colour
	pulls(&y2);
	pulls(&x2);
	pulls(&y1);
	pulls(&x1);
	if(xv) return;
	if(x1.data.sint64!=x2.data.sint64 && y1.data.sint64!=y2.data.sint64) {	// any line
		draw_line((int)x1.data.sint64, (int)y1.data.sint64,
					(int)x2.data.sint64, (int)y2.data.sint64, abs(c.data.sint64));
	}
	else {	// horizontal or vertical line
		display_DrawRect((int)x1.data.sint64, (int)y1.data.sint64,
					(int)x2.data.sint64, (int)y2.data.sint64, abs(c.data.sint64));
	}
}


void __circle(void) {
	rdata_t dx,dy,dr,dc;
	pulls(&dc);		// colour
	pullu(&dr);		// radius
	pulls(&dy);
	pulls(&dx);
	if(xv) return;
	int x0=(int)dx.data.sint64;
	int y0=(int)dy.data.sint64;
	int r=(int)dr.data.sint64;
	if(r<=0) return;
	signed long col=dc.data.sint64;

	if(col>=0) {	// solid
		signed long x, y, rr=r*r;
		for(y=-r; y<=r; y++)
			for(x=-r; x<=r; x++)
				if((x*x+y*y)<=rr) display_Pixel(x0+x, y0+y, col);
	}

	else {	// outline
		col=-col;
		int f=1-r;
		int ddF_x=1;
		int ddF_y=-2*r;
		int x=0;
		int y=r;
		display_Pixel(x0  , y0+r, col);
		display_Pixel(x0  , y0-r, col);
		display_Pixel(x0+r, y0  , col);
		display_Pixel(x0-r, y0  , col);
		while(x<y) {
			if(f>=0) {
				y--;
				ddF_y+=2;
				f+=ddF_y;
			}
			x++;
			ddF_x+=2;
			f+=ddF_x;
			display_Pixel(x0 + x, y0 + y, col);
			display_Pixel(x0 - x, y0 + y, col);
			display_Pixel(x0 + x, y0 - y, col);
			display_Pixel(x0 - x, y0 - y, col);
			display_Pixel(x0 + y, y0 + x, col);
			display_Pixel(x0 - y, y0 + x, col);
			display_Pixel(x0 + y, y0 - x, col);
			display_Pixel(x0 - y, y0 - x, col);
		}
	}
}


void __ellipse(void) {
	rdata_t dx,dy,drx,dry,dt,dc;
	pulls(&dc);		// colour
	pullr(&dt);		// tilt angle
	pullu(&dry);	// y radius
	pullu(&drx);	// x radius
	pulls(&dy);
	pulls(&dx);
	if(xv) return;
	int x0=(int)dx.data.sint64;
	int y0=(int)dy.data.sint64;
	int rx=(int)drx.data.sint64;
	int ry=(int)dry.data.sint64;
	if(rx<=0 || ry<=0) return;
	double tang=(double)dt.data.real;
	signed long col=dc.data.sint64;
	double C=cos(tang);
	double S=sin(tang);
	long x,y,xp,yp;

	if(dc.data.sint64>=0) {	// solid
		for(y=-ry; y<=ry; y++) {
			for(x=-rx; x<=rx; x++) {
				double dx = (double)x / (double)rx;
				double dy = (double)y / (double)ry;
				double dd = (dx*dx+dy*dy);
				if(dd<=1.0) {
					xp =  C*x + S*y;
					yp = -S*x + C*y;
					display_Pixel(x0+xp, y0+yp, col);
				}
			}
		}
	}

	else {	// outline
		col=-col;
		double th=0.0, st=0.025;
		x = rx*cos(th);
		y = ry*sin(th);
		xp =  C*x + S*y;
		yp = -S*x + C*y;
		long xb=xp; long yb=yp;
		for(th=st; th<=2*PI; th+=st) {
			x = rx*cos(th);
			y = ry*sin(th);
			xp =  C*x + S*y;
			yp = -S*x + C*y;
			draw_line(x0+xb, y0+yb, x0+xp, y0+yp, col);
			xb=xp; yb=yp;
		}
	}
}


void __sector(void) {
	rdata_t dx,dy,drx,dry,dt,da1,da2,dc;
	pulls(&dc);		// colour
	pullr(&da2);	// end angle
	pullr(&da1);	// start angle
	pullr(&dt);		// tilt angle
	pullu(&dry);	// y radius
	pullu(&drx);	// x radius
	pulls(&dy);
	pulls(&dx);
	if(xv) return;
	int x0=(int)dx.data.sint64;
	int y0=(int)dy.data.sint64;
	int rx=(int)drx.data.sint64;
	int ry=(int)dry.data.sint64;
	double tang=(double)dt.data.real;
	double ang1=(double)da1.data.real;
	double ang2=(double)da2.data.real;
	if(rx<=0 || ry<=0 || ang1==ang2) return;
	signed long col=dc.data.sint64;
	double C=cos(tang);
	double S=sin(tang);
	if(ang1>ang2) {
		double v=ang1;
		ang1=ang2;
		ang2=v;
	}
	long x,y,xp,yp;
	double th, st=0.015;
	x = rx*cos(ang1);
	y = ry*sin(ang1);
	xp =  C*x + S*y;
	yp = -S*x + C*y;
	draw_line(x0, y0, x0+xp, y0+yp, abs(col));
	long xb=xp; long yb=yp;
	for(th=ang1; th<=ang2; th+=st) {
		x = rx*cos(th);
		y = ry*sin(th);
		xp =  C*x + S*y;
		yp = -S*x + C*y;
		draw_line(x0+xb, y0+yb, x0+xp, y0+yp, abs(col));
		if(col>=0) draw_line(x0, y0, x0+xp, y0+yp, col);	// make it solid
		xb=xp; yb=yp;
	}
	draw_line(x0, y0, x0+xp, y0+yp, abs(col));
}


void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, long c) {
	if(c>=0) {	// solid
		int t,y,l,a,b;
		if(y1>y2) {
			{ t=x1; x1=x2; x2=t; }
			{ t=y1; y1=y2; y2=t; }
		}
		if(y2>y3) {
			{ t=x2; x2=x3; x3=t; }
			{ t=y2; y2=y3; y3=t; }
		}
		if(y1>y2) {
			{ t=x1; x1=x2; x2=t; }
			{ t=y1; y1=y2; y2=t; }
		}
		if(y1==y2 && y2==y3) {	// handle the odd all-on-same-line case
			a=b=x1;
			if(x2<a) a=x2;
			else if(x2>b) b=x2;
			if(x3<a) a=x3;
			else if(x3>b) b=x3;
			display_DrawRect(a, y1, b-a+1, y1, c);
			return;
		}
		int dx12=x2-x1;
		int dy12=y2-y1;
		int dx13=x3-x1;
		int dy13=y3-y1;
		int dx23=x3-x2;
		int dy23=y3-y2;
		long sa=0;
		long sb=0;
		if(y2==y3) l=y2;
		else l=y2-1;
		for(y=y1; y<=l; y++) {
			a=x1+sa/dy12;
			b=x1+sb/dy13;
			sa+=dx12;
			sb+=dx13;
			// a=x1+((x2-x1)*(y-y1))/(y2-y1);
			// b=x1+((x3-x1)*(y-y1))/(y3-y1);
			if(a > b) { t=a; a=b; b=t; }
			display_DrawRect(a, y, b, y, c);
		}
		sa=dx23*(y-y2);
		sb=dx13*(y-y1);
		for(; y<=y3; y++) {
			a=x2+sa/dy23;
			b=x1+sb/dy13;
			sa+=dx23;
			sb+=dx13;
			// a=x2+((x3-x2)*(y-y2))/(y3-y2);
			// b=x1+((x3-x1)*(y-y1))/(y3-y1);
			if(a > b) { t=a; a=b; b=t; }
			display_DrawRect(a, y, b, y, c);
		}
	}

	else {		// outline
		c=-c;
		draw_line(x1, y1, x2, y2, c);
		draw_line(x2, y2, x3, y3, c);
		draw_line(x1, y1, x3, y3, c);
	}
}


void __triangle(void) {
	rdata_t dx1,dy1,dx2,dy2,dx3,dy3,dc;
	pulls(&dc);		// colour
	pulls(&dy3);
	pulls(&dx3);
	pulls(&dy2);
	pulls(&dx2);
	pulls(&dy1);
	pulls(&dx1);
	if(xv) return;
	int x1=dx1.data.sint64;
	int y1=dy1.data.sint64;
	int x2=dx2.data.sint64;
	int y2=dy2.data.sint64;
	int x3=dx3.data.sint64;
	int y3=dy3.data.sint64;
	draw_triangle(x1,y1,x2,y2,x3,y3,dc.data.sint64);
}


void __rect(void) {
	rdata_t x1,y1,x2,y2,dc;
	pulls(&dc);		// colour
	pulls(&y2);
	pulls(&x2);
	pulls(&y1);
	pulls(&x1);
	if(xv) return;
	signed long c=dc.data.sint64;

	if(c>=0) {	// solid
		display_DrawRect((int)x1.data.sint64, (int)y1.data.sint64, (int)x2.data.sint64, (int)y2.data.sint64, c);
	}

	else {		// outline
		c=-c;
		draw_line((int)x1.data.sint64, (int)y1.data.sint64, (int)x2.data.sint64, (int)y1.data.sint64, c);
		draw_line((int)x2.data.sint64, (int)y1.data.sint64, (int)x2.data.sint64, (int)y2.data.sint64, c);
		draw_line((int)x1.data.sint64, (int)y1.data.sint64, (int)x1.data.sint64, (int)y2.data.sint64, c);
		draw_line((int)x1.data.sint64, (int)y2.data.sint64, (int)x2.data.sint64, (int)y2.data.sint64, c);
	}
}


void __gprint(void) {
	rdata_t d,dx,dy;
	if(rvm[thd]->dsp<2) {
		xv=-24;
		return;
	}
	pullfifo(&dx);
	pullfifo(&dy);
	if(xv || dx.type!=rSINT64 || dy.type!=rSINT64) return;
	posX=(int)dx.data.sint64;
	posY=(int)dy.data.sint64;
	while(rvm[thd]->dsp) {
		pullfifo(&d);
		char z[25];
		memset(z,0,sizeof(z));
		if(d.type==rSINT64 || d.type==rSINT32 || d.type==rSINT16 || d.type==rSINT8) {
			sprintf(z,"%lli",d.data.sint64);
			d.data.text=z;
		}
		else if(d.type==rREAL) {
			sprintf(z,"%G",(double)d.data.real);
			d.data.text=z;
		}
		if(d.data.text) {
			no_crtl_chars=1;
			unsigned char *zz=(unsigned char *)d.data.text;
			while(*zz) gPutChr((int)*(zz++));
			no_crtl_chars=0;
		}
	}
	__clear();
}


// standard ASCII font 5x8 pixels
const font_t sysFont0508 = { {
	1,		// code of the first character in the font
	255,	// 255 characters in this font (ASCII codes 0x01 ... 0xFF)
	5,		// fixed with 5 columns in every character
	8,		// 8 rows in every character
	1,		// blank columns on the left side of every character
	0,		// blank columns on the right side of every character
	1,		// blank rows on the top side of every character
	1,		// blank rows on the bottom side of every character
	"System Font 5x8"
	}, {
	   0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
	   0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
	   0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
	   0x18, 0x3C, 0x7E, 0x3C, 0x18,
	   0x1C, 0x57, 0x7D, 0x57, 0x1C,
	   0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
	   0x00, 0x18, 0x3C, 0x18, 0x00,
	   0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
	   0x00, 0x18, 0x24, 0x18, 0x00,
	   0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
	   0x30, 0x48, 0x3A, 0x06, 0x0E,
	   0x26, 0x29, 0x79, 0x29, 0x26,
	   0x40, 0x7F, 0x05, 0x05, 0x07,
	   0x40, 0x7F, 0x05, 0x25, 0x3F,
	   0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
	   0x7F, 0x3E, 0x1C, 0x1C, 0x08,
	   0x08, 0x1C, 0x1C, 0x3E, 0x7F,
	   0x14, 0x22, 0x7F, 0x22, 0x14,
	   0x5F, 0x5F, 0x00, 0x5F, 0x5F,
	   0x06, 0x09, 0x7F, 0x01, 0x7F,
	   0x00, 0x66, 0x89, 0x95, 0x6A,
	   0x60, 0x60, 0x60, 0x60, 0x60,
	   0x94, 0xA2, 0xFF, 0xA2, 0x94,
	   0x08, 0x04, 0x7E, 0x04, 0x08,
	   0x10, 0x20, 0x7E, 0x20, 0x10,
	   0x08, 0x08, 0x2A, 0x1C, 0x08,
	   0x08, 0x1C, 0x2A, 0x08, 0x08,
	   0x1E, 0x10, 0x10, 0x10, 0x10,
	   0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
	   0x30, 0x38, 0x3E, 0x38, 0x30,
	   0x06, 0x0E, 0x3E, 0x0E, 0x06,
	   0x00, 0x00, 0x00, 0x00, 0x00,
	   0x00, 0x00, 0x5F, 0x00, 0x00,
	   0x00, 0x07, 0x00, 0x07, 0x00,
	   0x14, 0x7F, 0x14, 0x7F, 0x14,
	   0x24, 0x2A, 0x7F, 0x2A, 0x12,
	   0x23, 0x13, 0x08, 0x64, 0x62,
	   0x36, 0x49, 0x56, 0x20, 0x50,
	   0x00, 0x08, 0x07, 0x03, 0x00,
	   0x00, 0x1C, 0x22, 0x41, 0x00,
	   0x00, 0x41, 0x22, 0x1C, 0x00,
	   0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
	   0x08, 0x08, 0x3E, 0x08, 0x08,
	   0x00, 0x80, 0x70, 0x30, 0x00,
	   0x08, 0x08, 0x08, 0x08, 0x08,
	   0x00, 0x00, 0x60, 0x60, 0x00,
	   0x20, 0x10, 0x08, 0x04, 0x02,
	   0x3E, 0x51, 0x49, 0x45, 0x3E,
	   0x00, 0x42, 0x7F, 0x40, 0x00,
	   0x72, 0x49, 0x49, 0x49, 0x46,
	   0x21, 0x41, 0x49, 0x4D, 0x33,
	   0x18, 0x14, 0x12, 0x7F, 0x10,
	   0x27, 0x45, 0x45, 0x45, 0x39,
	   0x3C, 0x4A, 0x49, 0x49, 0x31,
	   0x41, 0x21, 0x11, 0x09, 0x07,
	   0x36, 0x49, 0x49, 0x49, 0x36,
	   0x46, 0x49, 0x49, 0x29, 0x1E,
	   0x00, 0x00, 0x14, 0x00, 0x00,
	   0x00, 0x40, 0x34, 0x00, 0x00,
	   0x00, 0x08, 0x14, 0x22, 0x41,
	   0x14, 0x14, 0x14, 0x14, 0x14,
	   0x00, 0x41, 0x22, 0x14, 0x08,
	   0x02, 0x01, 0x59, 0x09, 0x06,
	   0x3E, 0x41, 0x5D, 0x59, 0x4E,
	   0x7C, 0x12, 0x11, 0x12, 0x7C,
	   0x7F, 0x49, 0x49, 0x49, 0x36,
	   0x3E, 0x41, 0x41, 0x41, 0x22,
	   0x7F, 0x41, 0x41, 0x41, 0x3E,
	   0x7F, 0x49, 0x49, 0x49, 0x41,
	   0x7F, 0x09, 0x09, 0x09, 0x01,
	   0x3E, 0x41, 0x41, 0x51, 0x73,
	   0x7F, 0x08, 0x08, 0x08, 0x7F,
	   0x00, 0x41, 0x7F, 0x41, 0x00,
	   0x20, 0x40, 0x41, 0x3F, 0x01,
	   0x7F, 0x08, 0x14, 0x22, 0x41,
	   0x7F, 0x40, 0x40, 0x40, 0x40,
	   0x7F, 0x02, 0x1C, 0x02, 0x7F,
	   0x7F, 0x04, 0x08, 0x10, 0x7F,
	   0x3E, 0x41, 0x41, 0x41, 0x3E,
	   0x7F, 0x09, 0x09, 0x09, 0x06,
	   0x3E, 0x41, 0x51, 0x21, 0x5E,
	   0x7F, 0x09, 0x19, 0x29, 0x46,
	   0x26, 0x49, 0x49, 0x49, 0x32,
	   0x03, 0x01, 0x7F, 0x01, 0x03,
	   0x3F, 0x40, 0x40, 0x40, 0x3F,
	   0x1F, 0x20, 0x40, 0x20, 0x1F,
	   0x3F, 0x40, 0x38, 0x40, 0x3F,
	   0x63, 0x14, 0x08, 0x14, 0x63,
	   0x03, 0x04, 0x78, 0x04, 0x03,
	   0x61, 0x59, 0x49, 0x4D, 0x43,
	   0x00, 0x7F, 0x41, 0x41, 0x41,
	   0x02, 0x04, 0x08, 0x10, 0x20,
	   0x00, 0x41, 0x41, 0x41, 0x7F,
	   0x04, 0x02, 0x01, 0x02, 0x04,
	   0x40, 0x40, 0x40, 0x40, 0x40,
	   0x00, 0x03, 0x07, 0x08, 0x00,
	   0x20, 0x54, 0x54, 0x78, 0x40,
	   0x7F, 0x28, 0x44, 0x44, 0x38,
	   0x38, 0x44, 0x44, 0x44, 0x28,
	   0x38, 0x44, 0x44, 0x28, 0x7F,
	   0x38, 0x54, 0x54, 0x54, 0x18,
	   0x00, 0x08, 0x7E, 0x09, 0x02,
	   0x18, 0xA4, 0xA4, 0x9C, 0x78,
	   0x7F, 0x08, 0x04, 0x04, 0x78,
	   0x00, 0x44, 0x7D, 0x40, 0x00,
	   0x20, 0x40, 0x40, 0x3D, 0x00,
	   0x7F, 0x10, 0x28, 0x44, 0x00,
	   0x00, 0x41, 0x7F, 0x40, 0x00,
	   0x7C, 0x04, 0x78, 0x04, 0x78,
	   0x7C, 0x08, 0x04, 0x04, 0x78,
	   0x38, 0x44, 0x44, 0x44, 0x38,
	   0xFC, 0x18, 0x24, 0x24, 0x18,
	   0x18, 0x24, 0x24, 0x18, 0xFC,
	   0x7C, 0x08, 0x04, 0x04, 0x08,
	   0x48, 0x54, 0x54, 0x54, 0x24,
	   0x04, 0x04, 0x3F, 0x44, 0x24,
	   0x3C, 0x40, 0x40, 0x20, 0x7C,
	   0x1C, 0x20, 0x40, 0x20, 0x1C,
	   0x3C, 0x40, 0x30, 0x40, 0x3C,
	   0x44, 0x28, 0x10, 0x28, 0x44,
	   0x4C, 0x90, 0x90, 0x90, 0x7C,
	   0x44, 0x64, 0x54, 0x4C, 0x44,
	   0x00, 0x08, 0x36, 0x41, 0x00,
	   0x00, 0x00, 0x77, 0x00, 0x00,
	   0x00, 0x41, 0x36, 0x08, 0x00,
	   0x02, 0x01, 0x02, 0x04, 0x02,
	   0x3C, 0x26, 0x23, 0x26, 0x3C,
	   0x1E, 0xA1, 0xA1, 0x61, 0x12,
	   0x3A, 0x40, 0x40, 0x20, 0x7A,
	   0x38, 0x54, 0x54, 0x55, 0x59,
	   0x21, 0x55, 0x55, 0x79, 0x41,
	   0x22, 0x54, 0x54, 0x78, 0x42, // a-umlaut
	   0x21, 0x55, 0x54, 0x78, 0x40,
	   0x20, 0x54, 0x55, 0x79, 0x40,
	   0x0C, 0x1E, 0x52, 0x72, 0x12,
	   0x39, 0x55, 0x55, 0x55, 0x59,
	   0x39, 0x54, 0x54, 0x54, 0x59,
	   0x39, 0x55, 0x54, 0x54, 0x58,
	   0x00, 0x00, 0x45, 0x7C, 0x41,
	   0x00, 0x02, 0x45, 0x7D, 0x42,
	   0x00, 0x01, 0x45, 0x7C, 0x40,
	   0x7D, 0x12, 0x11, 0x12, 0x7D, // A-umlaut
	   0xF0, 0x28, 0x25, 0x28, 0xF0,
	   0x7C, 0x54, 0x55, 0x45, 0x00,
	   0x20, 0x54, 0x54, 0x7C, 0x54,
	   0x7C, 0x0A, 0x09, 0x7F, 0x49,
	   0x32, 0x49, 0x49, 0x49, 0x32,
	   0x3A, 0x44, 0x44, 0x44, 0x3A, // o-umlaut
	   0x32, 0x4A, 0x48, 0x48, 0x30,
	   0x3A, 0x41, 0x41, 0x21, 0x7A,
	   0x3A, 0x42, 0x40, 0x20, 0x78,
	   0x00, 0x9D, 0xA0, 0xA0, 0x7D,
	   0x3D, 0x42, 0x42, 0x42, 0x3D, // O-umlaut
	   0x3D, 0x40, 0x40, 0x40, 0x3D,
	   0x3C, 0x24, 0xFF, 0x24, 0x24,
	   0x48, 0x7E, 0x49, 0x43, 0x66,
	   0x2B, 0x2F, 0xFC, 0x2F, 0x2B,
	   0xFF, 0x09, 0x29, 0xF6, 0x20,
	   0xC0, 0x88, 0x7E, 0x09, 0x03,
	   0x20, 0x54, 0x54, 0x79, 0x41,
	   0x00, 0x00, 0x44, 0x7D, 0x41,
	   0x30, 0x48, 0x48, 0x4A, 0x32,
	   0x38, 0x40, 0x40, 0x22, 0x7A,
	   0x00, 0x7A, 0x0A, 0x0A, 0x72,
	   0x7D, 0x0D, 0x19, 0x31, 0x7D,
	   0x26, 0x29, 0x29, 0x2F, 0x28,
	   0x26, 0x29, 0x29, 0x29, 0x26,
	   0x30, 0x48, 0x4D, 0x40, 0x20,
	   0x38, 0x08, 0x08, 0x08, 0x08,
	   0x08, 0x08, 0x08, 0x08, 0x38,
	   0x2F, 0x10, 0xC8, 0xAC, 0xBA,
	   0x2F, 0x10, 0x28, 0x34, 0xFA,
	   0x00, 0x00, 0x7B, 0x00, 0x00,
	   0x08, 0x14, 0x2A, 0x14, 0x22,
	   0x22, 0x14, 0x2A, 0x14, 0x08,
	   0x55, 0x00, 0x55, 0x00, 0x55, // 25% block
	   0xAA, 0x55, 0xAA, 0x55, 0xAA, // 50% block
	   0xFF, 0x55, 0xFF, 0x55, 0xFF, // 75% block
	   0x00, 0x00, 0x00, 0xFF, 0x00,
	   0x10, 0x10, 0x10, 0xFF, 0x00,
	   0x14, 0x14, 0x14, 0xFF, 0x00,
	   0x10, 0x10, 0xFF, 0x00, 0xFF,
	   0x10, 0x10, 0xF0, 0x10, 0xF0,
	   0x14, 0x14, 0x14, 0xFC, 0x00,
	   0x14, 0x14, 0xF7, 0x00, 0xFF,
	   0x00, 0x00, 0xFF, 0x00, 0xFF,
	   0x14, 0x14, 0xF4, 0x04, 0xFC,
	   0x14, 0x14, 0x17, 0x10, 0x1F,
	   0x10, 0x10, 0x1F, 0x10, 0x1F,
	   0x14, 0x14, 0x14, 0x1F, 0x00,
	   0x10, 0x10, 0x10, 0xF0, 0x00,
	   0x00, 0x00, 0x00, 0x1F, 0x10,
	   0x10, 0x10, 0x10, 0x1F, 0x10,
	   0x10, 0x10, 0x10, 0xF0, 0x10,
	   0x00, 0x00, 0x00, 0xFF, 0x10,
	   0x10, 0x10, 0x10, 0x10, 0x10,
	   0x10, 0x10, 0x10, 0xFF, 0x10,
	   0x00, 0x00, 0x00, 0xFF, 0x14,
	   0x00, 0x00, 0xFF, 0x00, 0xFF,
	   0x00, 0x00, 0x1F, 0x10, 0x17,
	   0x00, 0x00, 0xFC, 0x04, 0xF4,
	   0x14, 0x14, 0x17, 0x10, 0x17,
	   0x14, 0x14, 0xF4, 0x04, 0xF4,
	   0x00, 0x00, 0xFF, 0x00, 0xF7,
	   0x14, 0x14, 0x14, 0x14, 0x14,
	   0x14, 0x14, 0xF7, 0x00, 0xF7,
	   0x14, 0x14, 0x14, 0x17, 0x14,
	   0x10, 0x10, 0x1F, 0x10, 0x1F,
	   0x14, 0x14, 0x14, 0xF4, 0x14,
	   0x10, 0x10, 0xF0, 0x10, 0xF0,
	   0x00, 0x00, 0x1F, 0x10, 0x1F,
	   0x00, 0x00, 0x00, 0x1F, 0x14,
	   0x00, 0x00, 0x00, 0xFC, 0x14,
	   0x00, 0x00, 0xF0, 0x10, 0xF0,
	   0x10, 0x10, 0xFF, 0x10, 0xFF,
	   0x14, 0x14, 0x14, 0xFF, 0x14,
	   0x10, 0x10, 0x10, 0x1F, 0x00,
	   0x00, 0x00, 0x00, 0xF0, 0x10,
	   0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	   0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	   0xFF, 0xFF, 0xFF, 0x00, 0x00,
	   0x00, 0x00, 0x00, 0xFF, 0xFF,
	   0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	   0x38, 0x44, 0x44, 0x38, 0x44,
	   0xFC, 0x4A, 0x4A, 0x4A, 0x34, // sharp-s or beta
	   0x7E, 0x02, 0x02, 0x06, 0x06,
	   0x02, 0x7E, 0x02, 0x7E, 0x02,
	   0x63, 0x55, 0x49, 0x41, 0x63,
	   0x38, 0x44, 0x44, 0x3C, 0x04,
	   0x40, 0x7E, 0x20, 0x1E, 0x20,
	   0x06, 0x02, 0x7E, 0x02, 0x02,
	   0x99, 0xA5, 0xE7, 0xA5, 0x99,
	   0x1C, 0x2A, 0x49, 0x2A, 0x1C,
	   0x4C, 0x72, 0x01, 0x72, 0x4C,
	   0x30, 0x4A, 0x4D, 0x4D, 0x30,
	   0x30, 0x48, 0x78, 0x48, 0x30,
	   0xBC, 0x62, 0x5A, 0x46, 0x3D,
	   0x3E, 0x49, 0x49, 0x49, 0x00,
	   0x7E, 0x01, 0x01, 0x01, 0x7E,
	   0x2A, 0x2A, 0x2A, 0x2A, 0x2A,
	   0x44, 0x44, 0x5F, 0x44, 0x44,
	   0x40, 0x51, 0x4A, 0x44, 0x40,
	   0x40, 0x44, 0x4A, 0x51, 0x40,
	   0x00, 0x00, 0xFF, 0x01, 0x03,
	   0xE0, 0x80, 0xFF, 0x00, 0x00,
	   0x08, 0x08, 0x6B, 0x6B, 0x08,
	   0x36, 0x12, 0x36, 0x24, 0x36,
	   0x06, 0x0F, 0x09, 0x0F, 0x06,
	   0x00, 0x00, 0x18, 0x18, 0x00,
	   0x00, 0x00, 0x10, 0x10, 0x00,
	   0x30, 0x40, 0xFF, 0x01, 0x01,
	   0x00, 0x1F, 0x01, 0x01, 0x1E,
	   0x00, 0x19, 0x1D, 0x17, 0x12,
	   0x00, 0x3C, 0x3C, 0x3C, 0x3C,
	   0x00, 0x00, 0x00, 0x00, 0x00  // #255 NBSP
	}
};


void __font(void) {
	rdata_t d;
	pullref(&d);
	if(xv) return;
	unsigned short vid=(unsigned short)(d.data.sint64 & USHRT_MAX);
	vmvar_t *x=rvm[thd]->vmv;
	while(x && x->vid!=vid) x=(vmvar_t *)x->next;
	if(x && (x->type==rSINT8)) {
		unsigned char *w;
		if(x->addr) {	// pointer to the data block
			w=(unsigned char *)((unsigned char *)(x->addr));
		} else {
			w=(unsigned char *)((unsigned char *)x+sizeof(vmvar_t));
		}
		if(!w) return;
		font=(font_t *)w;
	}
	else xv=-17;
}


void __shape(void) {
	rdata_t dx,dy,d;
	pullt(&d);	// definitions
	pulls(&dy);	// start y
	pulls(&dx);	// start x
	if(xv || !d.data.text || strlen(d.data.text)==0) return;
	char sx, sy, *s=d.data.text;
	signed long vx, vy, col=fontFcol;
	signed int x=dx.data.sint64;
	signed int y=dy.data.sint64;
	signed int xp=x, yp=y;
	while(*s) {

		// skip spaces in the definition string
		if(*s==' ') {
			s++;
			continue;
		}

		// set drawing colour
		else if(*s=='C' || *s=='c') {
			while(*(++s)==' ');
			col=0;
			char hexf=0;
			while((*s>='0' && *s<='9') || *s=='X' || *s=='x' ||
					(hexf && ((*s>='A' && *s<='F') || (*s>='a' && *s<='f')))) {
				if(*s=='X' || *s=='x') {
					if(!hexf) hexf=1;
					else break;
				}
				else if((*s>='A' && *s<='F') || (*s>='a' && *s<='f')) {
					if(*s>='a' && *s<='f') col=(col<<4)+(10+(*s-'a'));
					else col=(col<<4)+(10+(*s-'A'));
				}
				else if(*s>='0' && *s<='9') {
					if(hexf) col=(col<<4)+(*s-'0');
					else col=(10*col)+(*s-'0');
				}
				else break;
				s++;
			}
			continue;
		}

		// flood fill an area starting with seed from the current pixel
		else if(*s=='F' || *s=='f') {
			s++;
			flood_fill(x,y,col);
			continue;
		}

		// set no draw
		else if(*s=='N' || *s=='n') {
			s++;
			col=-1;
			continue;
		}

		else if(*s=='M' || *s=='m') {
			sx=sy=1;
			while(*(++s)==' ');
			if(*s=='-') {
				sx=-1;
				s++;
			}
			vx=getnum(&s);
			while(*s==' ') s++;
			if(*s!=',') continue;
			while(*(++s)==' ');
			if(*s=='-') {
				sy=-1;
				s++;
			}
			vy=getnum(&s);
			if(vx<0) vx=0;
			x+=(vx*sx);
			if(vy<0) vy=0;
			y+=(vy*sy);
		}

		else break;

		if(col>=0) draw_line(x,y,xp,yp,col);
		xp=x, yp=y;
	}
}


// MAIN TOKEN TABLE ===========================================================

const rtoken_t tokens[256] = {
#define IN_TOKENS

// NOTE: {...} refer to values from stack, <...> refer to the program code
// NOTE: do not change any functions outside of the platform-specific code area
//          codes 0xA0 ... 0xEF, in order to maintain compatibility among all platforms

//			name		level inc params func
/* 00 */	{".nop",		0,	0,	0,	__nop},		// no operation
/* 01 */	{".unit",		0,	0,	0,	_invalid_},	// NOT AVAILABLE. DO NOT USE!
/* 02 */	{".clear",		98,	0,	0,	__clear},	// clear the stack
/* 03 */	{".goto",		0,	4,	0,	__goto},	// unconditional jump to specified address
													// <token> <address>
/* 04 */	{".call",		0,	4,	0,	__call},	// call function at specified address
													// <token> <address>
/* 05 */	{".exit",		99,	0,	0,	__exit},	// end of function or end of the main code block
/* 06 */	{".pproc",		0,	4,	0,	__callnt},	// open new thread (parallel process) and call function at specified address
													// <token> <address>
/* 07 */	{".ppterm",		0,	0,	0,	__ppterm},	// terminate a thread (parallel process)
													// <token> <entry address>
/* 08 */	{".func",		0,	4,	0,	__goto},	// <token> <jump address>
													// mark begin of function block definition
													// the actual function code starts after the following 4-byte jump later
													// executes as .goto
/* 09 */	{".endfunc",	0,	0,	0,	__exit},	// end of function block definition
													// acts like ".exit", but unlike it must be present only once at
													// the very end of the function block
/* 0A */	{".ifnot",		0,	4,	0,	__ifnot},	// conditional branch; jumps to the address if {value} is 0
													// {value} <token> <address>
/* 0B */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 0C */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 0D */	{".drop",		0,	0,	0,	__drop},	// remove the top stack element
/* 0E */	{".dup",		0,	0,	0,	__dup},		// duplicate the top stack element
/* 0F */	{".reset",		0,	0,	0,	__reset},	// marks the start of execution; initialises VM
													// <token> <4b> <b8> <44>

/* 10 */	{".any",		0,	sizeof(rdata_t),			0,	_invalid_},	// not a valid data type but only used in definitions
/* 11 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 12 */	{".sint8",		0,	sizeof(signed char),		0,	__sint8},	// <token> <8 bits, signed constant>
/* 13 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 14 */	{".sint16",		0,	sizeof(signed short),		0,	__sint16},	// <token> <16 bits, signed constant, LSB first>
/* 15 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 16 */	{".sint32",		0,	sizeof(signed long),		0,	__sint32},	// <token> <32 bits, signed constant, LSB first>
/* 17 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 18 */	{".sint64",		0,	sizeof(signed long long),	0,	__sint64},	// <token> <64 bits, signed constant, LSB first>
/* 19 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 1A */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 1B */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 1C */	{".real",		0,	sizeof(double),				0,	__creal},	// <token> <64 bits, floating point, LSB first>
/* 1D */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 1E */	{".text",		0,	-1, 0,	__text},							// <token> <ASCII sequence> <00>
/* 1F */	{".fptr",		0,	sizeof(unsigned long),		0,	__exec},	// <token> <32 bits, executable address>

/* 20 */	{".data",		0,	-1,	0,	__data},	// data constants
													// single data type format:
													// <token> <data_type> <vid> <32-bit count> [<constant> ...]
													// mixed data type format:
													// <token> <data_type_any> <vid> <32-bit count> [<token> <constant> ...]
/* 21 */	{".comment",	0,	-1,	0,	__comment}, // text commentaries included in code - always ignored during execution
                                                    // <token> <ASCIIZ string>
/* 22 */	{".vafix",		0,	2,	0,	__vafix},	// set variable to point at fixed absolute location
													// NOTE: does not create data container at the new address
													// {address} <token> <vid>
/* 23 */	{".maxlen",		0,	2,	0,	__varmaxl},	// set maximum length for text variables
													// {length} <token> <vid>
/* 24 */	{".index",		0,	2,	0,	__varidx},	// set the index pointer for array variables
													// the number of {index} parameters must be exactly as many as the dimensions
													// {index [index ...]} <token> <vid>
													// <token> <vid>
/* 25 */	{".vardim",		0,	3,	0,	__vardim},	// add dimensions to variable
													// will take from the stack numbers in count of the specified parameter
													// {dimension [dimension ...]} <token> <vid> <8-bit dimensions count>
/* 26 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 27 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 28 */	{".get",		0,	2,	0,	__get},		// get variable or constant
													// <token> <vid>
/* 29 */	{".set",		0,	2,	0,	__set},		// assign the result of expression/constant to a variable
													// {value} <token> <vid>
/* 2A */	{".var",		0,	-1,	0,	__var},		// variable declaration
													// <token> <data_type> [<vid> ...] <0000>
/* 2B */	{".varin",		0,	-1,	0,	__varin},	// variable declaration for input parameters
													// <token> <data_type> [<vid> ...] <0000>
/* 2C */	{".varout",		0,	-1,	0,	__varout},	// variable declaration for output containers
													// <token> <data_type> [<vid> ...] <0000>
/* 2D */	{".varref",		0,	-1,	0,	__varref},	// declaration for 'refer' type variables
													// <token> <data_type> [<vid> ...] <0000>
/* 2E */	{".refer",		0,	4,	0,	__sint32},	// reference to variable
													// <token> <32-bit 0xfeed0000+vid>
													// doesn't have a dedicated handling function
/* 2F */	{".reffn",		0,	4,	0,	__reffn},	// reference to function
													// <token> <32-bit function entry address>

/* 30 */	{"==",			2,	0,	0,	__equal},	// check for equal
/* 31 */	{"<>",			2,	0,	0,	__nequal},	// check for not equal
/* 32 */	{"<",			2,	0,	0,	__smaller},	// check for smaller
/* 33 */	{"<=",			2,	0,	0,	__smequal},	// check for smaller or equal
/* 34 */	{">=",			2,	0,	0,	__grequal},	// check for greater or equal
/* 35 */	{">",			2,	0,	0,	__greater},	// check for greater
/* 36 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 37 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 38 */	{"CRLF",		11,	0,	0,	__crlf},    // returns text with ASCII codes 0x0d and 0x0a
/* 39 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 3A */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 3B */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 3C */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 3D */	{"conch",		12,	0,	0,	__conch},	// conch
/* 3E */	{"conrd",		12,	0,	0,	__conrd},	// conrd
/* 3F */	{"print",		12,	0,	-1,	__print},	// print [param, param, param...]

/* 40 */	{"~",			7,	0,	0,	__neg},		// bit negation/sign inversion
/* 41 */	{"not",			7,	0,	0,	__not},		// logical NOT
/* 42 */	{"and",			1,	0,	0,	__and},		// logical AND
/* 43 */	{"or",			1,	0,	0,	__or},		// logical OR
/* 44 */	{"xor",			1,	0,	0,	__xor},		// logical Exclusive OR
/* 45 */	{"<<",			5,	0,	0,	__shiftl},	// bit shift left
/* 46 */	{">>",			5,	0,	0,	__shiftr},	// bit shift right
/* 47 */	{"^",			6,	0,	0,	__power},	// exponentiation
/* 48 */	{"+",			3,	0,	0,	__add},		// addition
/* 49 */	{".++",			8,	0,	0,	__inc},		// increment
/* 4A */	{"-",			3,	0,	0,	__sub},		// subtraction
/* 4B */	{".--",			8,	0,	0,	__dec},		// decrement
/* 4C */	{"*",			4,	0,	0,	__mul},		// multiplication
/* 4D */	{"/",			4,	0,	0,	__div},		// division
/* 4E */	{"\\",			4,	0,	0,	__idiv},	// integer division/rounding (single backslash)
/* 4F */	{"\\\\",		4,	0,	0,	__mod},		// modulo division/fraction isolation (double backslash)

/* 50 */	{"sin",			10,	0,	1,	__sin},		// sine
/* 51 */	{"asin",		10,	0,	1,	__arcsin},	// arcsine
/* 52 */	{"cos",			10,	0,	1,	__cos},		// cosine
/* 53 */	{"acos",		10,	0,	1,	__arccos},	// arccosine
/* 54 */	{"tan",			10,	0,	1,	__tan},		// tangent
/* 55 */	{"atan",		10,	0,	1,	__arctan},	// arctangent
/* 56 */	{"hsin",		10,	0,	1,	__sinh},	// hyperbolic sine
/* 57 */	{"htan",		10,	0,	1,	__tanh},	// hyperbolic tangent
/* 58 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 59 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 5A */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 5B */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 5C */	{"bit",			10,	0,	1,	__bit},		// return an integer value with the parameter bit raised
/* 5D */	{"trim",		10,	0,	1,	__trim},	// integer value
/* 5E */	{"abs",			10,	0,	1,	__abs},		// absolute value
/* 5F */	{"sign",		10,	0,	1,	__sign},	// return -1 for negative argument, 0 for 0, 1 for positive argument

/* 60 */	{"deg",			10,	0,	1,	__deg},		// convert from radians to degrees
/* 61 */	{"rad",			10,	0,	1,	__rad},		// convert from degrees to radians
/* 62 */	{"log",			10,	0,	1,	__log},		// decimal logarithm
/* 63 */	{"ln",			10,	0,	1,	__ln},		// natural logarithm
/* 64 */	{"exp",			10,	0,	1,	__exp},		// natural exponent e^X
/* 65 */	{"E",			11,	0,	0,	__E},		// constant 'e'
/* 66 */	{"PI",			11,	0,	0,	__PI},		// constant 'pi'
/* 67 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 68 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 69 */	{"random",		10,	0,	0,	__random},	// random number
/* 6A */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 6B */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 6C */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 6D */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 6E */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 6F */	{".UNUSED",		0,	0,	0,	_invalid_},

/* 70 */	{"format",		12,	0,	-1,	__format},	// param, param, ..., fmtstr ---> str
/* 71 */	{"sim",			10,	0,	2,	__sim},		// str1, str2 ---> [0...1] similarity
/* 72 */	{"search",		10,	0,	3,	__search},	// str_what, str_where, index ---> index/-1
/* 73 */	{"insert",		10,	0,	3,	__insert},	// str_what, str_where, index ---> str
/* 74 */	{"char",		10,	0,	1,	__char},	// ASCII ---> str[1]
/* 75 */	{"code",		10,	0,	2,	__code},	// str, index ---> ASCII
/* 76 */	{"cut",			10,	0,	3,	__cut},		// str, begin, count ---> str(cut), str(remainder)
/* 77 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 78 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 79 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 7A */	{"clear",		10,	0,	1,	__clrvar},	// clear variable or array
/* 7B */	{"isval",		10,	0,	1,	__isnum},	// param ---> 1 if number, 0 otherwise
/* 7C */	{"val",			10,	0,	1,	__asnum},	// str ---> number
/* 7D */	{"count",		10,	0,	1,	__count},	// return the the number of elements in an array
/* 7E */	{"type",		10,	0,	1,	__type},	// anything ---> data type code
/* 7F */	{"size",		10,	0,	1,	__size},	// return the byte size of the argument, or symbol length of a text

/* 80 */	{"delete",		10,	0,	1,	__delete},	// fname --> 0:success, negative:failed
/* 81 */	{"rename",		10,	0,	2,	__rename},	// oldf, newf --> 0:success, negative:failed
/* 82 */	{"copy",		10,	0,	2,	__copy},	// fname, path --> 0:success, negative:failed
/* 83 */	{"ffirst",		10,	0,	2,	__ffirst},	// name_pattern ---> file_name or blank_text
/* 84 */	{"fnext",		10,	0,	0,	__fnext},	// file_name or blank_text
/* 85 */	{"open",		10,	0,	1,	__open},	// fname ---> handler or negatine
/* 86 */	{"close",		10,	0,	1,	__close},	// handler ---> 0:success, negative:failed
/* 87 */	{"ioerr",		10,	0,	1,	__ioerr},	// handler ---> positive:error_code, negative:failed
/* 88 */	{"isopen",		10,	0,	1,	__isopen},	// handler ---> 1:open, 0:closed, -1:error
/* 89 */	{"eof",			10,	0,	1,	__eof},		// handler ---> 1:end_of_data, 0:more data available
/* 8A */	{"fpos",		10,	0,	1,	__fpos},	// handler ---> positive:current_position, negative:failed
/* 8B */	{"seek",		10,	0,	2,	__seek},	// handler, pos ---> 0:success, negative:failed
/* 8C */	{"fsize",		10,	0,	1,	__fsize},	// handler ---> file size in bytes, negative:error
/* 8D */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 8E */	{"write",		12,	0,	-1,	__write},	// handler, param, param... ---> 0+:written_bytes, <0:error
/* 8F */	{"read",		10,	0,	1,	__read},	// handler ---> data...

/* 90 */	{"init",		10,	0,	1,	__init},	// dev: ---> positive:initialised size, negative:error
/* 91 */	{"mount",		10,	0,	1,	__drive},	// dev: ---> 0:success, negative:error
/* 92 */	{"mkdir",		10,	0,	1,	__mkdir},	// path ---> 0:success, negative:error
/* 93 */	{"rmdir",		10,	0,	1,	__rmdir},	// path ---> 0:success, negative:error
/* 94 */	{"chdir",		10,	0,	1,	__chdir},	// path ---> 0:success, negative:error
/* 95 */	{"where",		10,	0,	1,	__where},	// return current drive/dir
/* 96 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 97 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 98 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 99 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 9A */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 9B */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 9C */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 9D */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 9E */	{".UNUSED",		0,	0,	0,	_invalid_},
/* 9F */	{".UNUSED",		0,	0,	0,	_invalid_},

/* A0 */	{"cls",			10,	0,	0,	__cls},		// clear screen
/* A1 */	{"fill",		10,	0,	3,	__fill},	// x, y, fill_colour
/* A2 */	{"font",		10,	0,	1,	__font},	// @ref to font header
/* A3 */	{"gpattr",		10,	0,	3,	__gpattr},	// Fcol, Bcol, fontScale
/* A4 */	{"gprint",		12,	0,	-1,	__gprint},	// x, y, any [, any, ...]
/* A5 */	{"pixel",		10,	0,	3,	__pixel},	// x, y, col
/* A6 */	{"line",		10,	0,	5,	__line},	// x1, y1, x2, y2, col
/* A7 */	{"circle",		10,	0,	4,	__circle},	// x, y, rad, col
/* A8 */	{"ellipse",		10,	0,	6,	__ellipse},	// x, y, xrad, yrad, tilt, col
/* A9 */	{"sector",		10,	0,	8,	__sector},	// x, y, xrad, yrad, tilt, ang1, ang2, col
/* AA */	{"triangle",	10,	0,	7,	__triangle}, // x1, y1, x2, y2, x3, y3, col
/* AB */	{"rect",		10,	0,	5,	__rect},	// x1, y1, x2, y2, col
/* AC */	{"gput",		10,	0,	5,	__gput},	// put from array
													// x1, y2, x2, y2, @var
/* AD */	{"gget",		10,	0,	5,	__gget},	// get into array
													// x1, y2, x2, y2, @var
/* AE */	{"shape",		10,	0,	3,	__shape},	// x,y,"defs"
													// C[colour]	 - draw
													// N			 - no draw
													// M [stX],[stY] - move
													// F			 - flood fill
/* AF */	{".UNUSED",		0,	0,	0,	_invalid_},

/* B0 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* B1 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* B2 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* B3 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* B4 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* B5 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* B6 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* B7 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* B8 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* B9 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* BA */	{".UNUSED",		0,	0,	0,	_invalid_},
/* BB */	{".UNUSED",		0,	0,	0,	_invalid_},
/* BC */	{".UNUSED",		0,	0,	0,	_invalid_},
/* BD */	{".UNUSED",		0,	0,	0,	_invalid_},
/* BE */	{".UNUSED",		0,	0,	0,	_invalid_},
/* BF */	{".UNUSED",		0,	0,	0,	_invalid_},

// BEGIN OF THE PLATFORM-SPECIFIC CODE AREA =================================================

#if defined(PIC32MZEF)
    #include "..\pic32mzef\tokens.c"

#else

/* C0 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* C1 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* C2 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* C3 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* C4 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* C5 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* C6 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* C7 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* C8 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* C9 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* CA */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* CB */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* CC */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* CD */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* CE */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* CF */	{".UNUSED",		0,	0,	0,	_unsupported_},

/* D0 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* D1 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* D2 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* D3 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* D4 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* D5 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* D6 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* D7 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* D8 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* D9 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* DA */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* DB */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* DC */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* DD */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* DE */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* DF */	{".UNUSED",		0,	0,	0,	_unsupported_},

/* E0 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* E1 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* E2 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* E3 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* E4 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* E5 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* E6 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* E7 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* E8 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* E9 */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* EA */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* EB */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* EC */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* ED */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* EE */	{".UNUSED",		0,	0,	0,	_unsupported_},
/* EF */	{".UNUSED",		0,	0,	0,	_unsupported_},

#endif

// END OF PLATFORM-SPECIFIC CODE AREA =======================================================

/* F0 */	{"platform",	10,	0,	0,	__platform}, // return platform name and software version
													// ---> str(name), str(version)
/* F1 */	{"isword",		10,	0,	1,	__isword},	// return 1 if the parameters is a valid Rittle word
/* F2 */	{"freemem",		10,	0,	0,	__freemem},	// return the amount of free data memory in the system
/* F3 */	{"uptime",		10,	0,	0,	__uptime},	// return number of microseconds since the start of execution
/* F4 */	{"wait",		10,	0,	1,	__wait},	// delay for specified number microseconds; positive number: non-blocking, negative: blocking
/* F5 */	{"tick",		10,	0,	2,	__tick},	// execute function at specified microsecond interval
/* F6 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* F7 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* F8 */	{".UNUSED",		0,	0,	0,	_invalid_},
/* F9 */	{"run",			10,	0,	1,	__run},		// str(name) load and execute external file
/* FA */	{"userbrk",		10,	0,	1,	__ctrlbrk},	// enable or disable the Ctrl-C break during execution
/* FB */	{"break",		99,	0,	0,	__break},   // break instruction for tracing
/* FC */	{"sysreset",	10,	0,	0,	sysreset},	// system reset
/* FD */	{".EXP2",		0,	0,	0,	_unsupported_},	// RESERVED for future code expansions
/* FE */	{".EXP1",		0,	0,	0,	_unsupported_},	// RESERVED for future code expansions
/* FF */	{".nop",		0,	0,	0,	__nop}		// NOP instruction (reserved for future expansions)

#undef IN_TOKENS
};


// EXPORTED FUNCTIONS =======================================================================

// initialise the RVM
// *debugfunc is a callback function used for debugging; its result may update the value of (xv)
void initRVM(unsigned char *code, unsigned long length, void *debug(void)) {
	mem=code;
	mlen=length;
	dbgfunc=(void *)debug;
	enbrk=1;
    encout=((xres.role==rrINPUT && xres.type==rFUNC && xres.data.sint64==-1)? 0 : 1);
    memset((unsigned char *)&xres,0,sizeof(rdata_t));
	uptcounter=0;
	cclkss=cclk();
	__reset();
}


// execute and return error code
// NOTE: initRVM() must be executed before first run
// 			negative number: error code
// 			0: no error
//			1: execution terminated normally
//			2: execution terminated by a break instruction
// (mode) defines the way of execution
//			0: execute with no breakpoints (normal executable)
//			1: execute with breakpoints
//			2: single step of all threads
//			3: single step of single thread
signed long execute(unsigned char *code, unsigned long length, char mode) {
	unsigned short c=0;
	signed long (*dbgsave)(void)=dbgfunc;
	xdefrag();
	if(!mode) dbgfunc=NULL;
    execlen=length;
	xmode=mode;
	signed char p;
	while(!xv){
		if(kbhit()==KEY_BREAK && enbrk) {	// check for Ctrl-C during execution (return xv=-12)
			while(kbhit()) getch(); // clear the console buffer
			xv=-12;
			break;		// immediately terminate execution
		}
		if(c++==0) {		// periodically update the uptime counter
			cclkss_old=cclkss;
			cclkss=cclk();
			// increase the uptime counter with the microseconds passed since the last cycle
			uptcounter+=((cclkss>=cclkss_old)? (cclkss-cclkss_old) : (cclkss+(~cclkss_old)+1));
		}
		platform_link();	// connect to platform functions
		if(rvm[thd]->tickval>0 && uptcounter>=rvm[thd]->tickcntr && rvm[thd]->dsp==0) {	// ticks
			rvm[thd]->tickcntr=LONG_LONG_MAX;	// do not execute tick within the tick function
			call_addr(rvm[thd]->tickcall, rvm[thd]->pc);
			rvm[thd]->tickcntr=uptcounter+rvm[thd]->tickval;
		}
        if(rvm[thd]->pc<execlen) {
			p=tokens[(unsigned char)(*(mem+(rvm[thd]->pc)))].params;
			if(p>0 && rvm[thd]->dsp<p) {	// for functions with specified number of parameters
				xv=-24;
				break;
			}
            tokens[(unsigned char)(*(mem+(rvm[thd]->pc++)))].exec();
        } else __exit();
		if(threads) {
			if(thd<threads) thd++; else thd=0;
		}
		if(mode==2 || mode==3) {
			if(mode==3 || !thd) break;
		}
    }
	signed long xvs=xv;	// store the original (xv) at this point
	if(xv && xv!=2) {	// do not release the memory on breakpoints
		unsigned char thds=thd;		// store the original (thd) value
		unsigned char thrs=threads;	// store the original (threads) value
		thd=threads;
		while(thd<=threads) {		// stop all running parallel threads and release memory
			__exit();
			thd--;
		}
		for(thd=0; thd<=MAX_THREADS; thd++) {
			if(rvm[thd]) xfree((unsigned char **)rvm[thd]);
		}
		for(thd=0; thd<MAX_FILES; thd++) {
			if(files[thd]) xfree((unsigned char **)files[thd]);
		}
		threads=thrs;
		thd=thds;
	}
	dbgfunc=dbgsave;
	xdefrag();
	while(kbhit()) getch();	// clear the console buffer
	return xvs;
}
