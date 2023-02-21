/* 
 * File:   utimer.h
 * Author: Spas Spasov
 *
 * Created on 25.10.2016
 */

#ifndef UTIMER_H
#define	UTIMER_H

#include <xc.h>
#include <GenericTypeDefs.h>
#include <sys/attribs.h>

#ifdef	__cplusplus
extern "C" {
#endif


// *****************************************************************************
/* UART Module

  Summary:
    UART modules supported.

  Description:
    This enumeration identifies the available UART modules.
*/

typedef enum
{
#if defined(_TMR1)    
    // TMR1 Module ID.
    UTIMER1 = 0,
#endif   
#if defined(_TMR2)
    // TMR2 Module ID.
    UTIMER2 = 1,
#endif   
#if defined(_TMR3)
    // TMR3 Module ID.
    UTIMER3 = 2,
#endif   
#if defined(_TMR4)
    // TMR4 Module ID.
    UTIMER4 = 3,
#endif   
#if defined(_TMR5)
    // TMR5 Module ID.
    UTIMER5 = 4,
#endif   
#if defined(_TMR6)
    // TMR6 Module ID.
    UTIMER6 = 5,
#endif
#if defined(_TMR7)
    // TMR6 Module ID.
    UTIMER7 = 6,
#endif
#if defined(_TMR8)
    // TMR6 Module ID.
    UTIMER8 = 7,
#endif
#if defined(_TMR9)
    // TMR6 Module ID.
    UTIMER9 = 8,
#endif      
    // Number of available TMR modules.
    UTIMER_NUMBER_OF_MODULES = 0
#if defined(_TMR1)
                + 1
#endif
#if defined(_TMR2)
                + 1
#endif   
#if defined(_TMR3)
                + 1
#endif
#if defined(_TMR4)
                + 1
#endif             
#if defined(_TMR5)
                + 1
#endif
#if defined(_TMR6)
                + 1
#endif  
#if defined(_TMR7)
                + 1
#endif 
#if defined(_TMR8)
                + 1
#endif
#if defined(_TMR9)
                + 1
#endif             
} UTIMER_MODULE;


/******************************************************************************
     * Available options for config Timer parameter
     *****************************************************************************/
        // On/off control - values are mutually exclusive
        #define UT_ON                          (1 << _T2CON_ON_POSITION)        /* Timer89 ON */
        #define UT_OFF                         (0)

        // Stop-in-idle control - values are mutually exclusive
        #define UT_IDLE_STOP                   (1 << _T2CON_SIDL_POSITION)     /* stop during idle */
        #define UT_IDLE_CON                    (0)                             /* operate during idle */

        // Asynchronous write control - values are mutually exclusive
        #define UT_TMWDIS_ON                    (1 << _T1CON_TWDIS_POSITION)    /* Asynchronous Write Disable */
        #define UT_TMWDIS_OFF                   (0)				
				
        // Timer gate control - values are mutually exclusive
        #define UT_GATE_ON                     (1 << _T2CON_TGATE_POSITION)    /* Timer Gate accumulation mode ON */
        #define UT_GATE_OFF                    (0)

        // Prescale values - values are mutually exclusive
        #define UT_PS_256                       (7 << _T2CON_TCKPS_POSITION)    /* Prescaler 1:256 */
        #define UT_PS_64                        (6 << _T2CON_TCKPS_POSITION)    /*           1:64 */
        #define UT_PS_32                        (5 << _T2CON_TCKPS_POSITION)    /*           1:32 */
        #define UT_PS_16                        (4 << _T2CON_TCKPS_POSITION)    /*           1:16 */
        #define UT_PS_8                         (3 << _T2CON_TCKPS_POSITION)    /*           1:8 */
        #define UT_PS_4                         (2 << _T2CON_TCKPS_POSITION)    /*           1:4 */
        #define UT_PS_2                         (1 << _T2CON_TCKPS_POSITION)    /*           1:2 */
        #define UT_PS_1                         (0)                             /*           1:1 */

        // 32-bit or 16-bit - values are mutually exclusive
        #define UT_32BIT_MODE_ON                (1 << _T2CON_T32_POSITION)      /* Enable 32-bit mode */
        #define UT_32BIT_MODE_OFF               (0)

        // Sync option - values are mutually exclusive
        #define UT_SYNC_EXT_ON                  (1 << _T1CON_TSYNC_POSITION)    /* Synch external clk input */
        #define UT_SYNC_EXT_OFF                 (0)	 				
				
        // Sync external clock option - values are mutually exclusive
        #define UT_SOURCE_EXT                   (1 << _T2CON_TCS_POSITION)      /* External clock source */
        #define UT_SOURCE_INT                   (0)                             /* Internal clock source */
    /***********************************
     * End config parameter values
     ************************************/
		 
    /******************************************************************************
     * Available options for config INT Timer parameter
     *****************************************************************************/
        // Interrupt on/off - values are mutually exclusive
        #define UT_INT_ON                           (1 << 15)                   /* TIMx Interrupt Enable */
        #define UT_INT_OFF                      	(0)

        // Interrupt priority - values are mutually exclusive
        #define UT_INT_PRIOR_7                  	(7)
        #define UT_INT_PRIOR_6                  	(6)
        #define UT_INT_PRIOR_5                  	(5)
        #define UT_INT_PRIOR_4                  	(4)
        #define UT_INT_PRIOR_3                  	(3)
        #define UT_INT_PRIOR_2                  	(2)
        #define UT_INT_PRIOR_1                  	(1)
        #define UT_INT_PRIOR_0                  	(0)

        // Interrupt sub-priority - values are mutually exclusive
        #define UT_INT_SUB_PRIOR_3              	(3 << 4)
        #define UT_INT_SUB_PRIOR_2              	(2 << 4)
        #define UT_INT_SUB_PRIOR_1              	(1 << 4)
        #define UT_INT_SUB_PRIOR_0              	(0 << 4)
    /***********************************
     * End config parameter values
     ************************************/

/*********************************************************************
 * Timer X Interrupt Control Functions
 ********************************************************************/
extern inline void __attribute__((always_inline)) 
mUTClearIntFlag( UTIMER_MODULE id ) 
{
	volatile unsigned int	*ifsx_clr;
    unsigned int            mask;
    
	if( UTIMER_NUMBER_OF_MODULES < id ) return;  
    
    if( UTIMER7 > id)
    {
        ifsx_clr 	= (volatile UINT *)(0xBF810044);                            //IFS0CLR
        if( UTIMER6 > id)
        {
            mask 	= 1<<(4+(5*id));
        }
        else
        {
            mask 	= _IFS0_T6IF_MASK;
        }
    }
    else
    {
        ifsx_clr 	= (volatile UINT *)(0xBF810054);                            //IFS1CLR
        mask 	= 1<<(4*(id-6));
    }
    *ifsx_clr = mask;
}

extern inline unsigned int __attribute__((always_inline)) 
mUTGetIntFlag( UTIMER_MODULE id ) 
{
	volatile unsigned int	*ifsx;
    unsigned int            mask;
    
	if( UTIMER_NUMBER_OF_MODULES < id ) return 0;    
    
    if( UTIMER7 > id)
    {
        ifsx 	= (volatile UINT *)(0xBF810040);                                //IFS0
        if( UTIMER6 > id)
        {
            mask 	= 1<<(4+(5*id));
        }
        else
        {
            mask 	= _IFS0_T6IF_MASK;
        }
    }
    else
    {
        ifsx 	= (volatile UINT *)(0xBF810050);                                //IFS1
        mask 	= 1<<(4*(id-6));
    }    
    return (*ifsx & mask);
}

extern inline unsigned int __attribute__((always_inline)) 
mUTGetIntEnable( UTIMER_MODULE id )
{
	volatile unsigned int	*iecx;
    unsigned int            mask;
    
	if( UTIMER_NUMBER_OF_MODULES < id ) return 0;    
    
    if( UTIMER7 > id)
    {
        iecx 	= (volatile UINT *)(0xBF8100C0);                                //IEC0
        if( UTIMER6 > id)
        {
            mask 	= 1<<(4+(5*id));
        }
        else
        {
            mask 	= _IEC0_T6IE_MASK;
        }
    }
    else
    {
        iecx 	= (volatile UINT *)(0xBF8100D0);                                //IEC1
        mask 	= 1<<(4*(id-6));
    }    
    return (*iecx & mask);
}

extern inline void __attribute__((always_inline)) 
mUTIntEnable( UTIMER_MODULE id, bool enable)
{
	volatile unsigned int	*iecx;
    unsigned int            mask;
    
	if( UTIMER_NUMBER_OF_MODULES < id ) return;    
    
    if( UTIMER7 > id)
    {
        iecx 	= (volatile UINT *)(0xBF8100C4);                                //IEC0CLR
        if( UTIMER6 > id)
        {
            mask 	= 1<<(4+(5*id));
        }
        else
        {
            mask 	= _IEC0_T6IE_MASK;
        }
    }
    else
    {
        iecx 	= (volatile UINT *)(0xBF8100D4);                                //IEC1CLR
        mask 	= 1<<(4*(id-6));
    }        
    *iecx = mask;
    
    if( enable )
    {
        iecx    += 1;                                                           //IECxSET
        *iecx   = mask;
    }
}

extern inline void __attribute__((always_inline)) 
mUTSetIntPriority( UTIMER_MODULE id, unsigned int priority)         
{
    
    if( UTIMER_NUMBER_OF_MODULES < id ) return;     
    
    switch( id )
    {
        case UTIMER1:
            IPC1CLR = _IPC1_T1IP_MASK;
            IPC1SET = ((priority) << _IPC1_T1IP_POSITION);
            break;
        case UTIMER2:            
            IPC2CLR = _IPC2_T2IP_MASK;
            IPC2SET = ((priority) << _IPC2_T2IP_POSITION);
            break;
        case UTIMER3:
            IPC3CLR = _IPC3_T3IP_MASK;
            IPC3SET = ((priority) << _IPC3_T3IP_POSITION);
            break;
        case UTIMER4:   
            IPC4CLR = _IPC4_T4IP_MASK;
            IPC4SET = ((priority) << _IPC4_T4IP_POSITION);
            break;
        case UTIMER5:   
            IPC6CLR = _IPC6_T5IP_MASK;
            IPC6SET = ((priority) << _IPC6_T5IP_POSITION);
            break;
        case UTIMER6:   
            IPC7CLR = _IPC7_T6IP_MASK;
            IPC7SET = ((priority) << _IPC7_T6IP_POSITION);
            break;
        case UTIMER7:   
            IPC8CLR = _IPC8_T7IP_MASK;
            IPC8SET = ((priority) << _IPC8_T7IP_POSITION);
            break;
        case UTIMER8:   
            IPC9CLR = _IPC9_T8IP_MASK;
            IPC9SET = ((priority) << _IPC9_T8IP_POSITION);
            break;
        case UTIMER9:   
            IPC10CLR = _IPC10_T9IP_MASK;
            IPC10SET = ((priority) << _IPC10_T9IP_POSITION);
            break;
    }
}

extern inline unsigned int __attribute__((always_inline)) 
mUTGetIntPriority( UTIMER_MODULE id )
{
unsigned int ret;

    if( UTIMER_NUMBER_OF_MODULES < id ) return 0;     
    
    switch( id )
    {
        case UTIMER1:
            ret = IPC1bits.T1IP;
            break;
        case UTIMER2:            
            ret = IPC2bits.T2IP;
            break;
        case UTIMER3:
            ret = IPC3bits.T3IP;
            break;
        case UTIMER4:   
            ret = IPC4bits.T4IP;
            break;
        case UTIMER5:   
            ret = IPC6bits.T5IP;
            break;
        case UTIMER6:   
            ret = IPC7bits.T6IP;
            break;
        case UTIMER7:   
            ret = IPC8bits.T7IP;
            break;
        case UTIMER8:   
            ret = IPC9bits.T8IP;
            break;
        case UTIMER9:   
            ret = IPC10bits.T9IP;
            break;
    }    
    return ret;
}    
    
extern inline void __attribute__((always_inline)) 
mUTSetIntSubPriority( UTIMER_MODULE id, unsigned int subPriority)
{
    
    if( UTIMER_NUMBER_OF_MODULES < id ) return;     
    
    switch( id )
    {
        case UTIMER1:
            IPC1CLR = _IPC1_T1IS_MASK;
            IPC1SET = ((subPriority) << _IPC1_T1IS_POSITION);
            break;
        case UTIMER2:            
            IPC2CLR = _IPC2_T2IS_MASK;
            IPC2SET = ((subPriority) << _IPC2_T2IS_POSITION);
            break;
        case UTIMER3:
            IPC3CLR = _IPC3_T3IS_MASK;
            IPC3SET = ((subPriority) << _IPC3_T3IS_POSITION);
            break;
        case UTIMER4:   
            IPC4CLR = _IPC4_T4IS_MASK;
            IPC4SET = ((subPriority) << _IPC4_T4IS_POSITION);
            break;
        case UTIMER5:   
            IPC6CLR = _IPC6_T5IS_MASK;
            IPC6SET = ((subPriority) << _IPC6_T5IS_POSITION);
            break;
        case UTIMER6:   
            IPC7CLR = _IPC7_T6IS_MASK;
            IPC7SET = ((subPriority) << _IPC7_T6IS_POSITION);
            break;
        case UTIMER7:   
            IPC8CLR = _IPC8_T7IS_MASK;
            IPC8SET = ((subPriority) << _IPC8_T7IS_POSITION);
            break;
        case UTIMER8:   
            IPC9CLR = _IPC9_T8IS_MASK;
            IPC9SET = ((subPriority) << _IPC9_T8IS_POSITION);
            break;
        case UTIMER9:   
            IPC10CLR = _IPC10_T9IS_MASK;
            IPC10SET = ((subPriority) << _IPC10_T9IS_POSITION);
            break;
    }    
}            

extern inline unsigned int __attribute__((always_inline)) 
mUTGetIntSubPriority( UTIMER_MODULE id ) 
{
unsigned int ret;

    if( UTIMER_NUMBER_OF_MODULES < id ) return 0;     
    
    switch( id )
    {
        case UTIMER1:
            ret = IPC1bits.T1IS;
            break;
        case UTIMER2:            
            ret = IPC2bits.T2IS;
            break;
        case UTIMER3:
            ret = IPC3bits.T3IS;
            break;
        case UTIMER4:   
            ret = IPC4bits.T4IS;
            break;
        case UTIMER5:   
            ret = IPC6bits.T5IS;
            break;
        case UTIMER6:   
            ret = IPC7bits.T6IS;
            break;
        case UTIMER7:   
            ret = IPC8bits.T7IS;
            break;
        case UTIMER8:   
            ret = IPC9bits.T8IS;
            break;
        case UTIMER9:   
            ret = IPC10bits.T9IS;
            break;
    }
    return ret;    
}

/******************************************************************************
 * Function:        void OpenUTimer(UTIMER_MODULE id, unsigned int config, unsigned int period)
 *
 * Description:     Configures UTimer X
 *
 * PreCondition:    None
 *
 * Inputs:          config: Bit-wise OR value of UT_ON/OFF, UT_IDLE_XXX,
 *                          UT_GATE_XXX, UT_PS_XXX, UT_32BIT_MODE_ON,
 *                          UT_SOURCE_XXX...
 *
 *                  Note: An absent symbol assumes corresponding bit(s)
 *                  are disabled, or default value, and will be set = 0.
 *
 *                  period: A value between 0 - 0xffff inclusive
 *
 * Output:          None
 *
 * Example:         OpenUTimer(UTIMER2, UT_ON | UT_IDLE_ON | UT_PS_1_4, 123);
 *
 *****************************************************************************/
extern inline void __attribute__((always_inline)) 
OpenUTimer( UTIMER_MODULE id, unsigned int config,  unsigned int period )
{
	volatile unsigned int	*tmr_txcon;
	volatile unsigned int	*tmr_tmrx;
	volatile unsigned int	*tmr_prx;	
	
	if( UTIMER_NUMBER_OF_MODULES < id ) return;

	tmr_txcon 	= (volatile UINT *)(0xBF840000 + (0x200*((UINT)id)));           //TMR1TxCON
	tmr_tmrx    = (volatile UINT *)(0xBF840010 + (0x200*((UINT)id)));           //TMR1TMRx
	tmr_prx     = (volatile UINT *)(0xBF840020 + (0x200*((UINT)id)));           //TMR1PRx
	
	*tmr_txcon 	= (config)&~(UT_ON);
	*tmr_tmrx	= 0;
	*tmr_prx	= period;
	tmr_txcon 	+= 2;                                                           //TMRxTxCONSET
	*tmr_txcon	= (config)&(UT_ON);
}


/******************************************************************************
 * Function:        CloseUTimer(UTIMER_MODULE id)
 *
 * Description:     Switches off the UTimerX
 *
 * PreCondition:    None
 *
 * Inputs:          None
 *
 * Output:          None
 *
 * Example:         CloseUTimer(UTIMER2);
 *
 *****************************************************************************/
extern inline void __attribute__((always_inline))  
CloseUTimer( UTIMER_MODULE id )
{
	volatile unsigned int	*tmr_txcon;
	
	if( UTIMER_NUMBER_OF_MODULES < id ) return;
	
	tmr_txcon 	= (volatile UINT *)(0xBF840000 + (0x200*((UINT)id)));           //TMR1TxCON	
	
    mUTIntEnable(id, false), 
	*tmr_txcon = 0;
}


/******************************************************************************
 * Function:        void ConfigIntUTimer(UTIMER_MODULE id, unsigned int config)
 *
 * Description:     Configures UTimer X interrupt
 *
 * PreCondition:    None
 *
 * Inputs:          priority: Bit-wise OR value of UT_INT_XXX, UT_INT_PRIOR_XXX
 *                            and UT_INT_SUB_PRIOR_X.
 *
 *                  Note: An absent symbol assumes corresponding bit(s)
 *                  are disabled, or default value, and will be set = 0.
 *
 * Output:          None
 *
 * Example:         ConfigIntUTimer(UTIMER2, UT_INT_ON | UT_INT_PRIOR_3 | UT_INT_SUB_PRIOR1);
 *
 *****************************************************************************/
extern inline void __attribute__((always_inline)) 
ConfigIntUTimer( UTIMER_MODULE id, unsigned int config )
{
    
	if( UTIMER_NUMBER_OF_MODULES < id ) return;
    
    mUTClearIntFlag(id); 
    mUTSetIntPriority(id, ((config) & 7));
    mUTSetIntSubPriority(id, ((config) >> 4) & 3);
    mUTIntEnable(id, (config) >> 15);
}


#ifdef	__cplusplus
}
#endif

#endif	/* UTIMER_H */

