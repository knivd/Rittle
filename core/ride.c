#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ride.h"

unsigned long line=0;		// edit line number
unsigned char undop=0;		// pointer to the current undo operation
unsigned long undol[UNDO_DEPTH]={0};	// line number to perform an undo operation
char *undos[UNDO_DEPTH];	// strings for undo operation
char *cbsave;				// save the command buffer for repeat operations
char *finds;				// string for the find functions
char *repls;				// string for the replace operations
unsigned char *code=NULL;	// compiled code
signed long xcodelen=0;		// length of the compiled code
FIL *ff=NULL;   			// internal file object for load and save operations
char pmts[10];				// prompt string


// free all allocated RIDE blocks
// (freetext) specifies whether the text allocation needs to be freed as well
void freeall(char freetext) {
	for(undop=UNDO_DEPTH; undop; ) xfree((unsigned char **)&undos[--undop]);	// this will leave (undop) in 0
	xfree((unsigned char **)&code);
	xfree((unsigned char **)&finds);
	xfree((unsigned char **)&repls);
	xfree((unsigned char **)&cbsave);
	if(freetext) xfree((unsigned char **)&text);
}


// go to line with specified number
// also modifies (line)
char *gotoline(char *s, unsigned long ln) {
    line=0;
    while(s && *s && ln) {
        if(*s=='\n') {
            line++;
            ln--;
        }
        s++;
    }
    while(s && *s && *s<' ' && *s!='\n') s++;
    return(s);
}


// measure string length to NULL or to xterm inclusive
unsigned long strlenn(char *str, char xterm) {
    unsigned long r=0;
    while(str && *str) {
        r++;
        if(*str!=xterm) str++; else break;
    }
    return(r);
}


// print the current text line
void printline(unsigned long ln, char *s) {
    if(!s) return;
    printf("%3lu: ",(ln+1));
    while(*s && *s!='\n') {
        if(*s>=' ') printf("%c",*s);
        else if(*s=='\t') printf(TAB);
        s++;
    }
}


// add new string to the text and resize the buffer if necessary
// inserting from position (*s)
// adding NULL will only delete the line pointed by (*s)
// (xterm) is an extra termination character to consider when measuring string lengths to add
//         a special case is (xterm)==-1 to instruct not to add (adds) to the undo stack
void addremoveline(char *s, char *adds, signed char xterm) {
    if(xv<0) return;
    xfree((unsigned char **)&code);
    unsigned long lt;
    if(s && text && s>=text) {
        char *snext=s;
        while(*snext && *snext!='\n') snext++;
        if(*snext) snext++;
        while(*snext && *snext<' ' && *snext!='\n') snext++;
        if(s!=snext) {
            if(undop>=UNDO_DEPTH) { // manage the undo stack
                xfree((unsigned char **)&undos[0]);
                for(undop=0; undop<(UNDO_DEPTH-1); undop++) {
                    undos[undop]=undos[undop+1];
                    undol[undop]=undol[undop+1];
                }
                undop=UNDO_DEPTH-1;
                undos[undop]=NULL;
                undol[undop]=0;
            }
            if(adds && xterm>=0) {  // add to the undo stack
                xalloc((unsigned char **)&undos[undop],(snext-s)+1);
                if(undos[undop]) {
                    memcpy(undos[undop],s,(snext-s));
                    undos[undop][snext-s]=0;
                    undol[undop]=0;
                    char *st=text;
                    while(st<s) if(*(st++)=='\n') undol[undop]++;
                    undop++;
                }
            }
            memmove(s,snext,strlen(snext)+1);
            if(adds>=snext && adds<(snext+strlen(snext))) adds-=(snext-s);
        }
    }
    if(s<text) s=text;  // using this to trick the function to avoid deleting the first line when inserting
    if(!adds || *adds==0) return;
    if(xterm<0) xterm=0;
    unsigned long la=strlenn(adds,xterm);
    lt=1+strlenn(adds,xterm);
    if(!text) { // allocate new
        xalloc((unsigned char **)&text,lt);
        if(text) *text=0;
        s=text;
    }
    else {  // change size (originally with realloc())
        lt+=strlen(text);
		char *t0=text;
        xalloc((unsigned char **)&text,lt);
		s+=(text-t0);	// recalculate the (s) pointer after a possible data relocation
    }
    if(!text) xv=-1000;
    if(!xv && s && la) {
        lt=strlen(s)+1;
        memmove((s+la),s,lt);
        if(adds>=s && adds<(s+lt)) adds+=la;
        memmove(s,adds,la);
    }
}


// get a numeric parameter from text and return pointer in the text after the number
// will return 0 on an invalid number
unsigned long getnp(char **s) {
    char *s1=*s;
    unsigned long v=0;
    while(s1 && *s1>='0' && *s1<='9') {
        v=(v*10)+(*s1-'0');
        s1++;
    }
    *s=s1;
    return(v);
}


// save binary file from (*code)
// length is in (xcodelen)
// (fn) is the file name
void savebinfile(char *fn) {
	if(fn && *fn) {
        UINT written=0;
		if(code) {
			FRESULT fr=FR_OK;
			if(fn[0] && fn[1] && fn[2] && fn[3]==':') {
				if(fr==FR_OK) fr=f_chdrive(fn);
				if(fr==FR_OK) fr=f_mount(&FatFs, "", 1);
			}
			if(fr==FR_OK) fr=f_open(ff, fn, (FA_WRITE | FA_CREATE_ALWAYS));
			if(fr==FR_OK) fr=f_write(ff, code, xcodelen, &written);
			f_close(ff);
            if(fr!=FR_OK) xv=-1010;
		}
		printf("\r\n>>> %lu bytes written\r\n\n",(unsigned long)written);
	} else xv=-1006;
}


// save text file from (*text)
void savetextfile(char *fn) {
	if(fn && *fn) {
        UINT written=0;
		if(text) {
			FRESULT fr=FR_OK;
			if(fn[0] && fn[1] && fn[2] && fn[3]==':') {
				if(fr==FR_OK) fr=f_chdrive(fn);
				if(fr==FR_OK) fr=f_mount(&FatFs, "", 1);
			}
            if(fr==FR_OK) fr=f_open(ff, fn, (FA_WRITE | FA_CREATE_ALWAYS));
            if(fr==FR_OK) fr=f_write(ff, text, (strlen(text)+1), &written);
			f_close(ff);
            if(fr!=FR_OK) xv=-1010;
		}
		printf("\r\n>>> %lu bytes written\r\n\n",(unsigned long)written);
	} else xv=-1006;
}


// open text file into (*text)
// (fn) is the file name
// returns 1 on successful operation or 0 otherwise
char opentextfile(char *fn) {
	UINT read=0;
	if(fn && *fn) {
		FRESULT fr=FR_OK;
		if(fn[0] && fn[1] && fn[2] && fn[3]==':') {
			if(fr==FR_OK) fr=f_chdrive(fn);
			if(fr==FR_OK) fr=f_mount(&FatFs, "", 1);
		}
		if(fr==FR_OK) fr=f_open(ff, fn, (FA_READ | FA_OPEN_EXISTING));
		if(fr==FR_OK) {
			unsigned long fs=(unsigned long)f_size(ff);
			xalloc((unsigned char **)&text,(fs+1));
			if(text) {
				f_read(ff, text, fs, &read);
				if(read>0) printf("\r\n>>> %lu bytes read\r\n\n",(unsigned long)read);
			}
			else xv=-1000;
		}
		else xv=-1009;
		f_close(ff);
	} else xv=-1006;
	return !!read;
}


// breakpoint handler
void debug(void) {
    printf("\r\n>>> breakpoint\r\n");
}


// inspect the data in stack
void insp_stack(void) {
	if(rvm[thd]->dsp) {
		if(threads) printf(">>> thread %hhu",(thd+1));
		signed short t;
		for(t=(rvm[thd]->dsp-1); t>=0; t--) {
			if(rvm[thd]->stack[t].type!=rINVALID) {
				printf("\r\n]%3i: ",(t-rvm[thd]->dsp)+1);
				if(rvm[thd]->stack[t].type==rSINT8) printf("%hi",(signed char)rvm[thd]->stack[t].data.sint64);
				else if(rvm[thd]->stack[t].type==rSINT16) printf("%i",(signed short)rvm[thd]->stack[t].data.sint64);
				else if(rvm[thd]->stack[t].type==rSINT32) printf("%li",(signed long)rvm[thd]->stack[t].data.sint64);
				else if(rvm[thd]->stack[t].type==rSINT64) printf("%lli",(signed long long)rvm[thd]->stack[t].data.sint64);
				else if(rvm[thd]->stack[t].type==rREAL) printf("%G",(double)(rvm[thd]->stack[t].data.real));
				else if(rvm[thd]->stack[t].type==rTEXT) printf("`%s`",(char *)rvm[thd]->stack[t].data.text);
			}
		}
		printf("\r\n");
	}
}


// integrated development environment
void ride(char *param) {
    startnew:   // return point when opening a new file
    xv=0;
    for(undop=0; undop<UNDO_DEPTH; undop++) {
        undol[undop]=0;
        undos[undop]=NULL;
    }
    undop=0;
    code=NULL;
    finds=NULL;
    repls=NULL;
    cbsave=NULL;
    if(!ff) {
        xalloc((unsigned char **)&ff,sizeof(FIL));
        if(!ff) {
            printf("\r\n>>> can't allocate enough memory for file operations");
            freeall(1); // release all memory
            return;
        }
    }
    if(param && *param) {
		FRESULT r=FR_OK;
		if(param[0] && param[1] && param[2] && param[3]==':') {
			if(r==FR_OK) r=f_chdrive(param);
			if(r==FR_OK) r=f_mount(&FatFs, "", 1);
		}
		if(r==FR_OK) {
			r=f_open(ff, param, (FA_READ | FA_OPEN_EXISTING));
			f_close(ff);
		}
		if(r==FR_OK) {
			printf("opening %s\r\n\n",param);
			opentextfile(param);
		}
		else printf("new file %s\r\n\n",param);
		xfree((unsigned char **)&fname);
		xalloc((unsigned char **)&fname,(strlen(param)+1));
		if(fname) strcpy(fname,param);
    }
    gotoline(text,ULONG_MAX);   // count the number of lines in the text
    memset(conbuf,0,sizeof(conbuf));
    char qflag=0;
	while(!qflag) {
		enbrk=0;
        xv=0;
        memset(conbuf,0,sizeof(conbuf));
        printf("     ");
        unsigned long lt=(line+1);
        while(lt>=1000) {   // compensate for longer line numbers
            printf(" ");
            lt/=10;
        }
        char *pcmd=gotoline(text,line);
        while(pcmd && *pcmd && (*pcmd>=' ' || *pcmd=='\t')) {
            if(*pcmd>=' ') {
                printf("%c",*pcmd);
                conbuf[strlen(conbuf)]=*pcmd;
            }
            else if(*pcmd=='\t') {
                printf(TAB);
                conbuf[strlen(conbuf)]=' ';
                conbuf[strlen(conbuf)]=' ';
            }
            pcmd++;
        }
        sprintf(pmts,"\r%3lu: ",((line+1)%1000000));
		line_edit(pmts, conbuf, (sizeof(conbuf)-2), 0);
		if(*conbuf==0) {
			line++;
			continue;
		}
        if(*conbuf && conbuf[strlen(conbuf)-1]!='\n') {
			conbuf[strlen(conbuf)+1]=0;
			conbuf[strlen(conbuf)]='\n';
		}
		xv=0;
		pcmd=conbuf;
        char listf=0;   // compile listing flag
		char dbgif=0;	// debug information flag

        // RIDE command string
        if(*pcmd=='.') {
            skip(&pcmd,1);
			if(*pcmd==0) {	// a single dot will go to the last source line
				gotoline(text,ULONG_MAX);
				continue;
			}
            char first=1;   // flag raised only during the first iteration (in case there are many)
            signed long repeat=-1;
            while(pcmd && *pcmd && !xv && !qflag && (repeat<0 || repeat--)) {

				if(*pcmd == '\n') {
					skip(&pcmd,1);
					continue;
				}

                skip(&pcmd,0);
                if(*pcmd==0) break;

                // exit
                else if(*pcmd=='_') qflag=1;

                // help
                else if((*pcmd | 0x20)=='h') {
                    skip(&pcmd,1);
                    printf("\r\nA single dot is enough for full command line with multiple commands\r\n");
                    printf("._   exit (text remains in the memory)          .H   this information\r\n");
                    printf(".Z   clear screen (250 new lines)               .?   other useful information\r\n");
                    printf(".U   undo the last edited line                  .    jump to last source line\r\n");
					printf(".[J] [line] or <N> or <P>     jump to line or to next line or to previous line\r\n");
					printf(".*   [number of times]   repeat the following command line number of times\r\n");
					printf(".O   <NEW> or <file.RIT> start a new blank file or open a file with given name\r\n");
                    printf(".S   [file.RIT]  save file (optionally can save with a new file name)\r\n");
					printf(".F   [text to EOL]       define 'Find' string to find or perform find function\r\n");
                    printf(".R   [text to EOL]       define 'Replace' string or perform find and replace\r\n");
                    printf(".L   [number of lines] [,] [starting from line number]   list (recent or from)\r\n");
                    printf(".D   <number of lines> [,] [starting from line number]   delete lines\r\n");
                    printf(".I   <number of lines> [,] [starting from line number]   insert lines\r\n");
                    printf(".C   <number of lines> [,] [starting from line number]   copy lines at current\r\n");
                    printf(".M   <number of lines> [,] [starting from line number]   move lines to current\r\n");
                    printf("\r\nRittle Compilation and Debug:\r\n");
                    printf("..   first enter step mode then execute single instruction in current process\r\n");
                    printf(".=   first enter step mode then execute single instruction in all processes\r\n");
                    printf(".#   <file.RXE>  create executable file for RVM\r\n");
                    printf(".V   <variable id> [, <linear index>]   inspect variable\r\n");
                    printf(".\\   inspect RVM stack for current process      .[   compile in mem [opts $%%]\r\n");
                    printf(".B   place/remove breakpoint                    .>   run or continue after BP\r\n");
                }

                // status
                else if(*pcmd=='?') {
                    skip(&pcmd,1);
                    printf("\r\nUse .H for commands help\r\n\r\n");
					printf("free memory: %lu bytes\r\n",xtotal());
                    printf("file: ");
					if(fname) printf("%s",fname);
					unsigned long ln=line;
					unsigned long lt=0;
					if(text) lt=strlen(text);
					gotoline(text,ULONG_MAX);
					printf("\r\n%lu lines, %lu bytes\r\n\n",line,lt);
					gotoline(text,ln);
                    if(finds) printf("find string: %s\r\n",finds);
                    if(repls) printf("replace string: %s\r\n",repls);
                    unsigned short t;
                    for(t=0; t<undop; t++) {
                        if(undos[t]) {
                            printf("(undo %hhi) %3lu: ",(t-undop),(undol[t]+1));
                            char *s=undos[t];
                            while(s && (*s>=' ' || *s=='\t')) printf("%c",*(s++));
                            printf("\r\n");
                        }
                    }
                }

                // repeat
                else if(*pcmd=='*') {
                    skip(&pcmd,1);
                    repeat=getnp(&pcmd);
                    xalloc((unsigned char **)&cbsave,strlen(pcmd)+1);
                    if(cbsave) {
						strcpy(cbsave,".");
						strcat(cbsave,pcmd);
					}
                    continue;
                }

                // clear screen
                else if((*pcmd | 0x20)=='z') {
                    skip(&pcmd,1);
                    unsigned short t;
                    for(t=0; t<250; t++) printf("\r\n");
                }

                // undo
                else if((*pcmd | 0x20)=='u') {
                    skip(&pcmd,1);
                    if(!xv && undop) {
                        undop--;
                        char *z=gotoline(text,undol[undop]);
                        addremoveline(z,undos[undop],-1);
                        gotoline(text,line);
                    }
                }

                // save
                else if((*pcmd | 0x20)=='s') {
                    skip(&pcmd,1);
                    unsigned long l=strlenn(pcmd,'\n');
                    if(l && *(pcmd+l-1)=='\n') *(pcmd+l-1)=0;
					if(*pcmd==0 && (!fname || *fname==0)) xv=-1006;
					else if(*pcmd && *pcmd!='.') {
                        xfree((unsigned char **)&fname);
                        xalloc((unsigned char **)&fname,strlen(pcmd)+1);
                        if(fname) strcpy(fname,pcmd); else xv=-1000;
                    }
                    if(!xv) {
                        printf("saving to %s",fname);
                        savetextfile(fname);
                        printf("\r\n");
                    }
                    pcmd=NULL;
                    if(repeat>0) repeat=0;
                }

                // open, new
                else if((*pcmd | 0x20)=='o') {
                    skip(&pcmd,1);
                    unsigned long l=strlenn(pcmd,'\n');
                    if(l && *(pcmd+l-1)=='\n') *(pcmd+l-1)=0;
                    if(*pcmd) {
                        freeall(1);
                        param=pcmd;
                        if(strcmp(param,"NEW") && strcmp(param,"New") && strcmp(param,"new")) {
							xfree((unsigned char **)&fname);
							xalloc((unsigned char **)&fname,(strlen(param)+1));
							if(fname) strcpy(fname,param);
                        }
                        else {
                            xfree((unsigned char **)&fname);
                            *param=0;
                        }
                        if(!xv) goto startnew;
                    } else xv=-1004;
                    pcmd=NULL;
                    if(repeat>0) repeat=0;
                }

                // jump to line
                else if((*pcmd | 0x20)=='j' || (*pcmd | 0x20)=='n' || (*pcmd | 0x20)=='p' ||
                        (*pcmd>='0' && *pcmd<='9')) {
                    char opr=(*pcmd | 0x20);
                    if(*pcmd>='0' && *pcmd<='9') skip(&pcmd,0); else skip(&pcmd,1);
                    if(opr=='j' && (*pcmd=='n' || *pcmd=='p')) {
                        opr=(*pcmd | 0x20);
                        skip(&pcmd,1);
                    }
                    if(opr=='n') gotoline(text,++line);
                    else if(opr=='p') {
                        if(line) gotoline(text,--line);
                    }
                    else {
                        unsigned long l=getnp(&pcmd);
                        if(l) {
                            l--;
                            gotoline(text,l);   // go to valid line number
                        } else gotoline(text,ULONG_MAX);    // go to the end of the file
                    }
                }

                // list lines
                else if((*pcmd | 0x20)=='l') {
                    skip(&pcmd,1);
                    unsigned long lorg=line;
                    unsigned long lbeg=0;
                    unsigned long lc=getnp(&pcmd);     // get the number of lines to print
                    skip(&pcmd,0);
                    if(*pcmd==',') {
                        skip(&pcmd,1);
                        gotoline(text,ULONG_MAX);       // count the number of lines in the text
                        lt=getnp(&pcmd); // get the start line number
                        if(lt && lt<=line) lbeg=(lt-1);
                    }
                    if(!xv) {
                        if(lc && !lbeg) {
                            if(lc>line) lc=line;
                            lbeg=line-lc;
                        }
                        else if(!lc) lc=line-lbeg;
                        char *cl=gotoline(text,lbeg);
                        if(cl && *cl && lc) printf("\r\n");
                        while(cl && *cl && lc--) {
                            printline(lbeg++,cl);
                            cl=gotoline(cl,1);
                            printf("\r\n");
                        }
                    }
                    line=lorg;
                }

                // delete, insert, copy, move
                else if(((*pcmd | 0x20)=='d') || ((*pcmd | 0x20)=='i') ||
                        ((*pcmd | 0x20)=='c') || ((*pcmd | 0x20)=='m')) {
                    char ifunc;
                    if((*pcmd | 0x20)=='m') ifunc=3;        // move
                    else if((*pcmd | 0x20)=='c') ifunc=2;   // copy
                    else if((*pcmd | 0x20)=='i') ifunc=1;   // insert
                    else ifunc=0;   // delete
                    skip(&pcmd,1);
                    unsigned long lorg=line;
                    unsigned long lbegm=line, lbeg=line;
					unsigned long lc=getnp(&pcmd);     // get the number of lines
                    unsigned long lcc=lc, lcc1=lc;
					unsigned long lt=line;
                    if(!lc) xv=-1002; else skip(&pcmd,0);
                    // if(!xv && *pcmd!=',' && (ifunc==2 || ifunc==3)) xv=-1001;
                    if(!xv && *pcmd==',') {
                        skip(&pcmd,1);
                        gotoline(text,ULONG_MAX);	// count the number of lines in the text
                        lt=getnp(&pcmd);			// get the start line number
                        if(lt && lt<=line) lbeg=(lt-1);
                    }
                    if(!xv) {
                        char *cl=text;
                        if(ifunc) {
                            if(ifunc==2 || ifunc==3) {
                                lt=lbeg;
                                lbeg=lorg;
                            }
                            if(!lbeg) {
                                if(text) cl=text-1;     // tricking the addremovelines() function
                            }
                            else {
                                cl=gotoline(text,(lbeg-1));
                                while(cl && (*cl>=' ' || *cl=='\t')) cl++;
                            }
                            if(ifunc==2 || ifunc==3) lbeg=lt;
                        } else cl=gotoline(text,lbeg);
                        lcc1=lcc=lc;    // store (lc) for potentially reusing it later
                        while(lc--) {
                            if(ifunc) {
                                if(cl<text) addremoveline(cl,(char *)"\n",0); else addremoveline(cl,(char *)"\n\n",0);
                                if(cl) cl++;
                            }
                            else if(cl && *cl) {
                                addremoveline(cl,NULL,0);
                            }
                        }
                    }
                    gotoline(text,ULONG_MAX);   // count the number of lines in the text
                    if(lorg<=line) line=lorg;
                    if(!xv && (ifunc==2 || ifunc==3)) { // copy, move
                        lc=lorg=line;
                        if(lbeg>=line) lbeg+=lcc;
                        lbegm=lbeg; // store (lbeg) for move operation
                        while(lcc--) {
                            char *zf=gotoline(text,lbeg++);
                            char *zt=gotoline(text,lc++);
                            addremoveline(zt,zf,'\n');
                        }
                        gotoline(text,ULONG_MAX);   // count the number of lines in the text
                        line=lorg;
                    }
                    if(!xv && ifunc==3) {   // move
                        lorg=line;
                        char *cl=gotoline(text,lbegm);
                        while(lcc1--) addremoveline(cl,NULL,0);
                        gotoline(text,ULONG_MAX);   // count the number of lines in the text
                        line=lorg;
                    }
					if(!xv) {
						if(ifunc==1 && lt && lbeg!=line) lt--;
						gotoline(text,lt);
					}
                }

                // find, replace
                else if((*pcmd | 0x20)=='f' || (*pcmd | 0x20)=='r') {
                    char **y;
                    char cmd=(*pcmd | 0x20);
                    if(cmd=='f') y=&finds; else y=&repls;
                    skip(&pcmd,1);
                    signed long l=strlenn(pcmd,'\n');
                    if(l && *(pcmd+l-1)=='\n') l--;
                    if(l && first) {    // define
                        xalloc((unsigned char **)y,l+1);
                        if(*y) memcpy(*y,pcmd,l); else xv=-1000;
                    }
                    char *s=gotoline(text,line);
                    char *z=s;
                    if(!xv && (cmd=='f' || cmd=='r')) { // find or replace
                        if(finds && *finds && strlen(s)>=strlen(finds)) {
                            unsigned long l=strlen(s)-strlen(finds);
                            while(!xv && s && *s && *finds) {
                                if(s<=(z+l) && memcmp(s,finds,strlen(finds))) s++; else break;
                            }
                            z=s;
                            if(z && *z && !memcmp(z,finds,strlen(finds))) {
                                line=0;
                                while(z>text) {
                                    if(*(z--)=='\n') line++;
                                }
                            } else s=gotoline(text,ULONG_MAX);
                        }
                    }
                    if(!xv && cmd=='r') {   // replace only - inheriting (s) from find
                        if(finds && *finds && repls && s && *s) {
                            z=s;
                            char *zf=s;
                            char *zt=conbuf;
                            memset(conbuf,0,sizeof(conbuf));
                            while((zt-conbuf)<(sizeof(conbuf)-2) && (*zf>=' ' || *zf=='\t')) *(zt++)=*(zf++);
                            s=strstr(conbuf,finds);
                            if(s && *s) {
                                signed long diff=strlen(repls)-strlen(finds);
                                if((strlen(conbuf)+diff+1)<sizeof(conbuf)) {
                                    if((s+diff)>=s) {
                                        l=(sizeof(conbuf)-diff-1);
                                        if(l>0) memmove((s+diff),s,l);
                                    }
                                    else {
                                        l=(sizeof(conbuf)+diff-1);
                                        if(l>0) memmove(s,(s-diff),l);
                                    }
                                    memmove(s,repls,strlen(repls));
                                    s[strlen(s)+1]=0;
                                    s[strlen(s)]='\n';
                                    addremoveline(z,conbuf,0);
                                    line=0;
                                    while(z>text) {
                                        if(*(z--)=='\n') line++;
                                    }
                                }
                            }
                        }
                    }
                    pcmd=NULL;
                }

                // commands related to RITTLE ====================================================

                // create executable file
                else if(*pcmd=='#') {
                    skip(&pcmd,1);
                    if(code) {
                        char *ptext=text;
                        unsigned char *pcode=code;
                        xv=compile((char **)&ptext,(unsigned char **)&pcode,0,0,0);	// compilation without debug info
						if(xv>0) {
							xcodelen=xv;
                        	initRVM((unsigned char *)code,xcodelen,(void *(*)())debug);
							unsigned long l=strlenn(pcmd,'\n');
							if(l && *(pcmd+l-1)=='\n') *(pcmd+l-1)=0;
							if(*pcmd && *pcmd!='.') {
								char *pc=pcmd;
								while(*pc) {
									if(*pc<' ') *pc=0;
									else if(*pc>='A' && *pc<='Z') *(pc)|=0x20;  // convert name to lower case and cut-off non-printable characters
									pc++;
								}
								while(*(pcmd+strlen(pcmd)-1)=='.') *(pcmd+strlen(pcmd)-1)=0;
								if(strlen(pcmd)<4 || (strlen(pcmd)>=4 && strcmp((pcmd+strlen(pcmd)-4),".rxe"))) strcat(pcmd,".rxe");
								printf("saving to %s",pcmd);
								savebinfile(pcmd);
								printf("\r\n");
							} else xv=-1006;
						}
                        pcmd=NULL;
                    } else xv=-1005;
                    if(repeat>0) repeat=0;
                }

                // place/remove breakpoint
                else if((*pcmd | 0x20)=='b') {
                    skip(&pcmd,1);
                    unsigned long lorg=line;
                    char *cl=gotoline(text,line);
                    if(cl && memcmp(cl,"break;\n",7)) { // add breakpoint
                        if(!line) {
                            if(text) cl=text-1; // tricking the addremovelines() function
                        }
                        else {
                            cl=gotoline(text,(line-1));
                            while(cl && (*cl>=' ' || *cl=='\t')) cl++;
                        }
                        if(cl<text) addremoveline(cl,(char *)"\n",0); else addremoveline(cl,(char *)"\n\n",0);
                        if(cl) cl++;
                        addremoveline(cl,(char *)"break;\n",0);
                    }
                    else addremoveline(cl,NULL,0);  // remove breakpoint
                    line=lorg;
                    if(repeat>0) repeat=0;
                }

                // enable/disable compilation listing (for this command line only)
                else if(*pcmd=='%') {
                    skip(&pcmd,1);
                    listf=1-listf;
                }

				// enable/disable inclusion of debug commentaries into output code (for this command line only)
                else if(*pcmd=='$') {
                    skip(&pcmd,1);
                    dbgif=1-dbgif;
                }

                // compile
                else if(*pcmd=='[') {
                    skip(&pcmd,1);
                    if(text) {
                        unsigned char *pfake=fake;
                        char *ptext=text;
                        xv=compile((char **)&ptext,(unsigned char **)&pfake,0,1,dbgif);	// speculative compilation to estimate the code length
                        xvline(ptext);
                        if(xv>5 || (xv>0 && listf)) {       // successfully compiled
                            xfree((unsigned char **)&code);
                            xalloc((unsigned char **)&code,(xv+sizeof(fake)));
                            if(!code) xv=-1000;
                            if(xv>5 || (xv>0 && listf)) {
                                unsigned char *pcode=code;
                                ptext=text;
                                xv=compile((char **)&ptext,(unsigned char **)&pcode,0,0,dbgif);	// actual compilation
								if(xv>0) {
									xcodelen=xv;
                                	if(listf) list(0,(unsigned char *)code,xcodelen);
                                	initRVM((unsigned char *)code,xcodelen,(void *(*)())debug);
								}
                            }
                            xvline(NULL);	// exit code after execution
                        }
                        else if(xv<0) pcmd=NULL;
                        xv=0;
                    }
                    if(repeat>0) repeat=0;
                }

                // run or instruction step
                else if(*pcmd=='.' || *pcmd=='=' || *pcmd=='>') {
                    if((*pcmd=='.' && *(pcmd-1)=='.') || *pcmd=='=' || *pcmd=='>') {
                        while(pcmd && ((*pcmd=='.' && *(pcmd-1)=='.') || *pcmd=='=' || *pcmd=='>')) {
                            char xmode=1;
                            if(*pcmd=='=') xmode=2;
                            else if(*pcmd=='.') xmode=3;
                            pcmd++;
                            if(text) {
                                if(code) {
                                    gotoline(text,ULONG_MAX);
                                    if(repeat>=0) {
                                        repeat++;
                                        while(!xv && repeat--)
											xv=execute((unsigned char *)code,xcodelen,xmode);
                                    } else xv=execute((unsigned char *)code,xcodelen,xmode);
                                    if(!xv || xv==2) {
										insp_stack();
                                        if(xmode==2 || xmode==3) {
                                            unsigned char *m=(unsigned char *)mem;
                                            unsigned short t;
                                            printf("\r\n");
                                            if(xmode==2) {      // batch thread step execution
                                                for(t=0; t<=threads; t++) {
                                                    if(threads) printf(">>> thread %hhu\r\n",(t+1));
                                                    list(rvm[t]->pc,(m+rvm[t]->pc),1);
                                                }
                                            }
                                            else if(xmode==3) { // single step execution
                                                if(threads) printf(">>> thread %hhu\r\n",(thd+1));
                                                list(rvm[thd]->pc,(m+rvm[thd]->pc),1);
                                            }
                                        }
                                    }
                                    else {  // end execution
                                        xfree((unsigned char **)&code);
                                        if(xv==1) printf("\r\n>>> ok"); else pcmd=NULL;
										printf("\r\n");
                                    }
                                } else xv=-1005;
                            }
                        }
                    } else skip(&pcmd,1);
                }

                // stack data
                else if(*pcmd=='\\') {
                    skip(&pcmd,1);
                    if(code) insp_stack(); else xv=-1005;
                    if(repeat>0) repeat=0;
                }

                // inspect variable
                else if((*pcmd | 0x20)=='v') {
                    skip(&pcmd,1);
                    if(code) {
                        unsigned long index=0;
                        unsigned short vid=getnp(&pcmd);   // get variable id (number)
                        skip(&pcmd,0);
                        if(*pcmd==',') {
                            skip(&pcmd,1);
                            index=getnp(&pcmd);
                        }
                        vmvar_t *v=rvm[thd]->vmv;
                        while(v && v->vid!=vid) v=(vmvar_t *)v->next;  // find vid
                        if(v) {
                            unsigned long maxindex=1;
                            signed char dl=0, t=0;
                            for(t=0; t<MAX_DIMENSIONS; t++) if(v->dim[t]) maxindex*=v->dim[t];
                            if(index<maxindex) {
                                printf("[");
                                for(t=0; t<MAX_DIMENSIONS; t++) {
                                    if(v->dim[t]>0) {
                                        if(t) printf(":");
                                        printf("%lu",v->dim[t]);
                                    }
                                }
                                printf("]   ");
                                if(v->flags & vfCONST) printf("data"); else printf("var");
                                if(v->flags & vfMORPH) printf(".any");
                                if(v->flags & vfREFER) printf(".reference");
                                rdata_t d;
                                memset((unsigned char *)&d,0,sizeof(d));
                                d.type=v->type;
                                unsigned char *data;
                                dl=tokens[v->type].inc;
                                if(dl<0) dl=sizeof(char *); // for TEXT
                                if((v->type!=rANY && v->type!=rTEXT) || (v->flags & vfCONST)==0) {
                                    index*=dl;
                                    if(v->addr) {
                                        data=(unsigned char *)((unsigned char *)(v->addr)+index);
                                    } else {
                                        data=(unsigned char *)((unsigned char *)v+sizeof(vmvar_t)+index);
                                    }
                                }
                                else {  // ANY and TEXT constants will have to be counted
                                    dl=-1;
                                    if(v->addr) {
                                        data=(unsigned char *)((unsigned char *)(v->addr));
                                    } else {
                                        data=(unsigned char *)((unsigned char *)v+sizeof(vmvar_t));
                                    }
                                }
                                if(dl>0) {  // for ordinal types get the actual data in (d.data)
                                    if(v->type==rANY) memcpy((unsigned char *)&d,data,dl);
                                    else memcpy((unsigned char *)&d.data,data,dl);
                                }
                                else if(dl<0) { // non-ordinal types require counting
                                    if(v->type==rTEXT && (v->flags & vfCONST)==0) {	// text variables
                                        memcpy(&d,data,dl);
                                        data=(unsigned char *)&d.data;
                                        if(d.type==0) d.type=rANY;	// fixing initialised but previously never accessed arrays
                                    }
                                    else {	// constants (will have to count them)
                                        while(index--) {
                                            if(v->type==rANY) {
                                                d.type=(rtype_t)*(data++);    // get the data type for the single element to skip
                                                dl=tokens[d.type].inc;
                                                if(dl>0) data+=dl;	// ordinal types
                                                else if(dl<0) {		// TEXT
                                                    while((char)*data) data++;
                                                    data++;
                                                }
                                            }
                                            else if(v->type==rTEXT) {
                                                char *c=(char *)data;
                                                while(c && *c) c++;
                                                if(c) c++;
                                                data=(unsigned char *)c;
                                            }
                                        }
                                        if(v->type==rANY) {
                                            d.type=(rtype_t)*(data++);	// get the data type for the single indexed element
                                            dl=tokens[d.type].inc;
                                            if(dl<0) memcpy((unsigned char *)&d.data.text,&data,sizeof(char *));
                                            else if(dl>0) memcpy((unsigned char *)&d.data,data,dl);
                                        } else memcpy((unsigned char *)&d.data.text,&data,sizeof(char *));
                                    }
                                }
                                printf("%s   ",tokens[d.type].name);
                                if(d.type==rSINT8) {
                                    printf("%hhi (%02hhx)",(signed char)d.data.sint64,(unsigned char)d.data.sint64);
                                }
                                else if(d.type==rSINT16) {
                                    printf("%hi (%04hx)",(signed short)d.data.sint64,(unsigned short)d.data.sint64);
                                }
                                else if(d.type==rSINT32) {
                                    printf("%li (%08lx)",(signed long)d.data.sint64,(unsigned long)d.data.sint64);
                                }
                                else if(d.type==rSINT64) {
                                    printf("%lli (%016llx)",(signed long long)d.data.sint64,(unsigned long long)d.data.sint64);
                                }
                                else if(d.type==rREAL) {
                                    printf("%G",d.data.real);
                                }
                                else if(d.type==rTEXT) {
                                    char *c;
                                    c=(char *)d.data.text;
                                    printf("\"");
                                    while(c && *c) {
                                        if(*c>=' ') printf("%c",*c); else printf("_%02hhx",*c);
                                        c++;
                                    }
                                    printf("\"");
                                }
                                else if(d.type==rFUNC) {
                                    printf("%08lx",(unsigned long)d.data.sint64);
                                }
                                printf("\r\n");
                            } else xv=-1008;
                        } else xv=-1007;
                    }
                    else xv=-1005;
                }

                // unrecognised command (not related to RITTLE) ==================================

                else xv=-1003;
                if(repeat>0) {
					pcmd=conbuf;
                    if(cbsave) {
						strcpy(conbuf,cbsave);
                    	pcmd++;
					}
					else *conbuf=0;
                }
                if(repeat>0) first=0;
            }
            printf("\r\n");
        }

        // write text
        else if (*pcmd>=' ' || *pcmd=='\t') {
            while(*pcmd && *(pcmd+strlen(pcmd)-1)==' ') *(pcmd+strlen(pcmd)-1)=0;   // remove trailing spaces
            char *s=gotoline(text,line);
            addremoveline(s,pcmd,0);
            line++;
        }

        if(xv<0) xvline(NULL);
		xv=0;
	}
    if(ff) xfree((unsigned char **)&ff);    // free up the file buffer
    freeall(0);     // keep the text in memory
}
