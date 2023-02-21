#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "rittle.h"

#define iscmd(x,pcmd) (!strncmp(x,pcmd,strlen(x)) && (*(pcmd+strlen(x))==0 || strchr(" \r\n",*(pcmd+strlen(x)))))

unsigned char editff=1;

// forward declarations
void run(char *fname, signed char errf);
void fferr(int fe);


// RITTLE CONSOLE =============================================================

// single step for the main environment loop
// (editf) directs whether to read line by using the line_edit() function, or
//			text has bee already pre-loaded into the console buffer
void rittle_main_step(char editf) {
	init_warm();
	if(editf && editff) {
		memset(conbuf,0,sizeof(conbuf));
		f_getcwd(conbuf,(sizeof(conbuf)-strlen(PROMPT)-1));
		strcat(conbuf,PROMPT);
		line_edit(conbuf, conbuf, (sizeof(conbuf)-2), 0);
	}
	xv=0;
	editff=1;
	char *pcmd=conbuf;
	skip(&pcmd,0);

	// ride [<file>]
	if(iscmd("ride",pcmd)) {
		skip(&pcmd,4);
		ride(pcmd);
		return;
	}

	// dir [<path>]
	else if(iscmd("dir",pcmd)) {
		skip(&pcmd,3);
		if(*pcmd==0) {
			pcmd=conbuf;
			f_getcwd(pcmd,sizeof(conbuf)-1);
			strcat(pcmd,"\n");
		}
		DIR *dir=NULL;
		FILINFO *finfo=NULL;
		xalloc((unsigned char **)&dir,sizeof(DIR));
		xalloc((unsigned char **)&finfo,sizeof(FILINFO));
		if(!dir || !finfo) {
			xfree((unsigned char **)&finfo);
			xfree((unsigned char **)&dir);
			return;
		}
		unsigned long long p1=0;
		unsigned long s1,s2;
		s1=s2=0;
		printf("\r\n");
		FRESULT r=FR_OK;
		if(pcmd[0] && pcmd[1] && pcmd[2] && pcmd[3]==':') {
			if(r==FR_OK) r=f_chdrive(pcmd);
			if(r==FR_OK) r=f_mount(&FatFs, "", 1);
		}
		if(r==FR_OK) r=f_opendir(dir,pcmd);
		if(r==FR_OK) printf("directory %s\r\n",pcmd);
		for(; r==FR_OK; ) {
			if(r==FR_OK) r=f_readdir(dir,finfo);
			if(r!=FR_OK || !finfo->fname[0]) break;
			if(finfo->fattrib & AM_DIR) {
				s2++;
			} else {
				s1++;
				p1+=finfo->fsize;
			}
			printf("%c%c%c%c%c %4u/%02u/%02u %02u:%02u %10lu %c %-12s\r\n",
					(finfo->fattrib & AM_DIR) ? 'D' : '-',
					(finfo->fattrib & AM_RDO) ? 'R' : '-',
					(finfo->fattrib & AM_HID) ? 'H' : '-',
					(finfo->fattrib & AM_SYS) ? 'S' : '-',
					(finfo->fattrib & AM_ARC) ? 'A' : '-',
					(finfo->fdate >> 9) + 1980, (finfo->fdate >> 5) & 15, finfo->fdate & 31,
					(finfo->ftime >> 11), (finfo->ftime >> 5) & 63, finfo->fsize,
					(finfo->fattrib & AM_DIR) ? '>' : ' ',
					finfo->fname);
		}
		f_closedir(dir);
		printf("\r\n%4u file(s), %12llu bytes total\r\n%4u dir(s)",s1,p1,s2);
		FATFS *fs=&FatFs;
		if(f_getfree(pcmd,(DWORD*)&p1, &fs)==FR_OK) {
			printf(",  %12llu bytes free\r\n",(p1 * fs->csize *
				#if FF_MIN_SS!=FF_MAX_SS
					fs->ssize
				#else
					FF_MAX_SS
				#endif
			));
		}
		printf("\r\n");
		xfree((unsigned char **)&finfo);
		xfree((unsigned char **)&dir);
		fferr(r);
		return;
	}

	else if(iscmd("run",pcmd)) {
		skip(&pcmd,3);
		run(pcmd,1);
		return;
	}

	else if(iscmd("list",pcmd)) {
		skip(&pcmd,4);
		run(pcmd,-1);
		return;
	}

	// mount <device>
	else if(iscmd("mount",pcmd) || (strlen(pcmd)==4 && pcmd[3]==':')) {
		if(strlen(pcmd)>4) skip(&pcmd,5);
		FRESULT fe=f_chdrive(pcmd);
		if(fe==FR_OK) fe=f_mount(&FatFs,pcmd,1);
		fferr(fe);
		return;
	}

	// init <device>
	else if(iscmd("init",pcmd)) {
		skip(&pcmd,4);
		FRESULT fe=f_chdrive(pcmd);
		if(fe==FR_OK) fe=f_mount(&FatFs,"",1);
		if(fe!=FR_OK && fe!=FR_NO_FILESYSTEM) {
			fferr(fe);
			return;
		}
		if(fe==FR_OK) {
			f_getcwd(conbuf,sizeof(conbuf)-1);
			pcmd=conbuf;
			printf("WARNING: this operation will destroy ALL data on drive %s\r\nconfirm initialisation (y/n): ",pcmd);
			int k=getch();
			printf("\r\n");
			if(k!='y' && k!='Y') return;	// ignore initialisation
		}
		signed long long sz=ffinit(pcmd);
		if(sz>0) printf(">>> initialised size %llu bytes\r\n\n",sz);
		else fferr(-sz);
		return;
	}

	// mkdir <dir>
	else if(iscmd("mkdir",pcmd)) {
		skip(&pcmd,5);
		fferr(f_mkdir(pcmd));
		return;
	}

	// rmdir <dir>
	else if(iscmd("rmdir",pcmd)) {
		skip(&pcmd,5);
		fferr(f_rmdir(pcmd));
		return;
	}

	// chdir <dir>
	else if(iscmd("chdir",pcmd)) {
		skip(&pcmd,5);
		fferr(f_chdir(pcmd));
		return;
	}

	// delete <file>
	else if(iscmd("delete",pcmd)) {
		skip(&pcmd,6);
		fferr(f_unlink(pcmd));
		return;
	}

	// rename <file> , <file>
	else if(iscmd("rename",pcmd)) {
		skip(&pcmd,6);
		char *cf=pcmd;
		char *nf=pcmd;
		while(*nf && *nf>' ' && *nf!=',') nf++;
		if(*nf!=' ' && *nf!=',') {  // error
			fferr(FR_INVALID_PARAMETER);
			return;
		}
		if(*nf!=',') {
			*(nf++)=0;  // this marks the end of the 'current' file name
			while(*nf && *nf!=',') nf++;
			if(*(nf++)!=',') {  // error
				fferr(FR_INVALID_PARAMETER);
				return;
			}
		}
		else *(nf++)=0;
		while(*nf && *nf==' ') nf++;
		if(*nf<=' ' || *nf==',') {  // error
			fferr(FR_INVALID_PARAMETER);
			return;
		}
		fferr(f_rename(cf,nf));
		return;
	}

	// copy <file> , <path\file>
	else if(iscmd("copy",pcmd)) {
		skip(&pcmd,4);
		char *cf=pcmd;
		char *nf=pcmd;
		while(*nf && *nf>' ' && *nf!=',') nf++;
		if(*nf!=' ' && *nf!=',') {  // error
			fferr(FR_INVALID_PARAMETER);
			return;
		}
		if(*nf!=',') {
			*(nf++)=0;  // this marks the end of the 'current' file name
			while(*nf && *nf!=',') nf++;
			if(*(nf++)!=',') {  // error
				fferr(FR_INVALID_PARAMETER);
				return;
			}
		}
		else *(nf++)=0;
		while(*nf && *nf==' ') nf++;
		if(*nf<=' ' || *nf==',') {  // error
			fferr(FR_INVALID_PARAMETER);
			return;
		}
		fferr(-fcopy(cf,nf));
		return;
	}

	// try to execute Rittle words in direct mode
	unsigned char *pfake=fake;
	xv=compile(&pcmd,&pfake,0,1,0);	// speculative compilation to estimate the code length
	if(xv>5) {	// successfully compiled (the minimum meaningful code is 5 bytes)
		unsigned char *code=NULL;
		xalloc((unsigned char **)&code,(xv+sizeof(fake)));
		if(code) {
			char *pcmd=conbuf;
			unsigned char *pcode=code;
			xv=compile(&pcmd,(unsigned char **)&pcode,0,0,0);	// actual compilation
			if(xv>5) {
				signed long xclen=xv;
				// list(0,(unsigned char *)code,xclen);	// (used for debugging only!)
				initRVM((unsigned char *)code,xclen,NULL);
				xv=execute((unsigned char *)code,xclen,0);
				xfree((unsigned char **)&code);
				if(xv==3) return;	// keep the content in conbuf[]
			}
		} else xv=-7;
		printf("\r\n");
		if(xv<0 || xv>5) xvline(NULL);	// exit code after execution
	}
	else if(xv<0) xvline(pcmd);	// compilation error
	*conbuf=0;
}


// execute .RXE or .SYS file
// note: the file must be already open externally
// (errf) enables or disables the error message if the file doesn't exist
//		  value -1 will cause only listing the file
void run(char *fname, char signed errf) {
	if(!fname || *fname==0) return;
	FIL *fr=NULL;
	xalloc((unsigned char **)&fr,sizeof(FIL));
	if(!fr) {
		printf(">>> insufficient memory\r\n");
		return;
	}

	FRESULT fe=FR_OK;
	if(fname[0] && fname[1] && fname[2] && fname[3]==':') {
		if(fe==FR_OK) fe=f_chdrive(fname);
		if(fe==FR_OK) fe=f_mount(&FatFs, "", 1);
	}
	if(fe==FR_OK) fe=f_open(fr, fname, (FA_READ | FA_OPEN_EXISTING));
	if(fe==FR_OK) {
		unsigned long runl=(unsigned long)f_size(fr);
		if(runl) {
			unsigned char *runf=NULL;
			xalloc((unsigned char **)&runf,(runl+1));
			unsigned long r=0;
			f_rewind(fr);
			fe=f_read(fr, runf, runl, (UINT *)&r);
			if(fe==FR_OK && runl==r) {
				if(errf>=0) {
					initRVM((unsigned char *)runf,runl,NULL);
					xv=execute((unsigned char *)runf,runl,0);
				}
				else {
					printf("\r\n");
					xv=-2;		// force entering the .sys branch
				}
				if(xv==-2) {	// not executable so we assume it is a .sys file then
					xv=0;
					runl=0;
					char *cb, *cs=(char *)runf;
					while(*cs) {
						runl++;
						memset(conbuf,0,sizeof(conbuf));
						cb=conbuf;
						while(*cs>=' ') *(cb++)=*(cs++);
						while(*cs && *cs<' ') cs++;
						if(errf>=0) {	// execute
							if(*conbuf) rittle_main_step(0);
							if(xv<0) {
								printf(">>> line %lu\r\n",runl);
								xv=0;	// a message has been printed already
								break;
							}
						}
						else {	// just list
							printf("%3lu: %s\r\n",runl,conbuf);
							if(xv<0) break;
						}
					}
				}
				xfree((unsigned char **)&runf);
				if(xv!=3) printf("\r\n"); else editff=0;	// conbuf[] will be preserved for automatic execution
				if(xv<0 || xv>5) xvline(NULL);	// exit code after execution
			}
			else printf(">>> error reading \"%s\"\r\n",fname);
			xfree((unsigned char **)&runf);
		}
	}
	else if(errf) printf(">>> invalid file name \"%s\"\r\n",fname);
	f_close(fr);
	xfree((unsigned char **)&fr);
}


// main entry to the Rittle console and user interface
void rittle(void) {
	xv=0;
	fname=NULL;
	text=NULL;
    printf("\r\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("Rittle v%s [%s]\r\n\n", SW_VERSION, HW_PLATFORM);
	if(f_mount(&FatFs,"ifs:",1)!=FR_OK) {   // start in the internal flash drive
        printf("initialising IFS: ... ");
        signed long long sz=ffinit("ifs:");
        if(sz>0) printf("%llu bytes\r\n\n",sz);
        else fferr(-sz);
    }
	// fferr(f_chdrive("ifs:"));

	// execute config.sys
    run("ifs:\\config.sys",0);

	// main environment loop
	while(1) {
		enbrk=0;
		rittle_main_step(1);
        if(xv==-22) break;  // system reset
	}
	xfree((unsigned char **)&text);
	xfree((unsigned char **)&fname);
	f_mount(0, "", 0);
}


// ERROR CODES AND INFORMATION ================================================

// show error line and error code text
void xvline(char *source) {
	if(xv<0) {	// error
		printf(">>> ");
		if(xv>-1000) printf("exit code %li: ",xv);
		switch(xv) {
			default: break;

			// EXECUTION EXIT CODES (from RVM)
			// ===============================
			// 3:    chain running files
			// 2:    execution stopped at a breakpoint
			// 1:    normal execution end
			// 0:    no error
			case -1: 	printf("invalid token code"); break;
			case -2:	printf("not executable"); break;
			case -3:	printf("argument out of range"); break;
			case -4:	printf("data type mismatch"); break;
			case -5:	printf("invalid reference"); break;
			case -6:	printf("division by zero"); break;
			case -7:	printf("unable to allocate enough memory"); break;
			case -8:	printf("invalid dimension"); break;
			case -9:	printf("maximum level of nesting reached"); break;
			case -10:	printf("result overflow"); break;
			case -11:	printf("invalid format specifier"); break;
			case -12:	printf("execution stopped by the user"); break;
			case -13:	printf("invalid text constant"); break;
			case -14:	printf("unable to complete device operation"); break;
			case -15:	printf("no available file handlers"); break;
			case -16:	printf("invalid file handler"); break;
			case -17:	printf("invalid variable id"); break;
			case -18:	printf("maximum number of parallel threads reached"); break;
			case -19:	printf("invalid access rights"); break;
			case -20:	printf("invalid reference to data"); break;
            case -21:	printf("not supported in the current platform"); break;
            case -22:	printf("system reset"); break;
			case -23:	printf("invalid parameter"); break;
			case -24:	printf("insufficient number of parameters"); break;

			// COMPILATION EXIT CODES (from RSC)
			// =================================
			// 0 and greater: success; contains the byte size of the output code
			case -100:	printf("missing ';'"); break;
			case -101:	printf("invalid character in source"); break;
			case -102:	printf("invalid text constant"); break;
			case -103:	printf("invalid numeric constant"); break;
			case -104:	printf("unfinished comment"); break;
			case -105:	printf("unable to allocate memory"); break;
			case -106:	printf("unable to open include file"); break;
			case -107:	printf("invalid include file name"); break;
			case -108:	printf("maximum level of nesting reached"); break;
			case -109:	printf("missing opening 'while'"); break;
			case -110:	printf("missing opening 'if'"); break;
			case -111:	printf("redefined identifier"); break;
			case -112:	printf("'endfunc' without 'func'"); break;
			case -113:	printf("mismatched exit"); break;
			case -114:	printf("data type expected"); break;
			case -115:	printf("too many dimensions"); break;
			case -116:	printf("invalid dimension"); break;
			case -117:	printf("'[' expected"); break;
			case -118:	printf("']' expected"); break;
			case -119:	printf("execution not possible"); break;
			case -120:	printf("variable expected"); break;
			case -121:	printf("undefined function"); break;
			case -122:	printf("missing closing ')'"); break;
			case -123:	printf("missing opening '('"); break;
			case -124:	printf("invalid identifier"); break;
			case -125:	printf("valid for text variables only"); break;
			case -126:	printf("invalid constant"); break;
			case -127:	printf("missing '='"); break;
			case -128:	printf("unspecified internal error"); break;
			case -129:	printf("invalid 'endunit'"); break;
			case -130:	printf("unit nesting"); break;
			case -131:	printf("identifier name too long"); break;
			case -132:	printf("complex input/output variable"); break;
			case -133:	printf("',' expected"); break;

			// DEVELOPMENT EXIT CODES (from RIDE)
			// ==================================
			case -1000:	printf("unable to allocate memory"); break;
			case -1001:	printf("start line required"); break;
			case -1002:	printf("count required"); break;
			case -1003:	printf("unrecognised command"); break;
			case -1004:	printf("parameter required"); break;
			case -1005:	printf("compilation needed"); break;
			case -1006:	printf("file name required"); break;
			case -1007:	printf("invalid variable id"); break;
			case -1008:	printf("variable index out of range"); break;
			case -1009:	printf("unable to open file"); break;
            case -1010:	printf("unable to write file"); break;

		}

		// source position in compilation errors
		if(source && rscline>0) {
			printf("\r\n>>> source line %lu\r\n>>> ",rscline);
			while(*source>=' ' || *source=='\t') {
				if(*source>=' ') printf("%c",*source);
				else if(*source=='\t') printf(TAB);
				source++;
			}
			printf("\r\n");
		}

		// RVM information in execution errors
		else if(xv>-100) {
			printf("\r\n>>> thread %hu (%hu)",thd,threads);
			printf("\r\n>>> entry: 0x%08lx    dsp: 0x%04hx",rvm[thd]->entry,rvm[thd]->dsp);
			printf("\r\n>>>    pc: 0x%08lx    csp: 0x%04hx",rvm[thd]->pc,rvm[thd]->csp);
			printf("\r\n");
		}

	}
	else if(xv>0) {	// successful compilation
		printf("lines compiled: %lu\r\n",rscline);
		printf("code length: %li bytes\r\n",(xv+1));
	}
	printf("\r\n");
}


// when executing in direct mode print errors from FatFs
// (fe) error code
void fferr(int fe) {
    if(fe==FR_OK) return;
    printf(">>> drive error %i: ",fe);
    switch(fe) {
        default: break;
        /* (-1) */ case FR_DISK_ERR: printf("unrecoverable error occurred in the media access layer"); break;
        /* (-2) */ case FR_INT_ERR: printf("internal error in the drive access functions"); break;
        /* (-3) */ case FR_NOT_READY: printf("device is not ready"); break;
        /* (-4) */ case FR_NO_FILE: printf("file is not found"); break;
        /* (-5) */ case FR_NO_PATH: printf("path is not found"); break;
        /* (-6) */ case FR_INVALID_NAME: printf("invalid filename"); break;
        /* (-7) */ case FR_DENIED: printf("access denied"); break;
        /* (-8) */ case FR_EXIST: printf("file already exists"); break;
        /* (-9) */ case FR_INVALID_OBJECT: printf("invalid file/directory structure"); break;
        /* (-10)*/ case FR_WRITE_PROTECTED: printf("device is write-protected"); break;
        /* (-11)*/ case FR_INVALID_DRIVE: printf("invalid drive"); break;
        /* (-12)*/ case FR_NOT_ENABLED: printf("device is not mounted"); break;
        /* (-13)*/ case FR_NO_FILESYSTEM: printf("no valid file system on the device"); break;
        /* (-14)*/ case FR_MKFS_ABORTED: printf("function aborted"); break;
        /* (-15)*/ case FR_TIMEOUT: printf("timeout error"); break;
        /* (-16)*/ case FR_LOCKED: printf("object is locked for processing"); break;
        /* (-17)*/ case FR_NOT_ENOUGH_CORE: printf("not enough memory for file functions"); break;
        /* (-18)*/ case FR_TOO_MANY_OPEN_FILES: printf("too many open files"); break;
        /* (-19)*/ case FR_INVALID_PARAMETER: printf("invalid parameter in the file functions"); break;
    }
    printf("\r\n\n");
}


// initialise storage drive and return error code (negative) or size in bytes (positive)
// (*dev) device identifier
signed long long ffinit(char *dev) {
    signed long long res=-FR_MKFS_ABORTED;
    if(!dev || *dev==0) return res;
    unsigned char *wbuf=NULL;
    xalloc((unsigned char **)&wbuf,FF_MAX_SS);
    MKFS_PARM ftype;
    memset(&ftype, 0, sizeof(ftype));
    ftype.fmt=(FM_ANY | FM_SFD);
    if(wbuf) {
        signed int r=(signed int)f_mkfs(dev, &ftype, wbuf, FF_MAX_SS);
        xfree((unsigned char **)&wbuf);
        if((FRESULT)r==FR_OK) {	// get drive free space after initialisation
            unsigned long long cl=0;
            FATFS *fs=&FatFs;
            r=(signed int)f_getfree(dev, (DWORD *)&cl, &fs);
            if((FRESULT)r==FR_OK) {
                res=(signed long long)(cl * fs->csize *
                    #if FF_MIN_SS!=FF_MAX_SS
                        fs->ssize
                    #else
                        FF_MAX_SS
                    #endif
                );
            }
            else res=-((signed long long)r);
        }
        else res=-((signed long long)r);
    } else res=-7;
    return res;
}


// INTERNAL TOP-LEVEL FUNCTIONS AND VARIABLES =================================

// skip whitespace characters in source
// (preinc) specifies the minimum number of characters that definitely need to be skipped
void skip(char **source, signed long preinc) {
	if(xv>=0) {
		char *src=*source;
		while(*src && preinc--) src++;
		while(*src && xv>=0) {
			if(strchr(" \t\v\r\n",*src)) {
				if(*src=='\n') {
					rscline++;
					rscsrc=(src+1);
					break;
				}
				else src++;
			}
			else {
				if(*src<' ') xv=-101;
				break;
			}
		}
		*source=src;
	}
}


// return token code by given its text name, or return 0 if not found
unsigned char token(char *s) {
	unsigned short t;
	for(t=0; t<0x100; t++) if(!strcmp(s,tokens[t].name)) return (unsigned char)t;
	return 0;
}


// determine the best integer data type based on the value of (v)
void cast(void *v, rdata_t *c) {
	memcpy(&(c->data.sint64),v,tokens[rSINT64].inc);
	if(c->data.sint64<LONG_MIN || c->data.sint64>LONG_MAX) c->type=rSINT64;
	else if(c->data.sint64<SHRT_MIN || c->data.sint64>SHRT_MAX) c->type=rSINT32;
	else if(c->data.sint64<SCHAR_MIN || c->data.sint64>SCHAR_MAX) c->type=rSINT16;
	else c->type=rSINT8;
}


// a backup plan since the function round() is not available in the XC32 compiler
double xround(double v) {
    // return round(v);
    double vf=floor(v);
    if((v-vf)>=0.5) vf+=1.0;
    return (double)vf;
}


// return a multi-byte integer value from the memory
signed long long getival(unsigned char *memaddr, unsigned char len) {
    signed long long v=0;
    memcpy((unsigned char *)&v,memaddr,len);
    return v;
}


// return a floating point value from the memory
double getrval(unsigned char *memaddr, unsigned char len) {
    double r;
    memcpy((unsigned char *)&r,memaddr,len);
    return r;
}


// text line editor
// (*buf) text buffer
// (bufsz) maximum size of the text buffer including the terminating 0 character
// (x) initial position of the cursor (starting from 0)
// output
// (*buf) text after editing
void line_edit(char *prompt, char *buf, unsigned int bufsz, int x) {
    xdefrag();  // a good place for defragmenting the memory since user input is to be expected anyway
    if(!buf) return;
    if(!bufsz) {
        *buf=0;
        return;
    }
	if(strlen(buf)>bufsz) return;
	if(strlen(buf)<bufsz) memset(&buf[strlen(buf)],0,(bufsz-strlen(buf)));
	// good idea to reduce the clock when it is not needed but unfortunately
	// affects the work of the f*****g USB and it stops
	// unsigned long cfreq=get_clock();
	// set_clock(16);	// there is no need for higher clock here
#ifndef USE_FGETS
	if(prompt!=buf) printf("%s%s",prompt,buf);
	else {	// if the prompt was supplied in the buffer it is safe to assume that the buffer starts empty
		printf("%s",prompt);
		memset(buf,0,bufsz);
	}
    if(x<0) x=0;
    if(x>strlen(buf)) x=strlen(buf);
    int t=strlen(buf);
    while(t-->x) printf("\b");
    for(;;) {
        int ch=getch();
		if(ch==0) ch=KEY_ESC;
		if(ch==KEY_ESC) {
			ch=(ch<<8)+getch();	// two-byte extended keyboard code has been received
			if(ch==ESC_SEQ) {
				ch=(ch<<8)+getch();	// three-byte sequences
				if((ch & 0xff)>='1' && (ch & 0xff)<='9') ch=(ch<<8)+getch();	// four-byte sequences
			}
		}
		if(ch==KEY_BREAK && enbrk) {		// Ctrl-C
			while(kbhit()) getch(); 		// clear the console buffer
			xv=-12;
			return;
		}
		else if(ch==KEY_ENTER || ch=='\n') {	// [Enter]
			if(x) {
				while(x<bufsz) {	// clear all characters after the current position
					if(buf[x]) {
						buf[x]=0;
						printf(" ");
					}
					x++;
				}
			}
            printf("%c\n",KEY_ENTER);
            break;
        }
        else if(ch==KEY_BACKSPC || ch==KEY_BACKSPC_ALT || ch==KEY_DEL || ch==KEY_DEL_ALT) {	// [BckSpc] or [Del]
			if((ch==KEY_BACKSPC || ch==KEY_BACKSPC_ALT) && x<1) continue;	// backspace in the first position will do nothing
			t=1;
			if(ch==KEY_BACKSPC || ch==KEY_BACKSPC_ALT) {
				if(x>2 && !strncmp(&buf[x-3],TAB,strlen(TAB))) t=strlen(TAB);
				int z=t;
				while(z--) {
					printf("\b");
					x--;
				}
			}
			else {
				if(!strncmp(&buf[x],TAB,strlen(TAB))) t=strlen(TAB);
			}
			if((strlen(buf)+t)>=bufsz) t=bufsz-strlen(buf);
			memmove(&buf[x],&buf[x+t],(strlen(buf)-x-t+1));
			printf("%s",&buf[x]);
			int z=t;
			while(z--) printf(" ");
			t+=strlen(buf);
			while(t-->x) printf("\b");
		}
		else if(ch==KEY_LEFT || ch==KEY_LEFT_ALT) {		// left arrow
			if(x) {
				printf("\b");
				x--;
			}
		}
		else if(ch==KEY_RIGHT || ch==KEY_RIGHT_ALT) {	// right arrow
			if(x<strlen(buf)) printf("%c",buf[x++]);
		}
		else if(ch==KEY_HOME || ch==KEY_HOME_ALT) {		// [Home] key will move the cursor to the beginning of the line
			t=x;
			while(t--) printf("\b");
			x=0;
		}
		else if(ch==KEY_END || ch==KEY_END_ALT) {		// [End] key will move the cursor to the end of the line
			t=x;
			while(t--) printf("\b");
			printf("%s",buf);
			x=strlen(buf);
		}
		else if((ch>=' ' && ch<0x100) || ch==KEY_TAB) {	// characters and [Tab]
            t=1;
            if(ch==KEY_TAB) {   // [Tab] key will produce strlen(TAB) number of space characters
                ch=' ';
                t=strlen(TAB);
            }
			if((strlen(buf)+t)>=bufsz) {
				t=bufsz-strlen(buf)-1;
				printf("\a");	// alarm when the buffer limit has been reached
			}
			if(t>0) {
				memmove(&buf[x+t],&buf[x],(strlen(buf)-x+1));
				memset(&buf[x],(char)ch,t);
				printf("%s",&buf[x]);
				x+=t;
				t=strlen(buf);
				while(t-->x) printf("\b");
			}
        }
    }
#else   // USE_FGETS
	printf("%s",prompt);
	fgets(buf,bufsz-1,stdin);   // this is the alternative way of doing the user input; a bit limited but sometimes might be of help
	if(*buf && buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]=0;  // remove the closing LF from the input string
#endif  // USE_FGETS
	// set_clock(cfreq);	// restore the original clock
}


// RITTLE DISASSEMBLER ========================================================

#define DATADIV1 "DATA BEGIN *******************************************************************"
#define DATADIV2 "DATA END *********************************************************************"
#define FUNCDIV1 "FUNC BEGIN -------------------------------------------------------------------"
#define FUNCDIV2 "FUNC END ---------------------------------------------------------------------"

// base address for listing as received in the list() parameters
unsigned long baseaddr=0;


// print bytes from specified address
void tdump(unsigned char *begin, unsigned char *current, unsigned long len, const char *mnem) {
	printf("%06lx   ",(unsigned long)(current-begin)+baseaddr);
	signed char t=9;
	while(len--) {
		printf("%02hhx ",*(current++));
		if(--t<1 && len) {
			printf("\r\n         ");
			t=9;
		}
	}
	while(t--) printf("   ");
	printf("   ");
	if(mnem) {
		printf("%s",mnem);
		t=11-strlen(mnem);
	} else t=11;
	while(t--) printf(" ");
}


// print data types
unsigned char *dtypes(unsigned char *c0, unsigned char *code, unsigned char *fixed) {
	signed char r;
	unsigned char *code1=code;
	unsigned char l=1;
	if(fixed) {
		code1=fixed;
		l=0;
	}
	if(*code1==token(".sint8")) {
		r=sizeof(signed char);
		tdump(c0,code,r+l,tokens[*code1].name);
		printf("%hhi",*((signed char *)(code+l)));
		code+=(r+l);
	}
	else if(*code1==token(".sint16")) {
		r=sizeof(signed short);
		tdump(c0,code,r+l,tokens[*code1].name);
		printf("%hi",(signed short)getival((code+1),sizeof(signed short)));
		code+=(r+l);
	}
	else if(*code1==token(".sint32")) {
		r=sizeof(signed long);
		tdump(c0,code,r+l,tokens[*code1].name);
		printf("%li",(signed long)getival((code+1),sizeof(signed long)));
		code+=(r+l);
	}
	else if(*code1==token(".sint64")) {
		r=sizeof(signed long long);
		tdump(c0,code,r+l,tokens[*code1].name);
		printf("%lli",(signed long long)getival((code+1),sizeof(signed long long)));
		code+=(r+l);
	}
	else if(*code1==token(".real")) {
		r=sizeof(double);
		tdump(c0,code,r+l,tokens[*code1].name);
		printf("%G",getrval((code+1),sizeof(double)));
		code+=(r+l);
	}
	else if(*code1==token(".fptr")) {
		r=4;	// these tokens use fixed 4 bytes argument
		tdump(c0,code,r+l,tokens[*code1].name);
		printf("(addr 0x%08lx)",(unsigned long)getival((code+1),sizeof(unsigned long)));
		code+=(r+l);
	}
	else if(*code1==token(".text")) {
		unsigned long tl=1+l;
		unsigned char *ct=code+l;
		while(*(ct++)) tl++;
		tdump(c0,code,tl,tokens[*code1].name);
		code+=l;
		printf("\"");
		while(*code) {
			if(*code>=' ') printf("%c",*(code++)); else printf("_%02hhx",*(code++));
		}
		printf("\"");
		code++;
	}
	return(code);
}


// list compiled code
// (addr) is a virtual address from where to start then counting
void list(unsigned long addr, unsigned char *code, unsigned long length) {
	baseaddr=addr;
	unsigned char *c0=code;
	while((code-c0)<length) {
		signed char r;
		unsigned char *cc=code;
		code=dtypes(c0,code,NULL);
		if(code!=cc) {
			printf("\r\n");
			continue;
		}
		if(*code==token(".goto") || *code==token(".call") || *code==token(".reffn") ||
			*code==token(".pproc") || *code==token(".ppterm")) {
			r=4;	// these tokens use fixed 4 bytes argument
			tdump(c0,code,r+1,tokens[*code].name);
			printf("(addr 0x%08lx)",(unsigned long)getival((code+1),sizeof(unsigned long)));
			code+=(r+1);
		}
		else if(*code==token(".func") || *code==token(".ifnot")) {
			if(*code==token(".func")) printf("\r\n%s\r\n",FUNCDIV1);
			r=4;	// fixed 4 bytes argument
			tdump(c0,code,r+1,tokens[*code].name);
			printf("(jump 0x%08lx)",(unsigned long)getival((code+1),sizeof(unsigned long)));
			code+=(r+1);
		}
		else if(*code==token(".refer")) {	// special case
			r=2;	// fixed 4 bytes argument but only the first two bytes are listed
			tdump(c0,code,r+1,tokens[*code].name);
			printf("V%hu",(unsigned short)getival((code+1),sizeof(unsigned short)));
			code+=(2+(r+1));
		}
		else if(*code==token(".var") || *code==token(".varin") || *code==token(".varout") || *code==token(".varref")) {
			unsigned long tl=0;
            while((unsigned short)getival((code+2+tl),sizeof(unsigned short))) tl+=2;
			tdump(c0,code,(tl+4),tokens[*code].name);
			code++;
			printf("%-9s",tokens[*code].name);
			code++;
			while(tl) {
				printf("V%hu",(unsigned short)getival(code,sizeof(unsigned short)));
				code+=2;
				tl-=2;
				if(tl) printf(",  ");
			}
			code+=2;	// skip the trailing VID <0000>
		}
		else if(*code==token(".reset")) {
			r=4;
			tdump(c0,code,r,tokens[*code].name);
			code++;
            while(--r) printf("%c",*(code++));
		}
		else if(*code==token(".vafix") || *code==token(".index") || 	*code==token(".maxlen") ||
				*code==token(".set") || *code==token(".get")) {
			tdump(c0,code,3,tokens[*code].name);
			printf("V%hu",(unsigned short)getival((code+1),sizeof(unsigned short)));
			code+=3;
		}
		else if(*code==token(".vardim")) {
			tdump(c0,code,4,tokens[*code].name);
			printf("V%hu  array [",(unsigned short)getival((code+1),sizeof(unsigned short)));
			r=*(code+3);
			while(r--) {
				printf("n");
				if(r) printf(", ");
			}
			printf("]");
			code+=4;
		}
		else if(*code==token(".data")) {
			unsigned char *pdt=code+1;
			unsigned short vid=(unsigned short)getival((code+2),sizeof(unsigned short));
			unsigned long cnt=(unsigned long)getival((code+4),sizeof(unsigned long));
			tdump(c0,code,8,tokens[*code].name);
			code+=8;
			printf("%-9s V%hu  (num %lu)\r\n\n%s\r\n",tokens[*pdt].name,vid,cnt,DATADIV1);
			while(cnt--) {
				if((*pdt)!=rANY) {	// single type data
					unsigned char *cc=code;
					code=dtypes(c0,code,pdt);
					if(code==cc) break;	// unknown data type?
				}
				else code=dtypes(c0,code,NULL);	// mixed data
				printf("\r\n");
			}
			printf("%s\r\n",DATADIV2);
		}
        else if(*code==token(".comment")) {
            unsigned char tc=*(code);
            printf("%06lx   %02hhx ... 00   %s   ",(unsigned long)((code-c0)+baseaddr),tc,tokens[tc].name);
            code++;
            while(*code) {
                if(*code>=' ') printf("%c",*code); else printf("_%02hhx",*code);
                code++;
            }
            code++;
	    }
		else {
			tdump(c0,code,1,tokens[*code].name);
			if(*code==token(".endfunc")) printf("\r\n%s\r\n",FUNCDIV2);
			code++;
		}
		printf("\r\n");
	}
}
