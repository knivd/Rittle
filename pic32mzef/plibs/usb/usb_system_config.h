/* 
 * File:   usb_system_config.h
 * Author: Spas Spasov
 *
 * Created on November 2016
 */

#ifndef USB_SYSTEM_CONFIG_H
#define	USB_SYSTEM_CONFIG_H

#include "../../platform.h"
#include "osal/osal.h"
#include "system/system.h"

#ifdef	__cplusplus
extern "C" {
#endif

    
/*** USB Driver Configuration ***/
/* Enables Device Support */
#define DRV_USBHS_DEVICE_SUPPORT                    true

/* Disable Device Support */
#define DRV_USBHS_HOST_SUPPORT                      false

/* Maximum USB driver instances */
#define DRV_USBHS_INSTANCES_NUMBER                  1

/* Interrupt mode enabled */
#define DRV_USBHS_INTERRUPT_MODE                    true

/* Number of Endpoints used */
#define DRV_USBHS_ENDPOINTS_NUMBER                  4         // cdc + msd

/*** USB Device Stack Configuration ***/
/* The USB Device Layer will not initialize the USB Driver */
#define USB_DEVICE_DRIVER_INITIALIZE_EXPLICIT

/* Maximum device layer instances */
#define USB_DEVICE_INSTANCES_NUMBER                 1

/* EP0 size in bytes */
#define USB_DEVICE_EP0_BUFFER_SIZE                  64

/* Enable SOF Events */ 
//#define USB_DEVICE_SOF_EVENT_ENABLE

/* Maximum instances of CDC function driver */
#define USB_DEVICE_CDC_INSTANCES_NUMBER             1
    
/* CDC Transfer Queue Size for both read and
   write. Applicable to all instances of the
   function driver */
#define USB_DEVICE_CDC_QUEUE_DEPTH_COMBINED         3
    
/**/
#define USE_UTIMER_USBDEVICE_SYS_TASK    
    
#ifdef  USE_UTIMER_USBDEVICE_SYS_TASK
/* USB peripherial Timer instances used for USB DRV */
#define USB_DEVICE_UTIMER_NUMBER                    1    
#define USB_DEVICE_UTIMER_INT_PRIORITY              3
    
/**/    
#define USB_DEVICE_UTIMER_PB_FREQ                   PBCLK3
#define USB_DEVICE_UTIMER_PRESCALE                  256
#define USB_DEVICE_UTIMER_TOGGLES_PER_SEC           500
#define USB_DEVICE_UTIMER_TICK                      (USB_DEVICE_UTIMER_PB_FREQ/USB_DEVICE_UTIMER_PRESCALE/USB_DEVICE_UTIMER_TOGGLES_PER_SEC)    
#endif 
    
#define USB_SYS_DEBUG_MESSAGE(message)
#define USB_SYS_DEBUG(message) 

    
#ifdef	__cplusplus
}
#endif

#endif	/* USB_SYSTEM_CONFIG_H */

