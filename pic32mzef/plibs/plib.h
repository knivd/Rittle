/*
 * File:   plib.h
 * Author: Spas Spasov
 *
 * Created on May 2019
 */

#ifndef PLIB_H
#define	PLIB_H

#ifdef	__cplusplus
extern "C" {
#endif


#include <xc.h>

#if defined(__PIC32MZ__)
    #define microinstr  nomicromips
    #include <errno.h>
    #define NVM_USE_ADDITIONAL_SYNCHRO
    #include "mz_libs/include/plib_mz.h"           // PIC32MZ EF peripheral libraries
#else
		#error "Wrong platform defined"
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* PLIB_H */

