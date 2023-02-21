#include "platform.h"

void platform_link(void) {}
void init_warm(void) {}


void init_cold(void) {
	srand(time(NULL));
	xfree((unsigned char **)ramdisk);
	fclose(ifs);
	ifs=NULL;
    setbuf(stdin,NULL);
    setbuf(stdout,NULL);
}


// generate random number
int rnd(void) {
    return rand();
}


// system reset
void sysreset(void) {
    xv=-22; // emulated system reset
}


// delay for specified number of microseconds
void dlyus(unsigned long us) {
	us/=1000;	// clock is coming in milliseconds
	unsigned long c=(unsigned long)clock();
    us+=c;
    while(c>us) c=(unsigned long)clock();	// handle cases when (e) rolls over the counter
    while(c<us) c=(unsigned long)clock();
}


// read the core clock counter
// the core clock counter is a free-running counter clocked at precisely 1MHz
unsigned long cclk(void) {
	return 1000*clock();
}
