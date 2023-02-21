#ifndef ADDONS_H
#define ADDONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"

// number of ports in the system
#define PORTS 46

// port structure definition
typedef struct {
    unsigned char pnum;     // port number (typically, pin number on the MCU)
    IoPortId port;          // port id
    unsigned char pbit;     // bit in the hardware port
    unsigned char adcchn;   // ADC channel number (if available, otherwise -1)
    unsigned long func;     // bitmask of the possible assignment functions for the port (see PFN_xxx constants)
} port_t;

// port function flags (lowest 20 bits in port[].func are reserved for functionality flags)
#define PFN_DOUT    (BIT(0))    // digital output
#define PFN_DIN     (BIT(1))    // digital input
#define PFN_AIN     (BIT(2))    // analogue input
#define PFN_PWM     (BIT(3))    // analogue input

// port switch flags (highest 12 bits in port[].func are reserved for modifier flags)
#define PFN_OD      (BIT(31))   // open-drain output
#define PFN_PUP     (BIT(30))   // internal pull-up
#define PFN_PDN     (BIT(29))   // internal pull-down

// touch calibration values
unsigned long touch_calx;
unsigned long touch_caly;

// add-on function prototypes
void __port(void);
void __DOUT(void);
void __DTOG(void);
void __DIN(void);
void __AIN(void);
void __setPWM(void);
void __Vref(void);
void __enable(void);
void __disable(void);
void __trmt(void);
void __recv(void);
void __gettime(void);
void __settime(void);
void __srctime(void);
void __clock(void);
void __sleep(void);
void __display(void);
void __touch(void);

#ifdef __cplusplus
}
#endif

#endif // ADDONS_H
