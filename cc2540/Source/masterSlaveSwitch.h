/**************************************************************************************************
  Filename:       masterSlaveSwitch.h
  Revised:        $Date: 2010-08-01 14:03:16 -0700 (Sun, 01 Aug 2010) $
  Revision:       $Revision: 23256 $

  Description:    This file contains the Master Slave Switch sample application
                  definitions and prototypes.

  Copyright 2010 - 2011 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#ifndef MASTERSLAVESWITCH_H
#define MASTERSLAVESWITCH_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */


#define  MAC_LEN  6
#define  BUFFER_LEN   10
//#define  BUFFER_END_FLAG  0
  
//#define ROLE_TOTOL_TIME     5000                 //each switch last 5s at most
//#define CENTROL_ROLE_TOTOL_TIME     10000                 //each switch last 5s at most  
  
  
// Master Slave Switch Task Events
#define MSS_START_DEVICE_EVT                              0x0001
#define MSS_PERIODIC_EVT                                  0x0002
#define MSS_CHANGE_ROLE_EVT                               0x0004
#define CHECH_LINK_EVT                           		0x0008
//#define MSS_BROADCAST_EVT                           		0x0010
#define PERIOD_DETECH_EVT                            0x0020
//#define CHANGE_ROLE_EVT_OB                                0x0040
/*********************************************************************
 * MACROS
 */
  
  
  
  
typedef struct{
uint16 Handle;
uint8  MacType;
uint8  *MacAddr;
}ConnectionDeviceInfo_t;

#define WANTED_SERVICE_UUID  SIMPLEPROFILE_SERV_UUID 

extern ConnectionDeviceInfo_t CurrentConnectionInfo;
extern uint8 random_num;

extern uint16 record_switch_times;


/*********************************************************************
 * TYPEDEFS
 */
typedef enum {
  ROLE_PERIPHERAL = 1,
  ROLE_CENTRAL    = 2
} deviceRole_t;





/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
extern void MasterSlaveSwitch_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 MasterSlaveSwitch_ProcessEvent( uint8 task_id, uint16 events );


extern void simpleBLEAddDeviceInfo( uint8 *pAddr, uint8 addrType );


extern char *bdAddr2Str( uint8 *pAddr );


extern void SwitchChange_Init( void );//function in OASL_masterSlaveSwitch.c

extern uint8 MasterSlaveSwitchTaskID;
extern uint8 mac_buffer[BUFFER_LEN][MAC_LEN];
extern uint8 Buffer_Top;

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* MASTERSLAVESWITCH_H */