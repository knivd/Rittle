/*******************************************************************************
 USB CDC ACM SubClass

  Company:
    Microchip Technology Inc.

  File Name:
    usb_device_cdc_acm.c

  Summary:
    USB CDC ACM SubClass

  Description:
    USB CDC ACM SubClass
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
 Copyright (c) 2013 released Microchip Technology Inc.  All rights reserved.

 Microchip licenses to you the right to use, modify, copy and distribute
 Software only when embedded on a Microchip microcontroller or digital signal
 controller that is integrated into your product or third party product
 (pursuant to the sublicense terms in the accompanying license agreement).

 You should refer to the license agreement accompanying this Software for
 additional information regarding your rights and obligations.

 SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
 MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
 IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
 CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
 OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
 CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
 SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/*  This section lists the other files that are included in this file.
 */
#include <stdio.h>
#include <stdint.h>
#include "../../usb_cdc.h"
#include "../../usb_device_cdc.h"
#include "../usb_device_cdc_local.h"

// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Data Types
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Functions
// *****************************************************************************
// *****************************************************************************

//******************************************************************************
/* Function:
    void _USB_DEVICE_CDC_ACMSetUpPacketHandler ( USB_DEVICE_CDC_INSTANCE *instance,
                                                    uint16_t value )

  Summary:
    Handles ACM sub class specific requests.

  Description:
    This function handles ACM sub class specific requests.

  Remarks:
    Called by the CDC function driver.
 */

void _USB_DEVICE_CDC_ACMSetUpPacketHandler 
(
    SYS_MODULE_INDEX iCDC ,
    USB_DEVICE_CDC_INSTANCE * thisCDCDevice,
    USB_SETUP_PACKET * setupRequest
)
{
    
    /* Check the request */
    switch (setupRequest->bRequest )
    {
        case USB_CDC_REQUEST_SET_LINE_CODING:

            /* Send this event to application. The application
             * should issues a control receive request to receive
             * the data from the host. */

            if(thisCDCDevice->appEventCallBack != NULL)
            {
                thisCDCDevice->appEventCallBack(iCDC, 
                        USB_DEVICE_CDC_EVENT_SET_LINE_CODING, 
                        NULL, thisCDCDevice->userData);
            }

            break;

        case USB_CDC_REQUEST_GET_LINE_CODING:

            /* Send this event to application. The application should
             * issue a control send request to send this request to 
             * the host. */

            if(thisCDCDevice->appEventCallBack != NULL)
            {
                thisCDCDevice->appEventCallBack(iCDC, 
                        USB_DEVICE_CDC_EVENT_GET_LINE_CODING, 
                        NULL, thisCDCDevice->userData);
            }

            break;

        case USB_CDC_REQUEST_SET_CONTROL_LINE_STATE:

            /* In this event, the data is available in the
             * setup packet. Send it to the application */

            if(thisCDCDevice->appEventCallBack != NULL)
            {
                thisCDCDevice->appEventCallBack(iCDC,
                        USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE,
                        (USB_CDC_CONTROL_LINE_STATE *)(&setupRequest->wValue),
                        thisCDCDevice->userData);
            }

            break;

            /* AT commands */
        case USB_CDC_REQUEST_SEND_ENCAPSULATED_COMMAND:
        case USB_CDC_REQUEST_GET_ENCAPSULATED_RESPONSE:

            /* AT commands are not supported */
            USB_DEVICE_ControlStatus(thisCDCDevice->deviceHandle, 
                    USB_DEVICE_CONTROL_STATUS_ERROR);
            break;

            /* break request */
        case USB_CDC_REQUEST_SEND_BREAK:

            /* In this event, the data is available in the
             * setup packet. Send it to the application */

            if(thisCDCDevice->appEventCallBack != NULL)
            {
                thisCDCDevice->appEventCallBack(iCDC,
                        USB_DEVICE_CDC_EVENT_SEND_BREAK,
                        (uint16_t *)(&setupRequest->wValue), thisCDCDevice->userData);
            }

            break;

            /* requests that do not belog to ACM sub class */
        case USB_CDC_REQUEST_SET_COMM_FEATURE:
        case USB_CDC_REQUEST_GET_COMM_FEATURE:
        case USB_CDC_REQUEST_CLEAR_COMM_FEATURE:
        case USB_CDC_REQUEST_SET_AUX_LINE_STATE:
        case USB_CDC_REQUEST_SET_HOOK_STATE:
        case USB_CDC_REQUEST_PULSE_SETUP:
        case USB_CDC_REQUEST_SEND_PULSE:
        case USB_CDC_REQUEST_SET_PULSE_TIME:
        case USB_CDC_REQUEST_RING_AUX_JACK:
        case USB_CDC_REQUEST_SET_RINGER_PARMS:
        case USB_CDC_REQUEST_GET_RINGER_PARMS:
        case USB_CDC_REQUEST_SET_OPERATIONAL_PARMS:
        case USB_CDC_REQUEST_GET_OPERATIONAL_PARMS:
        case USB_CDC_REQUEST_SET_LINE_PARMS:
        case USB_CDC_REQUEST_GET_LINE_PARMS:
        case USB_CDC_REQUEST_DIAL_DIGITS:
        case USB_CDC_REQUEST_SET_UNIT_PARAMETER:
        case USB_CDC_REQUEST_GET_UNIT_PARAMETER:
        case USB_CDC_REQUEST_CLEAR_UNIT_PARAMETER:
        case USB_CDC_REQUEST_GET_PROFILE:
        case USB_CDC_REQUEST_SET_ETHERNET_MULTICAST_FILTERS:
        case USB_CDC_REQUEST_SET_ETHERNET_POWER_MANAGEMENT_FILTER:
        case USB_CDC_REQUEST_GET_ETHERNET_POWER_MANAGEMENT_FILTER:
        case USB_CDC_REQUEST_SET_ETHERNET_PACKET_FILTER:
        case USB_CDC_REQUEST_GET_ETHERNET_STATISTIC:
        case USB_CDC_REQUEST_SET_ATM_DATA_FORMAT:
        case USB_CDC_REQUEST_GET_ATM_DEVICE_STATISTICS:
        case USB_CDC_REQUEST_SET_ATM_DEFAULT_VC:
        case USB_CDC_REQUEST_GET_ATM_VC_STATISTICS:
        default:

            /* These request are not supported */

            USB_DEVICE_ControlStatus(thisCDCDevice->deviceHandle,
                       USB_DEVICE_CONTROL_STATUS_ERROR);

            break;
    }
}



/*******************************************************************************
 End of File
 */
