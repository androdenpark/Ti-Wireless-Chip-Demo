/**************************************************************************************************
  Filename:       simpleGATTprofile.c
  Revised:        $Date: 2010-08-06 08:56:11 -0700 (Fri, 06 Aug 2010) $
  Revision:       $Revision: 23333 $

  Description:    This file contains the Simple GATT profile sample GATT service 
                  profile for use with the BLE sample application.

  Copyright 2010 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
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

/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "exampleService.h"

#include "hal_lcd.h"
#include "z_delay.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

//#define SERVAPP_NUM_ATTR_SUPPORTED        17

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Simple GATT Profile Service UUID: 0xFFF0
CONST uint8 simpleProfileServUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_SERV_UUID), HI_UINT16(SIMPLEPROFILE_SERV_UUID)
};

// Characteristic 1 UUID: 0xFFF1
CONST uint8 simpleProfilechar1UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR1_UUID), HI_UINT16(SIMPLEPROFILE_CHAR1_UUID)
};

// Characteristic 2 UUID: 0xFFF2
CONST uint8 simpleProfilechar2UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR2_UUID), HI_UINT16(SIMPLEPROFILE_CHAR2_UUID)
};

// Simple GATT Profile Service UUID: 0xFF00
CONST uint8 ServiceForFinishServUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SERVICEFORFINISH_SERV_UUID), HI_UINT16(SERVICEFORFINISH_SERV_UUID)
};


/*
// Characteristic 3 UUID: 0xFFF3
CONST uint8 simpleProfilechar3UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR3_UUID), HI_UINT16(SIMPLEPROFILE_CHAR3_UUID)
};

// Characteristic 4 UUID: 0xFFF4
CONST uint8 simpleProfilechar4UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR4_UUID), HI_UINT16(SIMPLEPROFILE_CHAR4_UUID)
};

// Characteristic 5 UUID: 0xFFF5
CONST uint8 simpleProfilechar5UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR5_UUID), HI_UINT16(SIMPLEPROFILE_CHAR5_UUID)
};
*/

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */



static simpleProfileCBs_t *simpleProfile_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Simple Profile Service attribute
static CONST gattAttrType_t simpleProfileService = { ATT_BT_UUID_SIZE, simpleProfileServUUID };


// Simple Profile Service attribute
static CONST gattAttrType_t ServiceForFinishService = { ATT_BT_UUID_SIZE, ServiceForFinishServUUID };


// Simple Profile Characteristic 1 Properties
static uint8 simpleProfileChar1Props = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 1 Value
//#ifdef HAL_UART_TRANS
static uint8 simpleProfileChar1[SIMPLEPROFILE_CHAR1_LEN] = {0};
//#else
//static uint8 simpleProfileChar1 = 0;
//#endif

// Simple Profile Characteristic 1 User Description
static uint8 simpleProfileChar1UserDesp[17] = "Characteristic 1\0";


// Simple Profile Characteristic 2 Properties
static uint8 simpleProfileChar2Props = GATT_PROP_NOTIFY;

// Characteristic 2 Value
//static uint8 simpleProfileChar2 = 0;
static uint8 simpleProfileChar2[SIMPLEPROFILE_CHAR2_LEN] = {0};

// Simple Profile Characteristic 2 Configuration
gattCharCfg_t simpleProfileChar2Config[GATT_MAX_NUM_CONN];

// Simple Profile Characteristic 2 User Description
static uint8 simpleProfileChar2UserDesp[17] = "Characteristic 2\0";

/*
// Simple Profile Characteristic 3 Properties
static uint8 simpleProfileChar3Props = GATT_PROP_WRITE;

// Characteristic 3 Value
static uint8 simpleProfileChar3 = 0;

// Simple Profile Characteristic 3 User Description
static uint8 simpleProfileChar3UserDesp[17] = "Characteristic 3\0";


// Simple Profile Characteristic 4 Properties
static uint8 simpleProfileChar4Props = GATT_PROP_NOTIFY;

// Characteristic 4 Value
static uint8 simpleProfileChar4 = 0;

// Simple Profile Characteristic 4 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t simpleProfileChar4Config[GATT_MAX_NUM_CONN];
                                        
// Simple Profile Characteristic 4 User Description
static uint8 simpleProfileChar4UserDesp[17] = "Characteristic 4\0";


// Simple Profile Characteristic 5 Properties
static uint8 simpleProfileChar5Props = GATT_PROP_READ;

// Characteristic 5 Value
static uint8 simpleProfileChar5[SIMPLEPROFILE_CHAR5_LEN] = { 0, 0, 0, 0, 0 };

// Simple Profile Characteristic 5 User Description
static uint8 simpleProfileChar5UserDesp[17] = "Characteristic 5\0";
*/

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t simpleProfileAttrTbl[] = 
{
  // Simple Profile Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&simpleProfileService            /* pValue */
  },

    // Characteristic 1 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar1Props 
    },

      // Characteristic Value 1
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar1UUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        simpleProfileChar1
      },

      // Characteristic 1 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar1UserDesp 
      },      

    // Characteristic 2 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar2Props 
    },

      // Characteristic Value 2
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar2UUID },
        0, 
        0, 
        simpleProfileChar2 
      },
      
      // Characteristic 2 configuration
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        (uint8 *)simpleProfileChar2Config 
      },

      // Characteristic 2 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar2UserDesp 
      },           
  /*    
    // Characteristic 3 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar3Props 
    },

      // Characteristic Value 3
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar3UUID },
        GATT_PERMIT_WRITE, 
        0, 
        &simpleProfileChar3 
      },

      // Characteristic 3 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar3UserDesp 
      },

    // Characteristic 4 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar4Props 
    },

      // Characteristic Value 4
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar4UUID },
        0, 
        0, 
        &simpleProfileChar4 
      },

      // Characteristic 4 configuration
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        (uint8 *)simpleProfileChar4Config 
      },
      
      // Characteristic 4 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar4UserDesp 
      },
     
    // Characteristic 5 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar5Props 
    },

      // Characteristic Value 5
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar5UUID },
        GATT_PERMIT_AUTHEN_READ, 
        0, 
        simpleProfileChar5 
      },

      // Characteristic 5 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar5UserDesp 
      },
*/

  // Profile for Finish
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&ServiceForFinishService            /* pValue */
      
  },

};


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static uint8 simpleProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen );
static bStatus_t simpleProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset );

static void simpleProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

static void ServerNotificationCallBack(linkDBItem_t *pLinkItem);


/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t simpleProfileCBs =
{
  simpleProfile_ReadAttrCB,  // Read callback function pointer
  simpleProfile_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleProfile_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t ExampleService_AddService( void )
{
 static uint8 excute_time =0;
 uint8 status = SUCCESS;
 
 if(excute_time > 0)          //make sure it is only excuted once, because everytime it excuted, it add the attr to the list ,no matter wheather it exists in it.
   return(status);
 else
   excute_time++;
 
 HalLcdWriteStringValue("excute_time",excute_time,10,HAL_LCD_LINE_6);
  


  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, simpleProfileChar2Config );

  // Register with Link DB to receive link status change callback
  VOID linkDB_Register( simpleProfile_HandleConnStatusCB );  
  

    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( simpleProfileAttrTbl, 
                                          GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                          &simpleProfileCBs );


  return ( status );
}


/*********************************************************************
 * @fn      SimpleProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t ExampleService_RegisterAppCBs( simpleProfileCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    simpleProfile_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}
  

/*********************************************************************
 * @fn      SimpleProfile_SetParameter
 *
 * @brief   Set a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to right
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t ExampleService_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
      case SIMPLEPROFILE_CHAR1:{
            if ( len == SIMPLEPROFILE_CHAR1_ATCLEN ) {
                VOID osal_memcpy( simpleProfileChar1, value, SIMPLEPROFILE_CHAR1_ATCLEN );
            }
                else
                    ret = bleInvalidRange;               
            break;
      }
  
      case SIMPLEPROFILE_CHAR2:{
            if ( len == SIMPLEPROFILE_CHAR2_ATCLEN ) {
              VOID osal_memcpy( simpleProfileChar2, value, SIMPLEPROFILE_CHAR2_ATCLEN );
              linkDB_PerformFunc( ServerNotificationCallBack);
              // See if Notification has been enabled
             // GATTServApp_ProcessCharCfg( simpleProfileChar2Config, simpleProfileChar2, FALSE,
             // simpleProfileAttrTbl, GATT_NUM_ATTRS( simpleProfileAttrTbl ),INVALID_TASK_ID );
            }
              else
                    ret = bleInvalidRange;              
            break;
       }
       

    

                                   /*    case SIMPLEPROFILE_CHAR2:
                                        if ( len == sizeof ( uint8 ) ) 
                                        {
                                          simpleProfileChar2 = *((uint8*)value);
                                        }
                                        else
                                        {
                                          ret = bleInvalidRange;
                                        }
                                        break;
                                  
                                    case SIMPLEPROFILE_CHAR3:
                                        if ( len == sizeof ( uint8 ) ) 
                                        {
                                          simpleProfileChar3 = *((uint8*)value);
                                        }
                                        else
                                        {
                                          ret = bleInvalidRange;
                                        }
                                        break;
      
                                  
                                        case SIMPLEPROFILE_CHAR4:{
                                            if ( len == sizeof ( uint8 )){
                                              simpleProfileChar4 = *((uint8*)value);                                         
                                              // See if Notification has been enabled
                                              GATTServApp_ProcessCharCfg( simpleProfileChar4Config, &simpleProfileChar4, FALSE,
                                                                          simpleProfileAttrTbl, GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                                                          INVALID_TASK_ID );
                                            }
                                       
                                            else
                                                  ret = bleInvalidRange;
                                        
                                        break;
                                        }
     
                                  
                                      case SIMPLEPROFILE_CHAR5:
                                        if ( len == SIMPLEPROFILE_CHAR5_LEN ) 
                                        {
                                          VOID osal_memcpy( simpleProfileChar5, value, SIMPLEPROFILE_CHAR5_LEN );
                                        }
                                        else
                                        {
                                          ret = bleInvalidRange;
                                        }
                                        break;
                                        
                                      default:
                                        ret = INVALIDPARAMETER;
                                        break;
                                   */ 
  }
 
  return ( ret );
}

/*********************************************************************
 * @fn      SimpleProfile_GetParameter
 *
 * @brief   Get a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t ExampleService_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
          case SIMPLEPROFILE_CHAR1:{
                          VOID osal_memcpy( value, simpleProfileChar1, SIMPLEPROFILE_CHAR1_ATCLEN );  //simpleProfileChar1 是数组名
                          break;
          }
          
          case SIMPLEPROFILE_CHAR2:{
                          VOID osal_memcpy( value, simpleProfileChar2, SIMPLEPROFILE_CHAR2_ATCLEN );  
                          break;
          }
          

                                   /* case SIMPLEPROFILE_CHAR2:
                                      *((uint8*)value) = simpleProfileChar2;
                                      break;      
                                
                                    case SIMPLEPROFILE_CHAR3:
                                      *((uint8*)value) = simpleProfileChar3;
                                      break;  
                               
                                    case SIMPLEPROFILE_CHAR4:
                                      *((uint8*)value) = simpleProfileChar4;
                                      break;
                                
                                    case SIMPLEPROFILE_CHAR5:
                                      VOID osal_memcpy( value, simpleProfileChar5, SIMPLEPROFILE_CHAR5_LEN );
                                      break;      
                                      
                                    default:
                                      ret = INVALIDPARAMETER;
                                      break;
                                    */
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn          simpleProfile_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 *
 * @return      Success or Failure
 */

static uint8 simpleProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr,uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen )
{
  
//  LCDPrintText("Profile_ReadAttr",0,PRINT_STRING);
  
  
  bStatus_t status = SUCCESS;

  // If attribute permissions require authorization to read, return error
  if ( gattPermitAuthorRead( pAttr->permissions ) )
  {
      LCDPrintText("no permission",1,PRINT_STRING);
  
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)

 
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
            switch ( uuid )
            {

                    case SIMPLEPROFILE_CHAR1_UUID:{

			  if ( offset >=SIMPLEPROFILE_CHAR1_ATCLEN )
    				    return ( ATT_ERR_ATTR_NOT_LONG );

			   // determine read length
        		   *pLen = MIN(maxLen, SIMPLEPROFILE_CHAR1_ATCLEN - offset);			                          
                            osal_memcpy( pValue, &simpleProfileChar1[offset], *pLen );


//						 LCDPrintText("offset",offset,PRINT_VALUE);
//						 LCDPrintText("maxLen",maxLen,PRINT_VALUE);
                        break;
                    }
                        
                    case SIMPLEPROFILE_CHAR2_UUID:{
                           *pLen = 1;
                            pValue[0] = *pAttr->pValue;
                    break;
                    }                    
               
            }
  }

  return ( status );
}

/*****************************************************************************************************************************
 * @fn      simpleProfile_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   complete - whether this is the last packet
 * @param   oper - whether to validate and/or write attribute value  
 *
 * @return  Success or Failure
 *****************************************************************************************************************************/

static bStatus_t simpleProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr, uint8 *pValue, uint8 len, uint16 offset )
{
  
  
//LCDPrintText("Profile_WriteAttr",0,PRINT_STRING);
  
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  
  // If attribute permissions require authorization to write, return error
  if(gattPermitAuthorWrite( pAttr->permissions))     // Insufficient authorization                            
                             return ( ATT_ERR_INSUFFICIENT_AUTHOR );

  
  if(pAttr->type.len == ATT_BT_UUID_SIZE){   // 16-bit UUID    
     uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
            switch (uuid){
                          case SIMPLEPROFILE_CHAR1_UUID:{
                                    if ( offset >= SIMPLEPROFILE_CHAR1_ATCLEN )
                                                      return ( ATT_ERR_ATTR_NOT_LONG );
                                    
                                    // determine read length
                                    uint8 actrual_len = MIN(len, SIMPLEPROFILE_CHAR1_ATCLEN - offset);        		   		                                            
                                    osal_memcpy( &simpleProfileChar1[offset], pValue, actrual_len);	
                                    notifyApp = SIMPLEPROFILE_CHAR1;                                 
                                    break;
                          }
                          
                          /*case SIMPLEPROFILE_CHAR2_UUID:{
                                     if ( offset >= SIMPLEPROFILE_CHAR2_LEN )
                                                      return ( ATT_ERR_ATTR_NOT_LONG );
                                    
                                    // determine read length
                                    uint8 actrual_len = MIN(len, SIMPLEPROFILE_CHAR2_LEN - offset);        		   		                                            
                                    osal_memcpy( &simpleProfileChar2[offset], pValue, actrual_len);	
                                    notifyApp = SIMPLEPROFILE_CHAR2;                                 
                                    break;
                          }*/
                          

                        case GATT_CLIENT_CHAR_CFG_UUID:{			
                        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len, offset, GATT_CLIENT_CFG_NOTIFY );                                                                      
                        notifyApp = SIMPLEPROFILE_CFG_CHAR2;
                        break;
                        }
                          
                        default:
                          // Should never get here! (characteristics 2 and 4 do not have write permissions)
                        status = ATT_ERR_ATTR_NOT_FOUND;
                        break;
                      }
  }
  else
    // 128-bit UUID
    status = ATT_ERR_INVALID_HANDLE;

  // If a charactersitic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && simpleProfile_AppCBs && simpleProfile_AppCBs->pfnSimpleProfileChange )
            simpleProfile_AppCBs->pfnSimpleProfileChange( notifyApp );  //调用回调函数 simpleProfileChangeCB()
  
  return ( status );
}


/********************************************************************************************************
 * @fn          simpleProfile_HandleConnStatusCB
 *
 * @brief       Simple Profile link status change handler function.
 *
 * @param       connHandle - connection handle
 * @param       changeType - type of change
 *
 * @return      none
 *********************************************************************************************************/
static void simpleProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{ 
  // Make sure this is not loopback connection
  if (connHandle != LOOPBACK_CONNHANDLE)
  {
    // Reset Client Char Config if connection has dropped
    if ((changeType == LINKDB_STATUS_UPDATE_REMOVED )||(( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) && ( !linkDB_Up( connHandle ))))
    { 
     GATTServApp_InitCharCfg( connHandle, simpleProfileChar2Config );
    }
  }
}


/************************************************************************************************************
*************************************************************************************************************/
/********************************************************************************************************
 * @fn          ServerNotificationCallBack()
 *
 * @brief       when connected, send notification automaticly!!.
 *
 * @param       connHandle - connection handle
 * @param       changeType - type of change
 *
 * @return      none
 *********************************************************************************************************/
static void ServerNotificationCallBack(linkDBItem_t *pLinkItem)
{ 
  if( pLinkItem->stateFlags & LINK_CONNECTED ){
    uint16 value = GATTServApp_ReadCharCfg( pLinkItem->connectionHandle,simpleProfileChar2Config );
    if ( value & GATT_CLIENT_CFG_NOTIFY ){
      attHandleValueNoti_t noti;

      noti.handle = simpleProfileAttrTbl[CHAR2_CONFIG_INDEX].handle;
      noti.len = SIMPLEPROFILE_CHAR2_ATCLEN;
 //     noti.value = simpleProfileChar2;
      
      osal_memcpy(noti.value, simpleProfileChar2, SIMPLEPROFILE_CHAR2_ATCLEN );
      
      uint8 return_status;
      if(return_status = GATT_Notification( pLinkItem->connectionHandle, &noti, FALSE ))
        LCDPrintText("Notif error",return_status,PRINT_VALUE);
      //else
        //LCDPrintText("Notif ok",0,PRINT_STRING);
        
    }
  }
}




uint16 GetATTCharaHandle(uint8 para){

return(simpleProfileAttrTbl[para].handle);

}

/************************************************************************************************************
*************************************************************************************************************/