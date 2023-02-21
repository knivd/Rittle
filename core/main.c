#include "rittle.h"

// this loop never ends
int main(void) {
	init_cold();
    while(1) {
       	init_warm();
        rittle();
    }
    return 0;
}
