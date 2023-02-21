#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "rsc.h"

// constants for expression() to define which side of an equation is currently being compiled
#define EQULEFT		0
#define EQURIGHT	1

// single variable structure
typedef struct {
	unsigned short vid;			// unique variable id
	unsigned short protovid;	// prototype
	char name[MAX_ID_LENGTH+1];	// text name
	rtype_t type;				// data type of the variable
	rrole_t role;				// variable role
	signed long long dim[MAX_DIMENSIONS];	// dimensions
	signed long long maxlen;	// 'maxlen' parameter
	signed long long addr;		// 'at' specifier
	void *next;
} var_t;

// function/label structure
typedef struct {
	unsigned long offset;		// relative offset from (dstmem0)
	unsigned char type;			// record type: 0=function, 1=label
	char name[MAX_ID_LENGTH+1];	// text name
	var_t *input[MAX_PARAMS];	// input variables (only applies to functions)
	var_t *output[MAX_PARAMS];	// output variables (only applies to functions)
	void *next;
} fun_t;

const char *idstr = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.";	// valid identifier characters
const char *oprstr = "<>=^~*+-/\\";	// valid operator characters

char wrdestf=1;					// flag to indicate whether to perform writes to destination or just speculate
char wbuf[MAX_ID_LENGTH+1];		// text buffer
char wnew[MAX_ID_LENGTH+1];		// text buffer
var_t unit;						// parent unit structure
unsigned char *dstmem0=NULL;	// store the initial destination address
fun_t *funs=NULL;				// pointer to the beginning of the list of functions
var_t *vars=NULL;				// pointer to the beginning of the list of variables
signed short fn_index=0;		// function stack index
unsigned long *fn_jmpaddr=NULL;	// "func" address (jump after the corresponding "endfunc")
unsigned short vid=0;			// current available variable id
unsigned short *vidstk=NULL;	// stack for variable id's that sit on the left side in an equation
unsigned short *vidmod=NULL;	// equivalent index pointers for post-modifiers
unsigned short vinitlx=0;		// variable initialisation list index
unsigned short *vinitl=NULL;	// variable initialisation list
char **vididx=NULL;				// equivalent index pointers in the source
unsigned short vidstkx=0;		// index for vidstk[], vidmod[], and vididx[]
char equside=EQULEFT;			// current side in an equation: 0=left, 1=right
signed short cntbrackets=0;		// number of bracket levels that still remain open


// write to destination
void s2d(char *source, unsigned char **destination, char c) {
	unsigned char *dst=*destination;
	if(wrdestf) *(dst)=c;
	dst++;
	if(source && abs((char *)dst-source)<1) xv=-105;
	*destination=dst;
}


// write string to destination (without the terminating NUL character)
void str2d(char *source, unsigned char **destination, char *c) {
	unsigned char *dst=*destination;
    while(xv>=0 && c && *c) {
        if(wrdestf) *(dst)=*c;
        dst++;
        c++;
        if(source && abs((char *)dst-source)<1) xv=-105;
    }
	*destination=dst;
}


// store constant in the output file
// will return the same input structure (c) with the only exception when the input type is TEXT/MORE and the role is SOURCE
// in which case the (*.data.text) pointer will be modified in the output to point after the end of the text constant
// the parameter (addtkn) directs whether the data token should be inserted in the destination code before the constant
rdata_t store(rdata_t c, unsigned char **destination, char addtkn) {
	unsigned char *d=NULL;
	unsigned char k,l=0;
	if(c.type==rTEXT || c.type==_rTEXT) {
		if(c.type==rTEXT && addtkn) s2d(NULL,destination,token(".text"));
		if(c.role==rrSOURCE) {	// when (*.data.role) is SOURCE, (*.data.text) is modified to point after the string
			char *src=c.data.text;
			while(*src && *src!='\"' && xv>=0) {
				if(*src!='_') {
					s2d(src,destination,*src);
					src++;
				}
				else {
					if(*(++src)==0) xv=-102;
					else if(strchr("0123456789",*src)) {	// _nnn decimal ASCII code
						unsigned char v=0;
						while(strchr("0123456789",*src)) v=(10*v)+(*(src++)-'0');
						s2d(src,destination,v);
						if(*src==0) xv=-102;
					}
					else if(*src=='x' || *src=='X') {	// _Xnn hexadecimal ASCII code
						unsigned char v=0;
						while(strchr("0123456789abcdefABCDEF",*src)) {
							v*=16;
							if(*src>'9') v+=(((*(src++) & 0x5f)-'A')+0x0a); else v+=(*(src++)-'0');
						}
						s2d(src,destination,v);
						if(*src==0) xv=-102;
					}
					else if(*src=='a') { src++; s2d(src,destination,'\a'); }
					else if(*src=='b') { src++; s2d(src,destination,'\b'); }
					else if(*src=='t') { src++; s2d(src,destination,'\t'); }
					else if(*src=='n') { src++; s2d(src,destination,'\n'); }
					else if(*src=='v') { src++; s2d(src,destination,'\v'); }
					else if(*src=='f') { src++; s2d(src,destination,'\f'); }
					else if(*src=='r') { src++; s2d(src,destination,'\r'); }
					else if(*src=='e') { src++; s2d(src,destination,'\x1b'); }
					else if(*src=='\"') { src++; s2d(src,destination,'\"'); }
					else if(*src=='_') { src++; s2d(src,destination,'_'); }
					else xv=-102;
				}
			}
			if(*src=='\"') src++;
			c.data.text=src;
		} else {
			while(*(c.data.text)) {
				s2d(c.data.text,destination,*c.data.text);
				c.data.text++;
			}
		}
		s2d(NULL,destination,0);
	}
	else if(c.type==rSINT8) {
		l=sizeof(signed char);
		d=(unsigned char *)&(c.data.sint64);
		k=token(".sint8");
	}
	else if(c.type==rSINT16) {
		l=sizeof(signed short);
		d=(unsigned char *)&(c.data.sint64);
		k=token(".sint16");
	}
	else if(c.type==rSINT32) {
		l=sizeof(signed long);
		d=(unsigned char *)&(c.data.sint64);
		k=token(".sint32");
	}
	else if(c.type==rSINT64) {
		l=sizeof(signed long long);
		d=(unsigned char *)&(c.data.sint64);
		k=token(".sint64");
	}
	else if(c.type==rREAL) {
		l=sizeof(double);
		d=(unsigned char *)&(c.data.real);
		k=token(".real");
	}
	if(l) {
		if(addtkn) s2d(NULL,destination,k);
		while(l--) s2d(NULL,destination,*(d++));
	}
	return c;
}


// get a single identifier or operator word from the source text and skip the following white space
// (str) string with valid character set
char *word(char **source, const char *str) {
	char *w=wbuf;
	char *c=*source;
	*w=0;
	while(strchr(str,*c) && (w-wbuf)<MAX_ID_LENGTH) {
		*(w++)=*(c++);
		*w=0;
	}
	skip(&c,0);
	*source=c;
	return wbuf;
}


// return pointer to variable structure with given name, or return NULL if not found
var_t *var(char *name) {
	var_t *p=vars;
	while(p) {
		if(strcmp(p->name,name)) p=(var_t *)p->next; else break;
	}
	return p;
}


// return pointer to function structure with given name, or return NULL if not found
fun_t *fun(char *name) {
	fun_t *p=funs;
	while(p) {
		if(strcmp(p->name,name)) p=(fun_t *)p->next; else break;
	}
	return p;
}


// get a constant (numeric and text) from the source
// text constants are automatically stored in the output
// returns data_t structure with the constant
// (ignsign) 0:normal operation (return also negative numbers), 1:ignore the sign in numbers and always return positive numbers
// the parameter (addtkn) directs whether the data token should be inserted in the destination code before the constant
rdata_t constant(char **source, unsigned char **destination, char ignsign, char addtkn) {
	char *src=*source;
	rdata_t x;
	x.type=rINVALID;
	x.role=rrNONE;
	x.data.text=NULL;

	// _"....." text constants appended to previous text constants
	if(*src=='_' && *(src+1)=='\"' && *(*destination-1)==0) {
		unsigned char *d=*(--destination);
		while(*d) d--;
		x.data.text=(char *)d;
		x.type=_rTEXT;
		src++;
	}

	// text constants
	if(*src=='\"') {
		if(!x.data.text) {
			x.data.text=(src+1);
			x.type=rTEXT;
		}
		rdata_t d;
		memcpy(&d,&x,sizeof(rdata_t));
		d.role=rrSOURCE;
		d=store(d,destination,addtkn);
		src=d.data.text;
	}

	// binary numeric constants
	else if(*src=='0' && (*(src+1)=='b' || *(src+1)=='B')) {
		src+=2;
		unsigned long long v=0;
		while(*src && xv>=0) {
			if(*src=='0' || *src=='1') v=(v<<1)+(*(src++)-'0'); else break;
		}
		cast(&v,&x);
	}

	// hexadecimal numeric constants
	else if(*src=='0' && (*(src+1)=='x' || *(src+1)=='X')) {
		src+=2;
		unsigned long long v=0;
		while(*src && xv>=0) {
			if(strchr("0123456789abcdefABCDEF",*src)) {
				v*=16;
				if(*src>'9') v+=(((*(src++) & 0x5f)-'A')+0x0a); else v+=(*(src++)-'0');
			} else break;
		}
		cast(&v,&x);
	}

	// decimal numeric constants (any type)
	else if(strchr("0123456789.",*src) || (!ignsign && *src=='+' && strchr("0123456789.",*(src+1)))  ||
			(!ignsign && *src=='-' && strchr("0123456789.",*(src+1)))) {
		double v=0.0;
		signed char vs=1;	// fraction sign 1:positive, -1:negative
		signed char es=1;	// 'E' sign 1:positive, -1:negative
		double f=0.0, e=0.0;
		char dp0=0,dp=0;	// 1:'decimal point found' flag
		char ef=0;	// 1:'E found' flag
		if(!ignsign) {
			if(*src=='+') src++;
			else if(*src=='-') {
				vs=-1;
				src++;
			}
		}
		while(xv>=0 && *src) {
			if(strchr("0123456789",*src)) {
				if(!ef) {
					if(!dp) v=(10*v)+(*src-'0');
					else {
						v+=(f*(*src-'0'));
						f*=0.1;
					}
				} else {
					if(!dp) e=(10*e)+(*src-'0');
					else {
						e+=(f*(*src-'0'));
						f*=0.1;
					}
				}
				src++;
			} else {
				if(*src=='.') {
					if(!dp) {
						dp0=dp=1;
						f=0.1;
						src++;
					} else xv=-103;
				}
				else if(*src=='E' || *src=='e') {
					if(!ef) {
						dp=0;
						ef=1;
						src++;
						if(*src=='-' || *src=='+') {
							if(*(src++)=='-') {
								es=-1;
								f=0.1;
							}
						}
					} else xv=-103;
				}
				else break;
			}
		}
		if(xv>=0) {
			v=v*vs*pow(10,(e*es));
			if(v==floor(v) && dp0==0 && (v>=LONG_LONG_MIN && v<=LONG_LONG_MAX)) {	// integer number?
				signed long long z=(signed long long)xround(v);
				cast(&z,&x);
				if(ignsign && x.data.sint64<0) x.data.sint64=-x.data.sint64;
			} else {	// real number
				x.type=rREAL;
				x.data.real=v;
				if(ignsign && x.data.real<0) x.data.real=-x.data.real;
			}
		}
	}
	skip(&src,0);
	*source=src;
	return x;
}


// get role and data type from source and modify the source pointer if necessary
char *type(char **source, rdata_t *x) {
	char *src=*source;
	char *tn=NULL;
	if(!src) return tn;
	memset((unsigned char *)x,0,sizeof(rdata_t));
	x->role=rrNONE;
	if(!strncmp("input",src,5) && (*(src+5)==0 || *(src+5)==' ')) {
		skip(&src,5);
		x->role=rrINPUT;
	}
	else if(!strncmp("output",src,6) && (*(src+6)==0 || *(src+6)==' ')) {
		skip(&src,6);
		x->role=rrOUTPUT;
	}
	else if(!strncmp("refer",src,5) && (*(src+5)==0 || *(src+5)==' ')) {
		skip(&src,5);
		x->role=rrREFER;
	}
	*source=src;
	x->type=rINVALID;
	if(!strncmp("any",src,3) && (*(src+3)==0 || *(src+3)==' ')) {
		skip(&src,3);
		x->type=rANY;
	}
	else if(!strncmp("sint8",src,5) && (*(src+5)==0 || *(src+5)==' ')) {
		skip(&src,5);
		x->type=rSINT8;
	}
	else if(!strncmp("byte",src,4) && (*(src+4)==0 || *(src+4)==' ')) {		// human format
		skip(&src,4);
		x->type=rSINT8;
	}
	else if(!strncmp("sint16",src,6) && (*(src+6)==0 || *(src+6)==' ')) {
		skip(&src,6);
		x->type=rSINT16;
	}
	else if(!strncmp("small",src,5) && (*(src+5)==0 || *(src+5)==' ')) {	// human format
		skip(&src,5);
		x->type=rSINT16;
	}
	else if(!strncmp("sint32",src,6) && (*(src+6)==0 || *(src+6)==' ')) {
		skip(&src,6);
		x->type=rSINT32;
	}
	else if(!strncmp("int",src,3) && (*(src+3)==0 || *(src+3)==' ')) {		// human format
		skip(&src,3);
		x->type=rSINT32;
	}
	else if(!strncmp("sint64",src,6) && (*(src+6)==0 || *(src+6)==' ')) {
		skip(&src,6);
		x->type=rSINT64;
	}
	else if(!strncmp("big",src,3) && (*(src+3)==0 || *(src+3)==' ')) {		// human format
		skip(&src,3);
		x->type=rSINT64;
	}
	else if(!strncmp("real",src,4) && (*(src+4)==0 || *(src+4)==' ')) {
		skip(&src,4);
		x->type=rREAL;
	}
	else if(!strncmp("text",src,4) && (*(src+4)==0 || *(src+4)==' ')) {
		skip(&src,4);
		x->type=rTEXT;
	}
	else if(!strncmp("func",src,4) && (*(src+4)==0 || *(src+4)==' ')) {
		skip(&src,4);
		x->type=rFUNC;
	}
	else {	// check in the definitions
		var_t *v=vars;
		while(v) {
			if(/* v->type==rUNIT && */ !strncmp(v->name,src,strlen(v->name)) &&
				(*(src+strlen(v->name))==' ' || *(src+strlen(v->name))==0)) {
				skip(&src,strlen(v->name));
				break;
			}
			v=(var_t *)v->next;
		}
		if(v) {
			x->type=v->type;
			tn=v->name;
		}
		else xv=-114;
	}
	if(!tn && x->type) {
		tn=(char *)tokens[x->type].name;
		if(*tn=='.') tn++;
	}
	*source=src;
	return tn;
}


// forward definition
rdata_t expression(char **source, unsigned char **destination, char equside, signed short *cntbrackets);


// generate indexing code for array variables
void idx(var_t *v, char **source, unsigned char **destination) {
	if(source && *source && v && (v->dim[0] || v->role==rrREFER)) {
		char *src=*source;
		skip(&src,0);
		unsigned char t, dm, cc=1;
		for(dm=0; dm<MAX_DIMENSIONS && v->dim[dm]; dm++);	// count how many dimensions this variable have
		if(*src=='[') {
			skip(&src,1);
			if(*src!=']') {
				for(t=0; *src && xv>=0 && (t<dm || v->role==rrREFER); t++) {
					rdata_t d=expression(&src,destination,EQURIGHT,&cntbrackets);
					if(xv>=0 && d.type!=rSINT8 && d.type!=rSINT16 && d.type!=rSINT32 && d.type!=rSINT64) xv=-116;
					if(*src==':') {
						skip(&src,1);
						cc++;
					}
					else break;
				}
			}
			if((v->role!=rrREFER && cc>dm) || *src!=']') xv=-118;
			else if(*src==']') {
				skip(&src,1);
				s2d(*source,destination,token(".index"));
				s2d(*source,destination,(unsigned char)(v->vid));
				s2d(*source,destination,(unsigned char)((v->vid)>>8));
			}
		}
		*source=src;
	}
}


// generate code for function call or jump to label
void call(fun_t *f, char **source, unsigned char **destination) {
	char *src=*source;
	signed char t;
	skip(&src,0);
	if(f->type==0) {
		for(t=(MAX_PARAMS-1); xv>=0 && t>=0; t--) {
			if(f->input[t]>0) expression(&src,destination,EQURIGHT,&cntbrackets);
			if(*src==')') {
				skip(&src,1);
				cntbrackets--;
			}
			else if(*src==',') skip(&src,1);
			else if(*src==';' || *src<' ') break;
		}
		s2d(*source,destination,token(".call"));
	} else s2d(*source,destination,token(".goto"));
	s2d(*source,destination,(unsigned char)(f->offset));
	s2d(*source,destination,(unsigned char)((f->offset)>>8));
	s2d(*source,destination,(unsigned char)((f->offset)>>16));
	s2d(*source,destination,(unsigned char)((f->offset)>>24));
	if(f->type==0) {
		for(t=(MAX_PARAMS-1); xv>=0 && t>=0; t--) {
			if(f->output[t]>0) expression(&src,destination,EQURIGHT,&cntbrackets);
			if(*src==')') {
				skip(&src,1);
				cntbrackets--;
			}
			else if(*src==',') skip(&src,1);
			else if(*src==';' || *src<' ') break;
		}
	}
	*source=src;
}


// generate code for token parameters
void tknparams(unsigned char k, char **source, unsigned char **destination) {
	if(tokens[k].params<0 || tokens[k].params>1) {
		char *src=*source;
		signed char t;
		for(t=0; xv>=0 && (tokens[k].params<0 || t<tokens[k].params); t++) {
			expression(&src,destination,EQURIGHT,&cntbrackets);
			if(*src==')') {
				skip(&src,1);
				cntbrackets--;
			}
			else if(*src==',') skip(&src,1);
			else if(*src==';' || *src<' ') break;
			else {
				if(tokens[k].params<0) break;
				else xv=-101;
			}
		}
		*source=src;
	}
}


// store get/set operations with vars and unroll complex vars
void varopr(char **source, unsigned char **destination, unsigned char tkncode, unsigned short vid) {
	var_t *v=vars;
	while(v && v->vid!=vid) v=(var_t *)v->next;	// find the variable
	if(!v) {
		xv=-128;
		return;
	}
	if(v->type!=rUNIT) {	// simple variables
		s2d(*source,destination,tkncode);
		s2d(*source,destination,(unsigned char)(v->vid));
		s2d(*source,destination,(unsigned char)((v->vid)>>8));
	}
	else {	// unroll units
		var_t *v1=vars;
		if(tkncode!=token(".set")) {
			while(v1 && xv>=0) {
				if(!memcmp(v1->name,v->name,strlen(v->name)) && *(v1->name+strlen(v->name))=='.') {
					if(v1->type!=rUNIT) {
						s2d(*source,destination,tkncode);
						s2d(*source,destination,(unsigned char)(v1->vid));
						s2d(*source,destination,(unsigned char)((v1->vid)>>8));
					}
					else varopr(source,destination,tkncode,v1->vid);
				}
				v1=(var_t *)v1->next;
			}
		}
		else {	// .set operations will unroll the unit in reverse order
			unsigned long cnt=0;
			while(v1) {	// count the total number of member variables in the unit
				if(!memcmp(v1->name,v->name,strlen(v->name)) && *(v1->name+strlen(v->name))=='.') cnt++;
				v1=(var_t *)v1->next;
			}
			while(cnt-- && xv>=0) {
				unsigned long t=0;
				v1=vars;
				while(v1) {
					if(!memcmp(v1->name,v->name,strlen(v->name)) && *(v1->name+strlen(v->name))=='.') t++;
					if(t>=cnt) break;
					v1=(var_t *)v1->next;
				}
				if(v1) {
					if(v1->type!=rUNIT) {
						s2d(*source,destination,tkncode);
						s2d(*source,destination,(unsigned char)(v1->vid));
						s2d(*source,destination,(unsigned char)((v1->vid)>>8));
					}
					else varopr(source,destination,tkncode,v1->vid);
				}
			}
		}
	}
}


// execute a pre- or post-modifier of a variable
void modopr(char **source, unsigned char **destination, var_t *v, signed short m) {
	char *src=*source;
	char *src0=src;
	if(m!=0) {
		if(v) idx(v,&src,destination);
		if(m==1 || m==-1) {
			if(v) varopr(&src,destination,token(".get"),(v->vid));
			if(m==1) s2d(*source,destination,token(".++")); else s2d(*source,destination,token(".--"));
			if(v) varopr(&src,destination,token(".set"),(v->vid));
		} else {
			if(v) varopr(&src,destination,token(".get"),(v->vid));
			s2d(*source,destination,token(".sint16"));
			s2d(*source,destination,(unsigned char)(m));
			s2d(*source,destination,(unsigned char)(m>>8));
			if(m>0) s2d(*source,destination,token("+")); else s2d(*source,destination,token("-"));
			if(v) varopr(&src,destination,token(".set"),(v->vid));
		}
	}
	*source=src0;
}


// unroll unit and create reference to all of its variables
void refunit(char **source, unsigned char **destination, var_t *z) {
	var_t *v=vars;
	while(v) {
		if(!strncmp(v->name,z->name,strlen(z->name)) && *(v->name+strlen(z->name))=='.') {
			if(v->type!=rUNIT) {
				s2d(*source,destination,token(".refer"));
				s2d(*source,destination,(unsigned char)(v->vid));
				s2d(*source,destination,(unsigned char)((v->vid)>>8));
				s2d(*source,destination,(unsigned char)0xed);
				s2d(*source,destination,(unsigned char)0xfe);	// vid key
			}
			else refunit(source,destination,v);
		}
		v=(var_t *)v->next;
	}
}


// compile given expression, store it in the output, and return data structure with the result (if constant)
// expression is a piece of source that ends with ';' or ',' or '='
// (equside) current side in an equation: 0=left, 1=right
// (*cntbrackets) returns the number of bracket levels that still remain open
rdata_t expression(char **source, unsigned char **destination, char equside, signed short *cntbrackets) {
	char *src=*source;
	rdata_t x;
	memset((unsigned char *)&x,0,sizeof(rdata_t));
	x.type=rINVALID;
	x.role=rrNONE;
	unsigned char *stack=NULL;
	unsigned char *level=NULL;
	xalloc((unsigned char **)&stack,((MAX_TKEXPR+1)*sizeof(unsigned char)));
	xalloc((unsigned char **)&level,((MAX_TKEXPR+1)*sizeof(unsigned char)));
	if(!stack || !level) {	// unable to allocate memory for the arrays
		xv=-105;
		return x;
	}
	unsigned short stx=0;
	signed char igns=0;
	for(skip(&src,0); (*src && xv>=0 && stx<MAX_TKEXPR); skip(&src,0)) {

		if(*src=='(') {
			(*cntbrackets)++;
			stack[stx++]=1;	// token code 1 indicates opening bracket
			igns=0;
			src++;
			continue;
		}

		else if(*src==')' && stx) {
			(*cntbrackets)--;
			while(stx) {
				if(stack[--stx]!=1) s2d(*source,destination,stack[stx]); else break;
			}
			src++;
			continue;
		}

		char vptr=0;	// flag to indicate that the variable is a pointer
		signed short premod=0;	 // positive number indicates ++ operations, negative number is for -- operations
		signed short postmod=0;
		char *w, *ss=src;
		char pflag;
		skip(&src,0);
		do {	// check for pre-modifiers
			pflag=0;
			if(*src=='+' && *(src+1)=='+') {
				src+=2;
				premod++;
				pflag=1;
			}
			if(*src=='-' && *(src+1)=='-') {
				src+=2;
				premod--;
				pflag=1;
			}
			skip(&src,0);
		} while(pflag);

		rdata_t d=constant(&src,destination,igns,1);	// check for constants
		igns=1;
		if(d.type!=rINVALID) {	// valid constant
			memcpy(&x,&d,sizeof(rdata_t));
			if(x.type!=rTEXT) store(x,destination,1);
			modopr(&src,destination,NULL,premod);	// this adds the pre-modifier AFTER the constant (otherwise doesn't make sense)
			continue;
		}
		w=word(&src,idstr);

		if(*w==0 && (*src!='=' || (*src=='=' && *(src+1)=='='))) w=word(&src,oprstr);
		if(*w==0 && *src=='$') {
			skip(&src,1);
			strcpy(wbuf,"format");
			w=wbuf;
		}

		if(*w==0 && *src=='@') {
			skip(&src,1);
			w=word(&src,idstr);
			if(*w==0) xv=-120;
			vptr=1;
		}

		if(*w==0) break;
		unsigned char k=token(w);
		fun_t *f=fun(w);
		var_t *v=var(w);
		if(xv>=0 && (f || k) && (premod!=0 || postmod!=0)) {	// functions and reserved words can't have pre- and post-modifiers
			xv=-119;
			break;
		}

		// variable
		if(v) {
			modopr(&src,destination,v,premod);	// pre-modifier
			skip(&src,0);
			char *s=src;
			unsigned char *dsttemp=*destination;
			if(*src=='[') vididx[vidstkx]=src;
			idx(v,&src,destination);	// skip for now if there are any indexes
			src=s;
			*destination=dsttemp;

			// check for post-modifiers
			do {
				pflag=0;
				if(*src=='+' && *(src+1)=='+') {
					src+=2;
					postmod++;
					pflag=1;
				}
				if(*src=='-' && *(src+1)=='-') {
					src+=2;
					postmod--;
					pflag=1;
				}
				skip(&src,0);
			} while(pflag);

			// normal access to variable (not pointer)
			if(!vptr) {
				if(equside==EQURIGHT) {
					idx(v,&src,destination);
					varopr(&src,destination,token(".get"),(v->vid));
					modopr(&src,destination,v,postmod);	// post-modifier
					x.type=v->type;
				} else {
					unsigned char *dsttemp=*destination;
					idx(v,&src,destination);
					*destination=dsttemp;
					vidstk[vidstkx]=v->vid;		// push in the stack for a 'set' operation later
					vidmod[vidstkx]=postmod;	// push in the stack for a postmod()  operation later
					vidstkx++;
				}
			}

			// push variable id into stack
			else {
				if(v->dim[0]) {
					if(xv>=0 && *src=='[') skip(&src,1); else xv=-117;
					if(xv>=0 && *src==']') skip(&src,1); else xv=-118;
				}
				if(xv>=0) {
					if(v->type!=rUNIT) {
						s2d(*source,destination,token(".refer"));
						s2d(*source,destination,(unsigned char)(v->vid));
						s2d(*source,destination,(unsigned char)((v->vid)>>8));
						s2d(*source,destination,(unsigned char)0xed);
						s2d(*source,destination,(unsigned char)0xfe);	// vid key
						modopr(&src,destination,v,postmod);				// post-modifier
					}
					else refunit(&src,destination,v);
				}
			}

		}

		// function
		else if(f) {
			equside=EQURIGHT;	// enforce right side equation behaviour
			if(!vptr) call(f,&src,destination);
			else {
				s2d(*source,destination,token(".reffn"));
				s2d(*source,destination,(unsigned char)(f->offset));
				s2d(*source,destination,(unsigned char)((f->offset)>>8));
				s2d(*source,destination,(unsigned char)((f->offset)>>16));
				s2d(*source,destination,(unsigned char)((f->offset)>>24));
				break;
			}
			x.type=rFUNC;
		}

		// reserved word (token)
		else if(k && !vptr) {
			equside=EQURIGHT;	// enforce right side equation behaviour
			while(stx) {
				if(stack[stx-1]!=1 && level[stx-1]>=tokens[k].level) s2d(*source,destination,stack[--stx]); else break;
			}
			stack[stx]=k;
			level[stx]=tokens[k].level;
			stx++;
			tknparams(k,&src,destination);
			x.type=rINVALID;
		}

		// could be an error
		else {
			if(vptr) xv=-120;
			else { // ### if(*src==';' || *src<' ' || *src==',' || (*src=='=' && *(src+1)!='=')) {
				src=ss;
				break;
			}
			// ### else xv=-124;
		}
	}

	while(stx) {
		if(stack[--stx]!=1) s2d(*source,destination,stack[stx]);
	}
	xfree((unsigned char **)&level);
	xfree((unsigned char **)&stack);
	skip(&src,0);
	*source=src;
	return x;
}


// add new variable
// return pointer to the new variable
var_t *newvar(char **source, unsigned char **destination, var_t *nv, unsigned short *tkn, unsigned short level, char dbginfo) {
	char *src=*source;
	if(level>=MAX_LEVEL) {
		xv=-108;
		return NULL;
	}

	// insert debug information (if enabled)
	if(dbginfo) {
		if(*tkn) {
			s2d(*source,destination,0);
			s2d(*source,destination,0);
			*tkn=0;
		}
		s2d(*source,destination,token(".comment"));
		str2d(*source,destination,"var ");
		str2d(*source,destination,(char *)tokens[nv->type].name);
		str2d(*source,destination," ");
		str2d(*source,destination,nv->name);
		s2d(*source,destination,0);
	}

	var_t *p=vars;
	while(p && p->next) p=(var_t *)p->next;
	var_t *n=NULL;
	xalloc((unsigned char **)&n,sizeof(var_t));	// the new block gets cleared here during allocation
	if(n) {
		if(p) p->next=n; else vars=n;
		memcpy(n,nv,sizeof(var_t));
		if(++vid==0) vid=1;				// avoid vid 0
		n->vid=vid;						// assign unique id to the variable
		n->next=NULL;

		// input/output/refer role in functions
		if(level) {
			fun_t *f=funs;
			while(f && f->next) f=(fun_t *)f->next;	// take the most recent function (the one we are currently in)
			if(n->type!=rUNIT) {
				if(n->role==rrINPUT || n->role==rrREFER) {
					unsigned char t;
					for(t=0; t<MAX_PARAMS && f->input[t]; t++);
					if(t<MAX_PARAMS) f->input[t]=n; else xv=-123;
				}
				if(n->role==rrOUTPUT) {
					unsigned char t;
					for(t=0; t<MAX_PARAMS && f->output[t]; t++);
					if(t<MAX_PARAMS) f->output[t]=n; else xv=-123;
				}
			}
			if(xv<0) return n;
		}

		// array
		unsigned char t, dimc;
		for(dimc=0; dimc<MAX_DIMENSIONS && n->dim[dimc]; dimc++);	// count the inherited dimensions in (dimc)
		if(*src=='[' || dimc) {
			char bf=0;
			if(*src=='[') {
				skip(&src,1);
				bf=1;
			}
			if(n->role!=rrREFER) {
				if(*tkn) {
					s2d(*source,destination,0);
					s2d(*source,destination,0);
					*tkn=0;
				}
				rdata_t z;
				for(t=0; t<dimc; t++) {	// insert the inherited dimensions
					cast(&n->dim[t],&z);
					z.role=rrNONE;
					store(z,destination,1);
				}
				if(bf) {
					for(t=1; *src && xv>=0; t++) {
						rdata_t d=expression(&src,destination,EQURIGHT,&cntbrackets);
						if(d.type==rSINT8 || d.type==rSINT16 || d.type==rSINT32 || d.type==rSINT64) {
							if(dimc<MAX_DIMENSIONS) n->dim[dimc++]=d.data.sint64; else xv=-115;
						} else xv=-116;
						if(*src==']') break;
						else if(*src==':') skip(&src,1);
						else if(!dimc) xv=-118;
					}
				}
			}
			if(bf) {
				if(*src==']') skip(&src,1); else xv=-118;
			}
			if(xv<0) return n;
		}

		// maxlen
		if((!strncmp("maxlen",src,6) || n->maxlen) && n->role!=rrREFER) {
			if(n->type==rTEXT) {
				if(*tkn) {
					s2d(*source,destination,0);
					s2d(*source,destination,0);
					*tkn=0;
				}
				if(!strncmp("maxlen",src,6)) {
					skip(&src,6);
					if(*src=='[') {
						skip(&src,1);
						rdata_t d=expression(&src,destination,EQURIGHT,&cntbrackets);
						if(d.type==rSINT8 || d.type==rSINT16 || d.type==rSINT32 || d.type==rSINT64) n->maxlen=d.data.sint64; else xv=-116;
						if(*src==']') skip(&src,1); else xv=-118;
					} else xv=-117;
				}
				else {	// insert the inherited maxlen parameter
					rdata_t z;
					cast(&n->maxlen,&z);
					z.role=rrNONE;
					store(z,destination,1);
				}
			} else xv=-125;
			if(xv<0) return n;
		}

		// address positioning
		if((!strncmp("at ",src,3) || n->addr) && n->role!=rrREFER) {
			if(*tkn) {
				s2d(*source,destination,0);
				s2d(*source,destination,0);
				*tkn=0;
			}
			if(!strncmp("at ",src,3)) {
				skip(&src,2);
				rdata_t d=expression(&src,destination,EQURIGHT,&cntbrackets);
				if(d.type==rSINT8 || d.type==rSINT16 || d.type==rSINT32 || d.type==rSINT64) n->addr=d.data.sint64; else xv=-116;
				if(xv<0) return n;
			}
			else {	// insert the inherited address parameter
				rdata_t z;
				cast(&n->addr,&z);
				z.role=rrNONE;
				store(z,destination,1);
			}
		}

		// check for top-level error conditions
		if((n->role==rrINPUT || n->role==rrOUTPUT) && (n->type==rUNIT || n->dim[0])) {
			xv=-132;
			return n;
		}

		// single data types
		if(n->type!=rUNIT) {

			// insert token block
			if(*tkn==0 || (*tkn!=((unsigned short)n->role<<8)+(n->type))) {
				if(*tkn) {
					s2d(*source,destination,0);
					s2d(*source,destination,0);
					*tkn=0;
				}
				if(n->role==rrINPUT) s2d(*source,destination,token(".varin"));
				else if(n->role==rrOUTPUT) s2d(*source,destination,token(".varout"));
				else if(n->role==rrREFER) s2d(*source,destination,token(".varref"));
				else s2d(*source,destination,token(".var"));
				if(n->type==rANY) s2d(*source,destination,token(".any"));
				else if(n->type==rSINT8) s2d(*source,destination,token(".sint8"));
				else if(n->type==rSINT16) s2d(*source,destination,token(".sint16"));
				else if(n->type==rSINT32) s2d(*source,destination,token(".sint32"));
				else if(n->type==rSINT64) s2d(*source,destination,token(".sint64"));
				else if(n->type==rREAL) s2d(*source,destination,token(".real"));
				else if(n->type==rTEXT) s2d(*source,destination,token(".text"));
				else if(n->type==rFUNC) s2d(*source,destination,token(".fptr"));
				else {
					xv=-114;
					return n;
				}
				*tkn=((unsigned short)n->role<<8)+(n->type);
			}

			// store the variable id (for discrete variables only)
			s2d(*source,destination,(unsigned char)(n->vid));
			s2d(*source,destination,(unsigned char)((n->vid)>>8));

			if(n->role!=rrREFER) {

				// the new variable has an address positioning parameter
				if(n->addr) {
					if(*tkn) {
						s2d(*source,destination,0);
						s2d(*source,destination,0);
						*tkn=0;
					}
					varopr(&src,destination,token(".vafix"),n->vid);
				}

				// the new variable has a maxlen parameter
				if(n->maxlen) {
					if(*tkn) {
						s2d(*source,destination,0);
						s2d(*source,destination,0);
						*tkn=0;
					}
					varopr(&src,destination,token(".maxlen"),n->vid);
				}

				// the new variable is an array
				if(n->dim[0]) {
					if(*tkn) {
						s2d(*source,destination,0);
						s2d(*source,destination,0);
						*tkn=0;
					}
					varopr(&src,destination,token(".vardim"),n->vid);
					s2d(*source,destination,dimc);
				}
			}

		}

		// units
		else if(*tkn==0) {

			// store the unit variable
			if(n->role==rrNONE) {
				s2d(*source,destination,token(".var"));
				s2d(*source,destination,token(".unit"));
				s2d(*source,destination,(unsigned char)(n->vid));
				s2d(*source,destination,(unsigned char)((n->vid)>>8));
				s2d(*source,destination,0);
				s2d(*source,destination,0);
				if(n->role!=rrREFER) {
					if(n->addr) {
						s2d(*source,destination,token(".vafix"));
						s2d(*source,destination,(unsigned char)(n->vid));
						s2d(*source,destination,(unsigned char)((n->vid)>>8));
					}
					if(n->maxlen) {
						s2d(*source,destination,token(".maxlen"));
						s2d(*source,destination,(unsigned char)(n->vid));
						s2d(*source,destination,(unsigned char)((n->vid)>>8));
					}
					if(n->dim[0]) {
						s2d(*source,destination,token(".vardim"));
						s2d(*source,destination,(unsigned char)(n->vid));
						s2d(*source,destination,(unsigned char)((n->vid)>>8));
						s2d(*source,destination,dimc);
					}
				}
			}

			// unroll the unit
			unsigned char pdots=0;
			char *pdc;
			var_t *pv=vars;
			while(pv && pv->vid!=n->protovid) pv=(var_t *)pv->next;	// find the proto-variable
			if(pv) {
				pdc=pv->name;
				while(pdc && *pdc) if(*(pdc++)=='.') pdots++;
			}
			var_t *v=vars;
			while(pv && v) {
				unsigned char dots=0;
				pdc=v->name;
				while(pdc && *pdc) if(*(pdc++)=='.') dots++;
				if((dots-pdots)==1 && !strncmp(v->name,pv->name,strlen(pv->name))
					&& *(v->name+strlen(pv->name))=='.') {
					var_t vv;
					memcpy(&vv,v,sizeof(var_t));
					if(n->addr) vv.addr=n->addr;
					if(n->maxlen) vv.maxlen=n->maxlen;
					if(n->dim[0]) {
						unsigned char t, dc=0;
						for(t=0; t<MAX_DIMENSIONS; t++) vv.dim[t]=n->dim[t];
						for(dc=0; dc<MAX_DIMENSIONS && vv.dim[dc]; dc++);
						for(t=dots; t<MAX_DIMENSIONS && dc<MAX_DIMENSIONS && v->dim[t]; t++) vv.dim[dc++]=v->dim[t];
					}
					strcpy(wbuf,n->name);
					char *xn=v->name;
					while(*xn && *xn!='.') xn++;
					if((strlen(n->name)+strlen(xn))>MAX_ID_LENGTH) {
						xv=-131;
						return n;
					}
					strcat(wbuf,xn);
					strcpy((char *)&vv.name,wbuf);
					vv.role=n->role;
					vv.vid=0;
					char qs=0;
					xn=&qs;
					newvar(&xn, destination, &vv, tkn, level, dbginfo);
				}
				v=(var_t *)v->next;
			}
		}

	} else xv=-105;
	*source=src;
	return n;
}


// compile and return error code (negative number) or total compiled code length (positive number or 0)
// (level) must be 0 when the function is called from outside
// (scf) is a "speculative compilation flag" - performs compilation without writing into the destination
//				the destination however still needs to point to a valid memory address
//				this can be used to estimate the size of a compiled code before performing an actual compilation
// (dbginfo) is a flag specifying whether debug commentaries should be included in the output code
signed long compile(char **source, unsigned char **destination, unsigned short level, char scf, char dbginfo) {
	char *src=*source;
	if(level==0) {
		wrdestf=!scf;
		rscline=0;
		rscsrc=*source;
		dstmem0=*destination;	// store the initial destination pointer for later offset calculations
		funs=NULL;
		vars=NULL;
		vid=0;
		memset(&unit,0,sizeof(var_t));
		xv=0;
		xalloc((unsigned char **)&vidstk,((MAX_TKEXPR+1)*sizeof(unsigned short)));
		xalloc((unsigned char **)&vidmod,((MAX_TKEXPR+1)*sizeof(unsigned short)));
		xalloc((unsigned char **)&vinitl,((MAX_TKEXPR+1)*sizeof(unsigned short)));
		xalloc((unsigned char **)&vididx,((MAX_TKEXPR+1)*sizeof(unsigned char *)));
		xalloc((unsigned char **)&fn_jmpaddr,(MAX_NESTED*sizeof(unsigned long)));
		if(!vidstk || !vidmod || !vinitl || !vididx || !fn_jmpaddr) {
			xv=-105;
			return 0;
		}
		s2d(*source,destination,token(".reset"));
		s2d(*source,destination,0x4b);
		s2d(*source,destination,0xb8);
		s2d(*source,destination,0x44);
		s2d(*source,destination,token(".nop"));
	}
	else if(level>=MAX_LEVEL) {
		xv=-108;
		return 0;
	}

	// code bookmarks
	signed short wh_index=0;
	unsigned long *wh_bgnaddr=NULL;	// begin "while" address (start of the "while" condition)
	unsigned long *wh_endaddr=NULL;	// end "while" address (first instruction after "until")
	signed short if_index=0;
	unsigned long *if_nxtaddr=NULL;	// address of the next "else" branch
	unsigned long *if_endaddr=NULL;	// end "if" address (first instruction after "endif")

	// reserve memory for the arrays
	xalloc((unsigned char **)&wh_bgnaddr,(MAX_NESTED*sizeof(unsigned long)));
	xalloc((unsigned char **)&wh_endaddr,(MAX_NESTED*sizeof(unsigned long)));
	xalloc((unsigned char **)&if_nxtaddr,(MAX_NESTED*sizeof(unsigned long)));
	xalloc((unsigned char **)&if_endaddr,(MAX_NESTED*sizeof(unsigned long)));

	if(wh_bgnaddr && wh_endaddr && if_nxtaddr && if_endaddr) {

		// create local copy of the variable and function structures
		var_t *vars0=vars;
		while(vars0 && vars0->next) vars0=(var_t *)vars0->next;
		fun_t *funs0=funs;
		while(funs0 && funs0->next) funs0=(fun_t *)funs0->next;

		// main compilation loop
		cntbrackets=0;
		char wasvar=0;
		for(skip(&src,0); (*src && xv>=0); skip(&src,0)) {

			// blank lines
			if(*src == '\n') {
				skip(&src,1);
				continue;
			}

			// commentaries in the source
			if(*src=='\'') {
				char ecf=0; // embed commentary flag
				if(*(src+1)=='!') {
					if(ecf) s2d(*source,destination,token(".comment"));
					ecf=1;
					src++;
				}
				if(*(src+1)=='_') {	// until a _' sequence
					while(*src && (*src!='_' || *(src+1)!='\'')) {
						if(ecf) s2d(*source,destination,(unsigned char)*src);
						src++;
					}
					if(*src=='_' && *(src+1)=='\'') src+=2; else xv=-104;
				} else {	// until the end of the current line
					while(*src>=' ') {
						if(ecf) s2d(*source,destination,(unsigned char)*src);
						src++;
					}
				}
				if(ecf) s2d(*source,destination,0);
				continue;
			}

			// include external source files
			else if(!strncmp("include ",src,8)) {
				skip(&src,8);
				if(*src=='\"') skip(&src,1); else xv=-126;
				while(xv>=0) {
					char *fn=NULL;
					xalloc((unsigned char **)&fn,(MAX_FN_LENGTH+1));
					if(fn) {
						char *pfn=fn;
						while(((pfn-fn)<MAX_FN_LENGTH) && *src && *src!='\"') *(pfn++)=*(src++);
						if(*fn && *src=='\"') {
							FIL *ff=NULL;
							xalloc((unsigned char **)&ff,sizeof(FIL));
							if(ff) {
								FRESULT fe=FR_OK;
								if(fn[0] && fn[1] && fn[2] && fn[3]==':') {
									if(fe==FR_OK) fe=f_chdrive(fn);
									if(fe==FR_OK) fe=f_mount(&FatFs, "", 1);
								}
								if(fe==FR_OK) fe=f_open(ff, fn, (FA_READ | FA_OPEN_EXISTING));
								if(fe==FR_OK) {
									signed int r;
									unsigned long l=f_size(ff);
									if(l>0) {
										f_rewind (ff);
										char *inc=NULL;
										xalloc((unsigned char **)&inc,(l+1));
										if(inc) {
											UINT rr;
											r=(signed int)f_read(ff, inc, l, &rr);
											if(r==FR_OK) {
												*(inc+rr)=0;
												char *incs=inc;
												signed long xv1=compile((char **)&incs,destination,(level+1),scf,dbginfo);
												if(xv1>=0) xv+=xv1; else xv=xv1;
												xfree((unsigned char **)&inc);
											} else xv=-106;
										} else xv=-105;
									}
									f_close(ff);
								}
								else xv=-106;
								xfree((unsigned char **)&ff);
							}
							else xv=-105;
						} else xv=-107;
						skip(&src,1);	// skip the closing "
					} else xv=-105;
					xfree((unsigned char **)&fn);
					if(*src==',') skip(&src,1); else break;
				}
				equside=EQULEFT;
				if(*src==';' || *src<' ') src++; else if(*(src+1)) xv=-100;
				continue;
			}

			// while
			else if((!strncmp("while",src,5) && !strchr(idstr,*(src+5)))) {
				skip(&src,5);
				wh_bgnaddr[wh_index]=(unsigned long)(*destination-dstmem0);
				if(*src==';' || *src<' ') {	// insert '1' into the VM stack if there is no expression after the keyword
					s2d(*source,destination,token(".sint8"));
					s2d(*source,destination,1);
				} else expression(&src,destination,EQURIGHT,&cntbrackets);
				s2d(*source,destination,token(".ifnot"));
				wh_endaddr[wh_index]=(unsigned long)(*destination-dstmem0);
				if(++wh_index>=MAX_NESTED) xv=-108;
				unsigned long a=(unsigned long)(*destination-dstmem0)+4;
				s2d(*source,destination,(unsigned char)a);
				s2d(*source,destination,(unsigned char)(a>>8));
				s2d(*source,destination,(unsigned char)(a>>16));
				s2d(*source,destination,(unsigned char)(a>>24));
				equside=EQULEFT;
				if(*src==';' || *src<' ') src++; else if(*(src+1)) xv=-100;
				continue;
			}

			// exitloop
			else if(!strncmp("exitloop",src,8) && !strchr(idstr,*(src+8))) {
				skip(&src,8);
				if(wh_index) {
					s2d(*source,destination,token(".sint8"));
					s2d(*source,destination,0);		// force false condition in the 'while' statement
					s2d(*source,destination,token(".goto"));
					unsigned long a=wh_endaddr[wh_index-1]-1;
					s2d(*source,destination,(unsigned char)(a));
					s2d(*source,destination,(unsigned char)(a>>8));
					s2d(*source,destination,(unsigned char)(a>>16));
					s2d(*source,destination,(unsigned char)(a>>24));
				} else xv=-109;
				equside=EQULEFT;
				if(*src==';' || *src<' ') src++; else if(*(src+1)) xv=-100;
				continue;
			}

			// repeat
			else if(!strncmp("repeat",src,6) && !strchr(idstr,*(src+6))) {
				skip(&src,6);
				s2d(*source,destination,token(".goto"));
				if(wh_index) {
					s2d(*source,destination,(unsigned char)(wh_bgnaddr[wh_index-1]));
					s2d(*source,destination,(unsigned char)(wh_bgnaddr[wh_index-1]>>8));
					s2d(*source,destination,(unsigned char)(wh_bgnaddr[wh_index-1]>>16));
					s2d(*source,destination,(unsigned char)(wh_bgnaddr[wh_index-1]>>24));
					unsigned long a=(unsigned long)(*destination-dstmem0);
					if(wrdestf) {
						*(dstmem0+wh_endaddr[wh_index-1])=(unsigned char)a;
						*(dstmem0+wh_endaddr[wh_index-1]+1)=(unsigned char)(a>>8);
						*(dstmem0+wh_endaddr[wh_index-1]+2)=(unsigned char)(a>>16);
						*(dstmem0+wh_endaddr[wh_index-1]+3)=(unsigned char)(a>>24);
					}
				} else xv=-109;
				equside=EQULEFT;
				if(*src==';' || *src<' ') src++; else if(*(src+1)) xv=-100;
				continue;
			}

			// until
			else if(!strncmp("until",src,5) && !strchr(idstr,*(src+5))) {
				skip(&src,5);
				if(wh_index) {
					if(*src==';' || *src<' ') {	// insert '0' into the VM stack if there is no expression after 'until'
						s2d(*source,destination,token(".sint8"));
						s2d(*source,destination,0);
					} else expression(&src,destination,EQURIGHT,&cntbrackets);
					wh_index--;
					s2d(*source,destination,token(".ifnot"));
					s2d(*source,destination,(unsigned char)(wh_bgnaddr[wh_index]));
					s2d(*source,destination,(unsigned char)(wh_bgnaddr[wh_index]>>8));
					s2d(*source,destination,(unsigned char)(wh_bgnaddr[wh_index]>>16));
					s2d(*source,destination,(unsigned char)(wh_bgnaddr[wh_index]>>24));
					unsigned long a=(unsigned long)(*destination-dstmem0);
					if(wrdestf) {
						*(dstmem0+wh_endaddr[wh_index])=(unsigned char)a;
						*(dstmem0+wh_endaddr[wh_index]+1)=(unsigned char)(a>>8);
						*(dstmem0+wh_endaddr[wh_index]+2)=(unsigned char)(a>>16);
						*(dstmem0+wh_endaddr[wh_index]+3)=(unsigned char)(a>>24);
					}
				} else xv=-109;
				equside=EQULEFT;
				if(*src==';' || *src<' ') src++; else if(*(src+1)) xv=-100;
				continue;
			}

			// if
			else if((!strncmp("if",src,2) && !strchr(idstr,*(src+2)))) {
				skip(&src,2);
				if(*src==';' || *src<' ') {	// insert '1' into the VM stack if there is no expression after the keyword
					s2d(*source,destination,token(".sint8"));
					s2d(*source,destination,1);
				} else expression(&src,destination,EQURIGHT,&cntbrackets);
				s2d(*source,destination,token(".ifnot"));
				if_nxtaddr[if_index]=(unsigned long)(*destination-dstmem0);
				if_endaddr[if_index]=0;
				if(++if_index>=MAX_NESTED) xv=-108;
				unsigned long a=(unsigned long)(*destination-dstmem0)+4;	// these four bytes will be filled later
				s2d(*source,destination,(unsigned char)a);
				s2d(*source,destination,(unsigned char)(a>>8));
				s2d(*source,destination,(unsigned char)(a>>16));
				s2d(*source,destination,(unsigned char)(a>>24));
				equside=EQULEFT;
				if(*src==';' || *src<' ') src++; else if(*(src+1)) xv=-100;
				continue;
			}

			// else
			else if(!strncmp("else",src,4) && !strchr(idstr,*(src+4))) {
				skip(&src,4);
				if(if_index) {
					unsigned long a;
					if(if_endaddr[if_index-1]) {
						a=(unsigned long)(*destination-dstmem0);
						if(wrdestf) {
							*(dstmem0+if_endaddr[if_index-1])=(unsigned char)a;
							*(dstmem0+if_endaddr[if_index-1]+1)=(unsigned char)(a>>8);
							*(dstmem0+if_endaddr[if_index-1]+2)=(unsigned char)(a>>16);
							*(dstmem0+if_endaddr[if_index-1]+3)=(unsigned char)(a>>24);
						}
					}
					s2d(*source,destination,token(".goto"));
					if_endaddr[if_index-1]=(unsigned long)(*destination-dstmem0);
					a=(unsigned long)(*destination-dstmem0)+4;	// these four bytes will be filled later
					s2d(*source,destination,(unsigned char)a);
					s2d(*source,destination,(unsigned char)(a>>8));
					s2d(*source,destination,(unsigned char)(a>>16));
					s2d(*source,destination,(unsigned char)(a>>24));
					if(wrdestf) {
						*(dstmem0+if_nxtaddr[if_index-1])=(unsigned char)a;
						*(dstmem0+if_nxtaddr[if_index-1]+1)=(unsigned char)(a>>8);
						*(dstmem0+if_nxtaddr[if_index-1]+2)=(unsigned char)(a>>16);
						*(dstmem0+if_nxtaddr[if_index-1]+3)=(unsigned char)(a>>24);
					}
					if(*src==';' || *src<' ') {		// insert '1' into the VM stack if there is no expression after 'else'
						s2d(*source,destination,token(".sint8"));
						s2d(*source,destination,1);
					} else expression(&src,destination,EQURIGHT,&cntbrackets);
					s2d(*source,destination,token(".ifnot"));
					if_nxtaddr[if_index-1]=(unsigned long)(*destination-dstmem0);
					a=(unsigned long)(*destination-dstmem0)+4;	// these four bytes will be filled later
					s2d(*source,destination,(unsigned char)a);
					s2d(*source,destination,(unsigned char)(a>>8));
					s2d(*source,destination,(unsigned char)(a>>16));
					s2d(*source,destination,(unsigned char)(a>>24));
				} else xv=-110;
				equside=EQULEFT;
				if(*src==';' || *src<' ') src++; else if(*(src+1)) xv=-100;
				continue;
			}

			// endif
			else if(!strncmp("endif",src,5) && !strchr(idstr,*(src+5))) {
				skip(&src,5);
				if(if_index) {
					if_index--;
					unsigned long a=(unsigned long)(*destination-dstmem0);
					if(wrdestf) {
						*(dstmem0+if_nxtaddr[if_index])=(unsigned char)a;
						*(dstmem0+if_nxtaddr[if_index]+1)=(unsigned char)(a>>8);
						*(dstmem0+if_nxtaddr[if_index]+2)=(unsigned char)(a>>16);
						*(dstmem0+if_nxtaddr[if_index]+3)=(unsigned char)(a>>24);
						if(if_endaddr[if_index]) {
							*(dstmem0+if_endaddr[if_index])=(unsigned char)a;
							*(dstmem0+if_endaddr[if_index]+1)=(unsigned char)(a>>8);
							*(dstmem0+if_endaddr[if_index]+2)=(unsigned char)(a>>16);
							*(dstmem0+if_endaddr[if_index]+3)=(unsigned char)(a>>24);
						}
					}
				} else xv=-110;
				equside=EQULEFT;
				if(*src==';' || *src<' ') src++; else if(*(src+1)) xv=-100;
				continue;
			}

			// func or label
			else if(!strncmp("func ",src,5) || *src=='!') {
				char label;
				if(*src=='!') {
					src++;	// label must be a single word that starts with '!'
					label=1;
				} else {
					skip(&src,5);
					label=0;
				}
				char *w=word(&src,idstr);
				if(*w==0) xv=-124;
				else if(var(w) || fun(w) || token(w)) xv=-111;
				else {	// valid definition
					if(dbginfo) {
						s2d(*source,destination,token(".comment"));
						if(!label) str2d(*source,destination,"func "); else str2d(*source,destination,"label ");
						str2d(*source,destination,w);
						s2d(*source,destination,0);
					}
					fun_t *p=funs;
					while(p && p->next) p=(fun_t *)p->next;
					fun_t *n=NULL;
					xalloc((unsigned char **)&n,sizeof(fun_t));
					if(n) {
						if(p) p->next=n; else funs=n;
						n->next=NULL;
						n->type=label;
						if(!label) {
							s2d(*source,destination,token(".func"));
							unsigned long a=(unsigned long)(*destination-dstmem0)+4;
							s2d(*source,destination,(unsigned char)a);
							s2d(*source,destination,(unsigned char)(a>>8));
							s2d(*source,destination,(unsigned char)(a>>16));
							s2d(*source,destination,(unsigned char)(a>>24));
							fn_jmpaddr[fn_index]=(a-4);
							if(++fn_index>=MAX_NESTED) xv=-108;
						}
						n->offset=(unsigned long)(*destination-dstmem0);
						strcpy(n->name,w);
						if(!label) {
							src++;
							xv=compile(&src,destination,(level+1),scf,dbginfo);
						}
					} else xv=-105;
				}
				equside=EQULEFT;
				continue;
			}

			// exitfunc or end
			else if(!strncmp("exitfunc",src,8) || (!strncmp("end",src,3) && !strchr(idstr,*(src+3)))) {
				if(!strncmp("exitfunc",src,8)) {
					skip(&src,8);
					if(!level) xv=-113;
				} else {
					skip(&src,3);
					if(level) xv=-113;
				}
				s2d(*source,destination,token(".exit"));
				equside=EQULEFT;
				continue;
			}

			// endfunc
			else if(!strncmp("endfunc",src,7)) {
				skip(&src,7);
				if(level) {
					fun_t *f=funs;
					while(f && f->next) f=(fun_t *)f->next;	// take the most recent function (the one we are currently in)
					signed short t;
					for(t=0; f->type==0 && xv>=0 && t<=(MAX_PARAMS-1); t++) {
						if(f->output[t]) varopr(&src,destination,token(".get"),(f->output[t]->vid));
					}
					s2d(*source,destination,token(".endfunc"));
					if(xv>=0) {
						if(fn_index) {
							fn_index--;
							unsigned long a=(unsigned long)(*destination-dstmem0);
							*(dstmem0+fn_jmpaddr[fn_index])=(unsigned char)(a);
							*(dstmem0+fn_jmpaddr[fn_index]+1)=(unsigned char)(a>>8);
							*(dstmem0+fn_jmpaddr[fn_index]+2)=(unsigned char)(a>>16);
							*(dstmem0+fn_jmpaddr[fn_index]+3)=(unsigned char)(a>>24);
						} else xv=-112;
					}
				} else xv=-112;
				equside=EQULEFT;
				if(*src==';' || *src<' ') src++; else if(*(src+1)) xv=-100;
				break;	// exit the compilation loop
			}

			// new parallel process
			else if(!strncmp("pproc ",src,6)) {
				skip(&src,6);
				char *w=word(&src,idstr);
				if(*w==0) xv=-124;
				else {
					fun_t *f=fun(w);
					if(f) {	// valid definition
						s2d(*source,destination,token(".pproc"));
						s2d(*source,destination,(unsigned char)(f->offset));
						s2d(*source,destination,(unsigned char)((f->offset)>>8));
						s2d(*source,destination,(unsigned char)((f->offset)>>16));
						s2d(*source,destination,(unsigned char)((f->offset)>>24));
					} else xv=-121;
				}
				equside=EQULEFT;
				continue;
			}

			// terminate parallel process
			else if(!strncmp("ppterm ",src,7)) {
				skip(&src,7);
				char *w=word(&src,idstr);
				if(*w==0) xv=-124;
				else {
					fun_t *f=fun(w);
					if(f) {	// valid definition
						s2d(*source,destination,token(".ppterm"));
						s2d(*source,destination,(unsigned char)(f->offset));
						s2d(*source,destination,(unsigned char)((f->offset)>>8));
						s2d(*source,destination,(unsigned char)((f->offset)>>16));
						s2d(*source,destination,(unsigned char)((f->offset)>>24));
					} else xv=-121;
				}
				equside=EQULEFT;
				continue;
			}

			// endunit
			else if(!strncmp("endunit",src,7)) {
				skip(&src,7);
				if(unit.vid) memset(&unit,0,sizeof(var_t)); else xv=-129;
				continue;
			}

			// renvar
			else if(!strncmp("renvar ",src,7)) {
				skip(&src,7);
				char *w=word(&src,idstr);	// variable name
				if(*w==0) xv=-124;
				if(xv<0) break;
				var_t *z=var(w);
				if(z) {
					if(*src==',') {
						skip(&src,1);
						char *w1=word(&src,idstr);	// new name
						if(*w1==0) xv=-124;
						else if(var(w1) || fun(w1) || token(w1)) xv=-111;
						else {	// valid name
							strcpy(wnew,w1);
							strcpy(wbuf,z->name);
							unsigned char diff=(strlen(wnew)-strlen(wbuf));
							var_t *v=vars;
							while(v && xv>=0) {
								if(!memcmp(v->name,wbuf,strlen(wbuf)) && (*(v->name+strlen(wbuf))=='.' || *(v->name+strlen(wbuf))==0)) {
									if((strlen(v->name)+diff)<=MAX_ID_LENGTH) {
										memmove(v->name,(v->name+strlen(wbuf)),(strlen(v->name+strlen(wbuf))+1));
										memmove((v->name+strlen(wnew)),v->name,(strlen(v->name)+1));
										memcpy(v->name,wnew,strlen(wnew));
									} else xv=-131;	// too long name
								}
								v=(var_t *)v->next;
							}
						}
					} else xv=-133;	// expecting ','
				} else xv=-124;		// unable to find the variable
				if(*src==';' || *src<' ') skip(&src,1);
				equside=EQULEFT;
				continue;
			}

			// redim
			else if(!strncmp("redim ",src,6)) {
				skip(&src,6);
				while(xv>=0) {
					char *w=word(&src,idstr);	// variable name
					if(*w==0) xv=-124;
					if(xv<0) break;
					var_t *v=var(w);
					if(v) {
						if(*src=='[') {
							skip(&src,1);
							unsigned char t, dimc=0;
							for(t=1; *src && xv>=0; t++) {
								rdata_t d=expression(&src,destination,EQURIGHT,&cntbrackets);
								if(d.type==rSINT8 || d.type==rSINT16 || d.type==rSINT32 || d.type==rSINT64) {
									if(dimc<MAX_DIMENSIONS) v->dim[dimc++]=d.data.sint64; else xv=-115;
								} else xv=-116;
								if(*src==']') break;
								else if(*src==':') skip(&src,1);
								else if(!dimc) xv=-118;
							}
							if(*src==']') {
								skip(&src,1);
								s2d(*source,destination,token(".vardim"));
								s2d(*source,destination,(unsigned char)(v->vid));
								s2d(*source,destination,(unsigned char)((v->vid)>>8));
								s2d(*source,destination,dimc);
							}
							else xv=-118;
						}
						else xv=-117;
					}
					else xv=-124;	// unable to find the variable
					if(*src==';' || *src<' ' || xv<0) break;
					else if(*src==',') skip(&src,1);
				}
				if(*src==';' || *src<' ') skip(&src,1);
				equside=EQULEFT;
				continue;
			}

			// unit and var
			else if((!strncmp("var ",src,4)) || (!strncmp("unit ",src,5))) {
				char uf=0;
				if(!strncmp("var ",src,4)) skip(&src,4);
				else {
					skip(&src,5);
					uf=1;	// unit flag
				}
				var_t *p0=vars;
				while(p0 && p0->next) p0=(var_t *)p0->next;		// store in (p0) the current last variable address
				unsigned short tkn=0;
				rdata_t x;
				var_t vv;
				memcpy(&vv,&unit,sizeof(var_t));
				if(!uf) {
					char *tn=type(&src,&x);	// get data type name and code
					if(tn) {
						memcpy(&vv.type,&x.type,sizeof(rtype_t));
						var_t *vn=var(tn);
						if(vn) {	// inherit from proto variable (if any)
							vv.protovid=vn->vid;
							if(vn->addr) vv.addr=vn->addr;
							if(vn->maxlen) vv.maxlen=vn->maxlen;
							if(vn->dim[0]) {
								unsigned char t, dc=0;
								for(dc=0; dc<MAX_DIMENSIONS && vv.dim[dc]; dc++);
								for(t=0; t<MAX_DIMENSIONS && dc<MAX_DIMENSIONS && vn->dim[t]; t++) vv.dim[dc++]=vn->dim[t];
							}
						}
						memcpy(&vv.role,&x.role,sizeof(rrole_t));
						strncpy((char *)&vv.name,tn,MAX_ID_LENGTH);
					} else xv=-114;
				} else vv.type=rUNIT;
				vv.vid=0;
				while(xv>=0) {
					char *w=word(&src,idstr);	// variable name
					if(*w==0) xv=-124;
					if((strlen(unit.name)+strlen(w)+1)>MAX_ID_LENGTH) xv=-131;
					if(xv<0) break;
					if(*(unit.name)) {
						memmove((w+strlen(unit.name)+1),w,(strlen(w)+1));	// (w) is in wbuf[] so there should be guaranteed room for this
						strcpy(w,unit.name);
						*(w+strlen(unit.name))='.';
					}
					if(var(w) || fun(w) || token(w)) xv=-111;
					else {	// valid name

						strcpy(vv.name,w);
						var_t *n=newvar(&src, destination, &vv, &tkn, level, dbginfo);
						if(xv>=0) {

							// units are copied in the parental structure
							if(uf) memcpy(&unit,n,sizeof(var_t));

							// more variables to follow
							if(*src==',') {
								skip(&src,1);
								if(tkn && dbginfo) {	// when debug commentaries are enabled every variable is declared on its own
									s2d(*source,destination,0);
									s2d(*source,destination,0);
									tkn=0;
								}
								continue;
							}

							// end of var definition block
							if(*src==';' || *src<' ' || *src=='=' || xv<0) {
								if(tkn) {
									s2d(*source,destination,0);
									s2d(*source,destination,0);
									tkn=0;
								}
								break;
							}

						}
					}
				}

				// assignment block
				if(xv>=0 && *src=='=') {
					skip(&src,1);
					if(dbginfo) {	// when debug commentaries are enabled every variable is declared on its own
						if(tkn) {
							s2d(*source,destination,0);
							s2d(*source,destination,0);
							tkn=0;
						}
					}
					if(!p0) p0=vars; else p0=(var_t *)p0->next;	// (*p0) now points to the first newly defined variable
					vinitlx=0;
					while(p0 && xv>=0 && p0->dim[0]==0) {	// only discrete variables can be initialised in declaration
						if(*src==',') {
							skip(&src,1);
							p0=(var_t *)p0->next;
							vinitl[vinitlx++]=0;
						}
						if(*src!=',') {
							equside=EQURIGHT;	// right side parsing behaviour
							expression(&src,destination,EQURIGHT,&cntbrackets);
							vinitl[vinitlx++]=p0->vid;
						}
						if(*src==',') skip(&src,1);
						p0=(var_t *)p0->next;
					}
					if(xv>=0) {
						if(cntbrackets<0) xv=-123;
						else if(cntbrackets>0) xv=-122;
					}
					vidstkx=0;
					while(vinitlx--) {
						if(vinitl[vinitlx]) {
							s2d(*source,destination,token(".set"));
							s2d(*source,destination,(unsigned char)(vinitl[vinitlx]));
							s2d(*source,destination,(unsigned char)(vinitl[vinitlx]>>8));
						}
					}
					skip(&src,0);
					equside=EQULEFT;
					if(*src!=';' && *src>=' ' && *(src+1)) xv=-100;
				}

				if(*src==';' || *src<' ') skip(&src,1);
				wasvar=1;
				equside=EQULEFT;
				continue;
			}

			// data
			else if(!strncmp("data ",src,5)) {
				skip(&src,5);
				unsigned char addtkn=0;
				signed char dl=0;
				rdata_t x;
				type(&src,&x);
				char *w=word(&src,idstr);
				if(*w==0) xv=-124;
				else if(var(w) || fun(w) || token(w)) xv=-111;
				else {	// valid name
					rtype_t x0=x.type;
					x.type=x0;
					var_t *p=vars;
					while(p && p->next) p=(var_t *)p->next;
					var_t *n=NULL;
					xalloc((unsigned char **)&n,sizeof(var_t));
					if(n) {
						if(p) p->next=n; else vars=n;
						n->next=NULL;
						n->type=x.type;
						n->role=x.role;
						n->dim[0]=1;
						strcpy(n->name,w);
						if(dbginfo) {
							s2d(*source,destination,token(".comment"));
							str2d(*source,destination,"data ");
							str2d(*source,destination,w);
							s2d(*source,destination,0);
						}
						if(++vid==0) vid=1;	// avoid vid 0
						n->vid=vid;	// assign unique id to the variable
						s2d(*source,destination,token(".data"));
						if(x.type==rANY) {
							s2d(*source,destination,token(".any"));
							dl=sizeof(rdata_t);
							addtkn=1;	// add token in front of every constant when the data type is 'any'
						}
						else if(x.type==rSINT8) {
							s2d(*source,destination,token(".sint8"));
							dl=sizeof(signed char);
						}
						else if(x.type==rSINT16) {
							s2d(*source,destination,token(".sint16"));
							dl=sizeof(signed short);
						}
						else if(x.type==rSINT32) {
							s2d(*source,destination,token(".sint32"));
							dl=sizeof(signed long);
						}
						else if(x.type==rSINT64) {
							s2d(*source,destination,token(".sint64"));
							dl=sizeof(signed long long);
						}
						else if(x.type==rREAL) {
							s2d(*source,destination,token(".real"));
							dl=sizeof(double);
						}
						else if(x.type==rTEXT) {
							s2d(*source,destination,token(".text"));
							dl=-1;
						}
						else if(x.type==rFUNC) {
							s2d(*source,destination,token(".fptr"));
							dl=sizeof(unsigned long);
						}
						else if(xv>=0) xv=-114;
						s2d(*source,destination,(unsigned char)(n->vid));
						s2d(*source,destination,(unsigned char)((n->vid)>>8));
					} else xv=-105;
				}
				skip(&src,0);
				if(*src=='=') skip(&src,1); else xv=-127;
				unsigned char *dstcnt=*destination;	// record the address of the <count> field
				unsigned long count=4;
				while(xv>=0 && count--) s2d(*source,destination,0);
				count=0;
				for(skip(&src,0); (*src && xv>=0); skip(&src,0)) {
					rdata_t d=constant(&src,destination,0,addtkn);
					d.role=rrSOURCE;
					if((x.type==rSINT8 || x.type==rSINT16 || x.type==rSINT32 || x.type==rSINT64) &&
						(d.type==rSINT8 || d.type==rSINT16 || d.type==rSINT32 || d.type==rSINT64)) d.type=x.type;
					if(x.type==rREAL && (d.type==rSINT8 || d.type==rSINT16 || d.type==rSINT32 || d.type==rSINT64)) {
						double r=1.0*(double)d.data.sint64;
						d.data.real=r;
						d.type=x.type;
					}
					if(x.type==d.type || (x.type==rANY && d.type!=rINVALID)) {
						if(dl>0) {		// ordinal types
							count++;
							if(x.type!=rANY) {
								memcpy(*destination,(unsigned char *)&d.data,dl);
								(*destination)+=dl;
							} else {
								signed char ddl=0;
								if(d.type==rSINT8) ddl=sizeof(signed char);
								else if(d.type==rSINT16) ddl=sizeof(signed short);
								else if(d.type==rSINT32) ddl=sizeof(signed long);
								else if(d.type==rSINT64) ddl=sizeof(signed long long);
								else if(d.type==rREAL) ddl=sizeof(double);
								else if(d.type==rTEXT) ddl=-1;
								else if(d.type==rFUNC) ddl=sizeof(unsigned long);
								else if(xv>=0) xv=-114;
								if(ddl>0) {
									s2d(*source,destination,(unsigned char)d.type);
									memcpy(*destination,(unsigned char *)&d.data,ddl);
									(*destination)+=ddl;
								}
								else if(ddl<0) {	// TEXT
									if(d.data.text && *(d.data.text) && xv>=0) {
										if(*(d.data.text)=='\"') skip(&d.data.text,1);
										if(*src==',') skip(&src,1);
										else if(*src==';' || *src<' ') {
											skip(&src,1);
											break;
										}
									}
								}
								else xv=-114;
							}
						}
						else if(dl<0) {	// TEXT
							if(d.data.text && *(d.data.text) && xv>=0) {
								count++;
								if(*(d.data.text)=='\"') skip(&d.data.text,1);
								if(*src==',') skip(&src,1);
								else if(*src==';' || *src<' ') {
									skip(&src,1);
									break;
								}
							}
						}
						else xv=-114;
					}
					else if(*src==',') skip(&src,1);
					else if(*src==';' || *src<' ') {
						skip(&src,1);
						break;
					}
					else xv=-126;
				}
				s2d(*source,&dstcnt,(unsigned char)count);	// finally store the counter
				s2d(*source,&dstcnt,(unsigned char)(count>>8));
				s2d(*source,&dstcnt,(unsigned char)(count>>16));
				s2d(*source,&dstcnt,(unsigned char)(count>>24));
				equside=EQULEFT;
				continue;
			}

			// everything else in a statement
			else {
				vidstkx=0;
				memset(vididx,0,xblksize((unsigned char *)vididx));
				equside=EQULEFT;	// initialise the parsing to start from the left side in an equation
				while(xv>=0) {
					expression(&src,destination,equside,&cntbrackets);
					if(*src==',') skip(&src,1);
					else if(*src=='=' && *(src+1)!='=') {
						skip(&src,1);
						equside=EQURIGHT;	// switch to the right side in the equation
					}
					else if(*src==';' || *src<' ') break;
					else if(xv>=0) xv=-101;
				}
				if(xv>=0) {
					while(vidstkx) {
						vidstkx--;
						unsigned short vid=vidstk[vidstkx];
						signed short postmod=vidmod[vidstkx];
						char *s=vididx[vidstkx];
						var_t *v=vars;
						while(v) {
							if(vid!=v->vid) v=(var_t *)v->next; else break;
						}
						if(equside==EQURIGHT) {
							idx(v,&s,destination);
							varopr(&src,destination,token(".set"),vid);
						}
						modopr(&src,destination,v,postmod);	// post-modifier
					}
					if(*src) {
						if(*src==';' || *src<' ') src++;
						else if(*(src+1)) xv=-100;
					}
					if(!wasvar) s2d(*source,destination,token(".clear"));
					wasvar=0;
				}
				if(xv>=0) {
					if(cntbrackets<0) xv=-123;
					else if(cntbrackets>0) xv=-122;
				}
			}

		}

		// free up all locally allocated memory when exiting the current level
		fun_t *f;
		if(funs0) {
			f=(fun_t *)funs0->next;
			funs0->next=NULL;
		} else {
			f=funs;
			funs=NULL;
		}
		while(f) {
			fun_t *p=(fun_t *)f->next;
			xfree((unsigned char **)&f);
			f=p;
		}
		var_t *v;
		if(vars0) {
			v=(var_t *)vars0->next;
			vars0->next=NULL;
		} else {
			v=vars;
			vars=NULL;
		}
		while(v) {
			var_t *p=(var_t *)v->next;
			xfree((unsigned char **)&v);
			v=p;
		}

	}
	else xv=-105;	// unable to allocate memory for the arrays

	// release the memory used by the arrays
	xfree((unsigned char **)&wh_bgnaddr);
	xfree((unsigned char **)&wh_endaddr);
	xfree((unsigned char **)&if_nxtaddr);
	xfree((unsigned char **)&if_endaddr);

	// update the pointers and exit the current level
	if(xv>=0) {
		*source=src;
		if(!level) {
			if(wrdestf) {
				if(*(*destination-1)==token(".clear")) (*destination)--;
				s2d(*source,destination,token("print"));
			}
			s2d(*source,destination,token(".exit"));
		}
		xv=(*destination-dstmem0);
	} else *source=rscsrc;

	// finally release the memory allocated for static arrays
	if(level==0) {
		xfree((unsigned char **)&vidstk);
		xfree((unsigned char **)&vidmod);
		xfree((unsigned char **)&vinitl);
		xfree((unsigned char **)&vididx);
		xfree((unsigned char **)&fn_jmpaddr);
	}
	return xv;
}
