

/************************************ INCLUDES****************************************************/

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "hal_lcd.h"

#include "gatt.h"
#include "hal_mcu.h"

//#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"

#include "central.h"
//#include "peripheral.h"
#include "gapbondmgr.h"

#include "linkdb.h"
#include "masterSlaveSwitch.h"
#include "auto_switch_central.h"
#include "auto_switch_peripheral.h"

//#include "OSAL_Tasks.h"

#include "z_delay.h"

#include <stdlib.h>
#include <string.h >



#include "exampleService.h"



/*********************************************************************
 * LOCAL VARIABLES
 */

#define  MAX_SCAN_RES   5                                                // max scan results
#define  DISCOVERY_MODE   DEVDISC_MODE_GENERAL                           // Discovey mode (limited, general, all)
#define  ACTIVE_SCAN   TRUE                                              // TRUE to use active scan 
#define  SCAN_DURATION   2000                                          // Scan duration in ms
#define  DISCOVERY_WHITE_LIST   FALSE                                    // TRUE to use white list during discovery
#define  LINK_HIGH_DUTY_CYCLE   FALSE                                    // TRUE to use high scan duty cycle when creating link
#define  LINK_WHITE_LIST  FALSE                                         // TRUE to use white list when creating link
//RSSI_PERIOD = 1000                                            // RSSI polling period in ms
#define  DEV_DISC_BY_SVC_UUID   TRUE                                      // TRUE to filter discovery results on desired service UUID 





//static void simpleBLECentralRssiCB( uint16 connHandle, int8  rssi );
static void simpleBLECentralEventCB( gapCentralRoleEvent_t *pEvent );

//static void simpleBLEGATTDiscoveryEvent( gattMsgEvent_t *pMsg );
static void simpleBLECentralEventCB( gapCentralRoleEvent_t *pEvent );

static void simpleBLECentralStartDiscovery( void );

static bool simpleBLEFindSvcUuid( uint16 uuid, uint8 *pData, uint8 dataLen );
static void simpleBLEAddDeviceInfo( uint8 *pAddr, uint8 addrType );

static void ClientWriteValue(void);

void SwitchCentral_Init( void );





static CharacterInfo_t BLEChar=
{
SIMPLEPROFILE_CHAR1_ATCLEN,
0,
SIMPLEPROFILE_CHAR1_UUID,
NULL
}; 


static bool CharHandleSearchFlag =1;      // determine whether to search char handle or not

static uint8 send_record=0;    //record the send times

static uint8 CharSendingFlag = 0;         //as the flag of sending status 

// Number of scan results and scan result index
static uint8 simpleBLEScanRes;
//static uint8 simpleBLEScanIdx;

// Scan result list
static gapDevRec_t simpleBLEDevList[MAX_SCAN_RES];

// Discovered service start and end handle
static uint16 HandleRangeStart = 0;
static uint16 HandleRangeEnd = 0;




// GAP Role Callbacks CENTRAL
static const gapCentralRoleCB_t simpleBLERoleCB =
{
NULL,						//  simpleBLECentralRssiCB,       // RSSI callback
simpleBLECentralEventCB       // Event callback
};



/*********************************************************************************************
 * @fn      simpleBLECentralEventCB
 *
 * @brief   Central event callback function.
 *
 * @param   pEvent - pointer to event structure
 *
 * @return  none
 **********************************************************************************************/
void CentralClientInitiat(void){
  SwitchCentral_Init();
  
  uint8 return_status;
  if(return_status = GAPCentralRole_StartDevice((gapCentralRoleCB_t *) &simpleBLERoleCB))
        LCDPrintText("Central Init Error",return_status,PRINT_VALUE);
  if(return_status = GATT_InitClient())
        LCDPrintText("Client Init Error",return_status,PRINT_VALUE);

  GATT_RegisterForInd(MasterSlaveSwitchTaskID);       
}

/*********************************************************************************************
 * @fn      simpleBLECentralEventCB
 *
 * @brief   Central event callback function.
 *
 * @param   pEvent - pointer to event structure
 *
 * @return  none
 **********************************************************************************************/


void SwitchCentral_Init( void ){

    uint8 scanRes = MAX_SCAN_RES;
    GAPCentralRole_SetParameter ( GAPCENTRALROLE_MAX_SCAN_RES, sizeof( uint8 ), &scanRes );
                
    GAP_SetParamValue( TGAP_GEN_DISC_SCAN, SCAN_DURATION );
    GAP_SetParamValue( TGAP_LIM_DISC_SCAN, SCAN_DURATION );  
    



    //static varibles initiation
    send_record=0;
    HandleRangeStart = 0;
    HandleRangeEnd = 0;
    
    simpleBLEScanRes=0;
    
//   for(uint8 i=0; i<MAX_SCAN_RES; ++i)
//              osal_memcpy( simpleBLEDevList[i].addr, 0, B_ADDR_LEN );


}


/*********************************************************************************************
 * @fn      simpleBLECentralEventCB
 *
 * @brief   Central event callback function.
 *
 * @param   pEvent - pointer to event structure
 *
 * @return  none
 **********************************************************************************************/

static void ClientWriteValue(void){
  
    attWriteReq_t req;  
    
    //one write last one seconds the most
    //WD_KICK();
                                        
    
    if(TRUE == osal_memcmp(mac_buffer[send_record], mac_buffer[send_record+1] ,MAC_LEN))  //check if it is the last value to send
          CharSendingFlag =0;
        
    if(send_record <= Buffer_Top)
        osal_memcpy(req.value, mac_buffer[send_record++], BLEChar.len);    
    else
        return;
    
    req.len = BLEChar.len;
    req.handle = BLEChar.handle;
    req.sig = 0;
    req.cmd = 0;
                   
    uint8 result_value_send;
    if(result_value_send = GATT_WriteCharValue(CurrentConnectionInfo.Handle, &req, MasterSlaveSwitchTaskID ))
        LCDPrintText("send error",result_value_send,PRINT_VALUE); 
      
}


/*********************************************************************************************
 * @fn      simpleBLECentralEventCB
 *
 * @brief   Central event callback function.
 *
 * @param   pEvent - pointer to event structure
 *
 * @return  none
 **********************************************************************************************/
static void simpleBLECentralEventCB( gapCentralRoleEvent_t *pEvent ){
  switch ( pEvent->gap.opcode ){
                case GAP_DEVICE_INIT_DONE_EVENT:{  
       			    //LCDPrintText("Central Initialized",0,PRINT_STRING);
                            osal_memcpy(mac_buffer[0],pEvent->initDone.devAddr, MAC_LEN);        		               
                            HalLcdWriteString(bdAddr2Str( mac_buffer[0]), HAL_LCD_LINE_1); 
			    
                            
                            uint32 random_scan_duration =500;//to avoid normal scan be discarded by the timer,so its lasting-time should be short
                            GAP_SetParamValue( TGAP_GEN_DISC_SCAN, random_scan_duration );        //random scan duration
                            //LCDPrintText("discovering",0,PRINT_STRING);
                            
                            uint32 timeout_value = 200;				// timeout_value - in milliseconds.
                            osal_start_timerEx(MasterSlaveSwitchTaskID, PERIOD_DETECH_EVT, timeout_value);
                            

                            
                            uint8 return_status;
                            if(return_status = GAPCentralRole_StartDiscovery(DISCOVERY_MODE, ACTIVE_SCAN, DISCOVERY_WHITE_LIST ))
                                 	LCDPrintText("discovery error:",return_status,PRINT_VALUE); 
			    break;
                }
      	
      
                case GAP_DEVICE_INFO_EVENT:{						//find a new device 				     			
				// filtering device discovery results based on service UUID
                            //LCDPrintText("find new device",0,PRINT_STRING);   
                            if(simpleBLEScanRes >= MAX_SCAN_RES){
                                            GAPCentralRole_CancelDiscovery();
                                            break;
                            }
                            
                            if ( simpleBLEFindSvcUuid(WANTED_SERVICE_UUID, pEvent->deviceInfo.pEvtData, pEvent->deviceInfo.dataLen) ){
         				simpleBLEAddDeviceInfo( pEvent->deviceInfo.addr, pEvent->deviceInfo.addrType );
                                   //GAPCentralRole_CancelDiscovery();     //stop discoverying                                    
                            }			
			    break;
                }
      		           
   	 	case GAP_DEVICE_DISCOVERY_EVENT:{   			//discaovery has completed 
                           osal_stop_timerEx(MasterSlaveSwitchTaskID,PERIOD_DETECH_EVT);
                            //LCDPrintText("disca completed ",0,PRINT_STRING);    
        		    if ( simpleBLEScanRes > 0 ){
	          			// connect to current device in scan result                                                                             
                                        uint8 random_select = random_num%simpleBLEScanRes;
                                        //LCDPrintText("random_select ",random_select,PRINT_STRING); 
					CurrentConnectionInfo.MacType= simpleBLEDevList[random_select].addrType;
	     		 		CurrentConnectionInfo.MacAddr= simpleBLEDevList[random_select].addr; 
          				uint8 return_status;
                                        if(return_status = GAPCentralRole_EstablishLink( LINK_HIGH_DUTY_CYCLE, LINK_WHITE_LIST, CurrentConnectionInfo.MacType, CurrentConnectionInfo.MacAddr)){
                                                  LCDPrintText("Link Error",return_status,PRINT_VALUE); 
                                                  osal_set_event(MasterSlaveSwitchTaskID, MSS_CHANGE_ROLE_EVT);
                                        } 
			    }

                            else{
					//LCDPrintText("no device found",0,PRINT_STRING);                                        
                                       	osal_set_event(MasterSlaveSwitchTaskID, MSS_CHANGE_ROLE_EVT);     //switch to periperal                                        
                            }


			    break;
      		}
      
    		case GAP_LINK_ESTABLISHED_EVENT:{
      	 		    if ( pEvent->gap.hdr.status == SUCCESS ){ 
                                        
					//LCDPrintText("Connected",0,PRINT_STRING);     				
          				CurrentConnectionInfo.Handle= pEvent->linkCmpl.connectionHandle;  
                                        
                                        if(CharHandleSearchFlag == 1)
         				      simpleBLECentralStartDiscovery(); 
                                        else{
                                              ClientWriteValue();                                              
                                              CharSendingFlag =1;
                                              //LCDPrintText("NO NEED TO",0,PRINT_STRING);  
                                        }
      	 		    }
       			
       			    else{				                                        
                                        LCDPrintText("Connect Failed ",pEvent->gap.hdr.status,PRINT_VALUE);
                                            osal_set_event(MasterSlaveSwitchTaskID, MSS_CHANGE_ROLE_EVT);
       			    	}

			    break;
      		}
      

    		case GAP_LINK_TERMINATED_EVENT:{       
                                        osal_set_event(MasterSlaveSwitchTaskID, MSS_CHANGE_ROLE_EVT);
      			    break;
      		}

    		case GAP_LINK_PARAM_UPDATE_EVENT:
      				break;
      
    		default:
      				break;
  	}
}



/*************************************************************************************************
 * @fn      simpleBLECentralStartDiscovery
 *
 * @brief   Start service discovery.
 *
 * @return  none
 **************************************************************************************************/


static void simpleBLECentralStartDiscovery( void ){

uint8 uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(WANTED_SERVICE_UUID),HI_UINT16(WANTED_SERVICE_UUID ) };
  
  // Initialize cached handles
 // HandleRangeStart = HandleRangeEnd = BLEChar.handle = 0;
  
  // Discovery simple BLE service
  uint8 dis_result;
  if(dis_result = GATT_DiscPrimaryServiceByUUID(CurrentConnectionInfo.Handle, uuid, ATT_BT_UUID_SIZE, MasterSlaveSwitchTaskID )){
            	
    LCDPrintText("GATT error:",0,PRINT_VALUE);             
    //LCDPrintText("switch_time:",record_switch_times,PRINT_VALUE); 
                
  }
}





/************************************************************************************************
 * @fn      simpleBLECentralProcessGATTMsg
 *
 * @brief   Process GATT messages
 *
 * @return  none
 ************************************************************************************************/
void simpleBLECentralProcessGATTMsg( gattMsgEvent_t *pMsg ){
  
  switch(pMsg->method){
        case ATT_READ_RSP:{   
              uint8 valueRead = pMsg->msg.readRsp.value[0];
              LCDPrintText("Read rsp:",valueRead,PRINT_VALUE); 
              
              //GAPCentralRole_TerminateLink( connHandle ); 
              break;
        }
  
        case ATT_WRITE_RSP:{  
              if(CharSendingFlag ==1){                    //to filt out the last rep message!
                         Z_DelayMS(50);
                         ClientWriteValue(); 
              }
              break;
        }

        case ATT_FIND_BY_TYPE_VALUE_RSP:{         
                                if(pMsg->msg.findByTypeValueRsp.numInfo > 0 ){
                                            //LCDPrintText("Found group h",0,PRINT_STRING);    
                                            HandleRangeStart = pMsg->msg.findByTypeValueRsp.handlesInfo[0].handle;          //Found attribute handle
                                            HandleRangeEnd = pMsg->msg.findByTypeValueRsp.handlesInfo[0].grpEndHandle;      //Group end handle
                                }
                                
                                if(pMsg->hdr.status == bleProcedureComplete)
                                           if( HandleRangeEnd != 0 ){ 
                                                attReadByTypeReq_t req;
                                                req.startHandle = HandleRangeStart;
                                                req.endHandle = HandleRangeEnd;
                                                req.type.len = ATT_BT_UUID_SIZE;                                                   
                                                req.type.uuid[0] = LO_UINT16(BLEChar.uuid);
                                                req.type.uuid[1] = HI_UINT16(BLEChar.uuid);           
                                                uint8 return_status;
                                                //LCDPrintText("seach attrib h",0,PRINT_STRING); 
                                                if(return_status = GATT_ReadUsingCharUUID(CurrentConnectionInfo.Handle, &req, MasterSlaveSwitchTaskID ))// Discover characteristic
                                                                LCDPrintText("ReadUsingChar error",return_status,PRINT_VALUE);                                                                                                                                       
                                          }
                                          else
                                                LCDPrintText("Handle range error",1,PRINT_STRING);           
                                break;
          
        }
  
        case ATT_READ_BY_TYPE_RSP:{
          
                                if(pMsg->msg.readByTypeRsp.numPairs > 0 ){
                                        uint16 temp_handle = BUILD_UINT16( pMsg->msg.readByTypeRsp.dataList[0], pMsg->msg.readByTypeRsp.dataList[1] );
                                        if((temp_handle == BLEChar.handle) && (BLEChar.handle != 0))
                                        {CharHandleSearchFlag = 0;
                                        //HalLcdWriteStringValue("HANDLE SA",temp_handle,10,HAL_LCD_LINE_5);
                                        }
                                        else
                                        {BLEChar.handle= temp_handle;	
                                        //HalLcdWriteStringValue("HANDLE DE",temp_handle,10,HAL_LCD_LINE_6);
                                        }
                                        
                                }      			
		    		
                                if(pMsg->hdr.status == bleProcedureComplete)
                                        if(BLEChar.handle != 0){                             
                                              ClientWriteValue();
                                              CharSendingFlag =1;
                                              //LCDPrintText("finished attrib h",0,PRINT_STRING);
                                        }
                                        else 
                                              LCDPrintText("Handle value error",1,PRINT_STRING);  
                                break;        
        }

	case ATT_HANDLE_VALUE_NOTI:{
          
                               //one transfer last 1s the most ,otherwise watchdog will reset the device
                                        //WD_KICK();
                                        
                                if(pMsg->msg.handleValueNoti.len != MAC_LEN){
                                        LCDPrintText("error noti lenth:",pMsg->msg.handleValueNoti.len,PRINT_VALUE);
                                        break;
                                }
                                
                                osal_memcpy(mac_buffer[Buffer_Top],pMsg->msg.handleValueNoti.value,pMsg->msg.handleValueNoti.len);
                                
                                if(TRUE == osal_memcmp(mac_buffer[Buffer_Top], mac_buffer[Buffer_Top+1] ,MAC_LEN)){
                                      
                                        
                                        GAPCentralRole_TerminateLink( CurrentConnectionInfo.Handle );
                                        uint32 timeout_value = 50;
                                        //osal_start_timerEx(MasterSlaveSwitchTaskID, CHECH_LINK_EVT, timeout_value);
                                        
                                        break;
                                }//wait for conn timeout
          
                                if(TRUE == MacBufferSearch(mac_buffer[Buffer_Top])){
                                        //LCDPrintText("valid value:",mac_buffer[Buffer_Top][0],PRINT_VALUE);                                                                            
                                        Buffer_Top++;                                        
                                        
                                }
                                
                                else
                                        osal_memset(mac_buffer[Buffer_Top], 0, MAC_LEN);
                                
                                break;
        }
        
        
        case ATT_ERROR_RSP:{
           
                        if(pMsg->msg.errorRsp.reqOpcode == ATT_READ_REQ){
                                      uint8 status = pMsg->msg.errorRsp.errCode;
                              LCDPrintText("Read Error",status,PRINT_VALUE);
                        }
          
                        else if(pMsg->msg.errorRsp.reqOpcode == ATT_WRITE_REQ){
                                      uint8 status = pMsg->msg.errorRsp.errCode;
                              LCDPrintText("Write Error",status,PRINT_VALUE);
                        }
                        
                        else if(pMsg->msg.errorRsp.reqOpcode == ATT_FIND_BY_TYPE_VALUE_REQ)
                              LCDPrintText("service find error",1,PRINT_STRING);
                        
                        else if(pMsg->msg.errorRsp.reqOpcode == ATT_READ_BY_TYPE_REQ)
                              LCDPrintText("charac find error",1,PRINT_STRING);
                        
                        break;
        }
        
        default:
                LCDPrintText("unknow gatt message",1,PRINT_STRING);
                break;


   
  } 

			
}



/************************************************************************************************
 * @fn      simpleBLEFindSvcUuid
 *
 * @brief   Find a given UUID in an advertiser's service UUID list.
 *
 * @return  TRUE if service UUID found
 ************************************************************************************************/
static bool simpleBLEFindSvcUuid( uint16 uuid, uint8 *pData, uint8 dataLen )
{
  uint8 adLen;
  uint8 adType;
  uint8 *pEnd;
  
  pEnd = pData + dataLen - 1;
  
  // While end of data not reached
  while ( pData < pEnd )
  {
    // Get length of next AD item
    adLen = *pData++;
    if ( adLen > 0 )
    {
      adType = *pData;
      
      // If AD type is for 16-bit service UUID
      if ( adType == GAP_ADTYPE_16BIT_MORE || adType == GAP_ADTYPE_16BIT_COMPLETE )
      {
        pData++;
        adLen--;
        
        // For each UUID in list
        while ( adLen >= 2 && pData < pEnd )
        {
          // Check for match
          if ( pData[0] == LO_UINT16(uuid) && pData[1] == HI_UINT16(uuid) )
          {
            // Match found
            return TRUE;
          }
          
          // Go to next
          pData += 2;
          adLen -= 2;
        }
        
        // Handle possible erroneous extra byte in UUID list
        if ( adLen == 1 )
        {
          pData++;
        }
      }
      else
      {
        // Go to next item
        pData += adLen;
      }
    }
  }
  
  // Match not found
  return FALSE;
}

/*********************************************************************
 * @fn      simpleBLEAddDeviceInfo
 *
 * @brief   Add a device to the device discovery result list
 *
 * @return  none
 *********************************************************************/
static void simpleBLEAddDeviceInfo( uint8 *pAddr, uint8 addrType )
{
  uint8 i;
  
  // If result count not at max
  if ( simpleBLEScanRes < MAX_SCAN_RES )
  {
    // Check if device is already in scan results
    for ( i = 0; i < simpleBLEScanRes; i++ )
    {
      if ( osal_memcmp( pAddr, simpleBLEDevList[i].addr , B_ADDR_LEN ) )
      {
        return;
      }
    }
    
    // Add addr to scan result list
    osal_memcpy( simpleBLEDevList[simpleBLEScanRes].addr, pAddr, B_ADDR_LEN );
    simpleBLEDevList[simpleBLEScanRes].addrType = addrType;
    
    // Increment scan result count
    simpleBLEScanRes++;
  }
}

/*********************************************************************
 * @fn      simpleBLEAddDeviceInfo
 *
 * @brief   Add a device to the device discovery result list
 *
 * @return  none
 *********************************************************************/

bool MacBufferSearch(uint8 *value){
  
  if(Buffer_Top<BUFFER_LEN){

      for(uint8 i=0; i<Buffer_Top; ++i)
                if(TRUE == osal_memcmp(mac_buffer[i],value,MAC_LEN))
                        return(FALSE);             //   the buffer contains this value,so throw it,
                else
                        continue;
      
      return(TRUE);   //the buffer do not contains this value,
  }
  else
      return(FALSE);  //the buffer has no area

}