

/************************************ INCLUDES****************************************************/

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "hal_lcd.h"

#include "gatt.h"
#include "linkdb.h"


#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"


#include "peripheral.h"
#include "gapbondmgr.h"

#include "masterSlaveSwitch.h"
#include "auto_switch_central.h"
#include "auto_switch_peripheral.h"

#include "OSAL_Tasks.h"

#include "z_delay.h"

#include <stdlib.h>
#include <string.h >


#include "exampleService.h"

/*********************************************************************/

static void peripheralStateNotificationCB( gaprole_States_t newState );
static void exampleServiceChangeCB( uint8 paramID );
void SwitchPeripheral_Init( void );



//advertising interval must between 20ms to 10.24s
#define ADVERTISING_INTERVAL   80                                                // What is the advertising interval when device is discoverable (units of 625us, 80=50ms)
#define UPDATE_REQUEST   TRUE 								// Whether to enable automatic parameter update request when a connection is formed
#define DISCOVERABLE_MODE   GAP_ADTYPE_FLAGS_GENERAL 		                  // General discoverable mode advertises indefinitely
#define MIN_CONN_INTERVAL   8 								// Minimum connection interval (units of 1.25ms, 8=10ms) if automatic parameter update request is enabled
#define MAX_CONN_INTERVAL   16 							// Maximum connection interval (units of 1.25ms, 16=20ms) if automatic parameter update request is enabled
#define SLAVE_LATENCY  1 										// Slave latency to use if automatic parameter update request is enabled
//#define CONN_TIMEOUT  100 									// Supervision timeout value (units of 10ms, 100=1s) if automatic parameter update request is enabled



//globle varibles
uint8 BroardcastTimeRecord =0;

/*********************************************************************/


// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // complete name
  14,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'M',  'a',  's',  't',  'e',  'r',  'S',  'l',  'a',  'v',  'e',  'S',  'w',
  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16( MIN_CONN_INTERVAL ),   // 100ms
  HI_UINT16( MIN_CONN_INTERVAL ),
  LO_UINT16( MAX_CONN_INTERVAL ),   // 1s
  HI_UINT16( MAX_CONN_INTERVAL ),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // service UUID, to notify central devices what services are included
  // in this peripheral
  0x03,   // length of this data
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
  LO_UINT16( WANTED_SERVICE_UUID ), // Device Info. Completely un-interesting
  HI_UINT16( WANTED_SERVICE_UUID ),
};





// GAP Role Callbacks PERIPHERAL
static gapRolesCBs_t masterSlaveSwitch_PeripheralCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller (not used by application)
};




// Example Service Callbacks
static simpleProfileCBs_t masterSlaveSwitch_exampleServiceCBs =
{
  exampleServiceChangeCB    // Charactersitic value change callback
};








/**************************************************************************************** 
*****************************************************************************************/

void PeripheralServerInitiat(void){
  SwitchPeripheral_Init(); 
  uint8 return_status;
  if(return_status = GAPRole_StartDevice(&masterSlaveSwitch_PeripheralCBs))
        LCDPrintText("Periph Init Er",return_status,PRINT_VALUE);
  if(return_status = GATT_InitServer())
        LCDPrintText("Server Init Error",return_status,PRINT_VALUE);
}

/******************************************************************************************
 * @fn      MasterSlaveSwitch_Init
 *
 * @brief   Initialization function for the Master Slave Switch App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 ******************************************************************************************/
void SwitchPeripheral_Init( void )
{
	    // By setting this to zero, the device will go into the waiting state after
	    // being discoverable for 30.72 second, and will not being advertising again
	    // until the enabler is set back to TRUE
	    uint16 gapRole_AdvertOffTime = 0;

	    // We set this to FALSE to bypass peripheral.c's auto-start
	    uint8 initial_advertising_enable = FALSE; 
	    // Set the GAP Role Parameters
	    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
	    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );

	    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
	    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );
            
            uint8 update_request = UPDATE_REQUEST;
	    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &update_request);
            
            uint8 min_conn_interval = MIN_CONN_INTERVAL;
	    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &min_conn_interval);
            
            uint8 max_conn_interval= MAX_CONN_INTERVAL;
	    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &max_conn_interval);
            
            uint8 slave_latency=SLAVE_LATENCY;
	    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &slave_latency);

	    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, ADVERTISING_INTERVAL);
	    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, ADVERTISING_INTERVAL);
	    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, ADVERTISING_INTERVAL);
	    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, ADVERTISING_INTERVAL);


	    ExampleService_AddService();
	    ExampleService_RegisterAppCBs( &masterSlaveSwitch_exampleServiceCBs );//registered function will Called when characteristic value changes


}




/************************************************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 ************************************************************************************************/
static void peripheralStateNotificationCB( gaprole_States_t newState ){

        switch ( newState ){
	  	
                            case GAPROLE_STARTED:{				   
			          	//LCDPrintText("Perip Initialized",0,PRINT_STRING);                                 
			          	uint8 advertising_enable = TRUE;		
			          	GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof(uint8), &advertising_enable );//start advertising
			          	break;
                            }    

                            case GAPROLE_ADVERTISING:{   
			          	//LCDPrintText("Advertising",0,PRINT_STRING); 
                                        //osal_stop_timerEx(MasterSlaveSwitchTaskID, DEVICE_SWITCH_EVT);
                                        //osal_set_event(MasterSlaveSwitchTaskID, PERIOD_DETECH_EVT);
                                                                             
				   	uint32 timeout_value = 200;				// timeout_value - in milliseconds.	
			   	        osal_start_timerEx(MasterSlaveSwitchTaskID, PERIOD_DETECH_EVT, timeout_value);
                              
			      	   	break;
                            }

                            case GAPROLE_CONNECTED:{
                                        
                                        //wait 1s the most before data wirte in
                                        //ConnetingFlag=1;
                                        //WD_KICK();
                                        
                                        osal_stop_timerEx(MasterSlaveSwitchTaskID,PERIOD_DETECH_EVT);
                                        //osal_stop_timerEx(MasterSlaveSwitchTaskID,MSS_BROADCAST_EVT);
                                        //osal_start_timerEx(MasterSlaveSwitchTaskID, DEVICE_SWITCH_EVT, ROLE_TOTOL_TIME); 
			     	   	//LCDPrintText("Connected",0,PRINT_STRING); 
					uint8 advertising_enable = FALSE;		
			          	GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof(uint8), &advertising_enable );
                                        GAPRole_GetParameter(GAPROLE_CONNHANDLE, &CurrentConnectionInfo.Handle);                                       
                                        GAPRole_GetParameter(GAPROLE_CONN_BD_ADDR, CurrentConnectionInfo.MacAddr);
                                        GATTServApp_WriteCharCfg(CurrentConnectionInfo.Handle, simpleProfileChar2Config, GATT_CLIENT_CFG_NOTIFY);
                                        
			     	   	break;
                            }
			    

                            case GAPROLE_WAITING:         // Disconnected or stopped adv
			    		break;

			    
			    case GAPROLE_WAITING_AFTER_TIMEOUT:  		// Disconnected due to superv. timeout
			       	        //LCDPrintText("conn timeout",0,PRINT_STRING);
					
                                        if(TRUE == linkDB_State(CurrentConnectionInfo.Handle, LINK_CONNECTED)){
                                                GAPRole_TerminateConnection();
                                                uint32 timeout_value = 10;
                                                osal_start_timerEx(MasterSlaveSwitchTaskID, CHECH_LINK_EVT, timeout_value);
                                        }
                                      
                                        else
                                          osal_set_event(MasterSlaveSwitchTaskID, MSS_CHANGE_ROLE_EVT);
			          	break;
			    

			    case GAPROLE_ERROR:
                              	        LCDPrintText("GAPROLE_ERROR",1,PRINT_STRING);//stop running
					osal_set_event(MasterSlaveSwitchTaskID, MSS_CHANGE_ROLE_EVT);
			          	break;

			    default:
                                        LCDPrintText("Undefined state",1,PRINT_STRING);
			         	break;
	 }

}


/***********************************************************************************************
 * @fn      exampleServiceChangeCB
 *
 * @brief   Callback from exampleService indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 ***********************************************************************************************/
static void exampleServiceChangeCB( uint8 paramID ){
  
//  LCDPrintText( "ServiceChangeCB",0,PRINT_VALUE);
	  switch( paramID ){
	      case SIMPLEPROFILE_CHAR1:{
                        
                        //one transfer last one seconds the most
                        //WD_KICK();
                          //LCDPrintText("E_CHAR1",0,PRINT_STRING);
                          
                          
	      		ExampleService_GetParameter( SIMPLEPROFILE_CHAR1, mac_buffer[Buffer_Top]); 
                        
                        //LCDPrintText(bdAddr2Str( mac_buffer[Buffer_Top]), 0, PRINT_STRING); 
                        
                        if(TRUE == osal_memcmp(mac_buffer[Buffer_Top],mac_buffer[(Buffer_Top+1)],MAC_LEN)){
                                      //LCDPrintText("end_CHAR1",0,PRINT_STRING); 
                                      
                                     // if(mac_buffer[Buffer_Top][0]!= 0)                         
                                         // LCDPrintText("chech error",mac_buffer[Buffer_Top][0],PRINT_VALUE);
                        
	                              if(GATT_CLIENT_CFG_NOTIFY != GATTServApp_ReadCharCfg(CurrentConnectionInfo.Handle,simpleProfileChar2Config)){
	                                          LCDPrintText("Noti not config",1,PRINT_STRING);
	                                          break;
	                              }
	                              //start notification!!!!
	                              for(uint8 i=0; i<= Buffer_Top; ++i){
	                                            Z_DelayMS(20);
	                                            attHandleValueNoti_t noti;
	                                            noti.handle =GetATTCharaHandle(CHAR2_CONFIG_INDEX);
	                                            noti.len = MAC_LEN;
	                                            osal_memcpy(noti.value,mac_buffer[i], noti.len);
	                                            uint8 return_status;
	                                            if(return_status = GATT_Notification(CurrentConnectionInfo.Handle, &noti, FALSE )){
	                                                  LCDPrintText("Noti error",return_status,PRINT_VALUE);
								 i--;
	                                            }
					 	
	                              }
	                               
	                              //LCDPrintText( "Device Num: ",Buffer_Top,PRINT_VALUE);
                                      //LCDPrintText("E_NOTI",0,PRINT_STRING); 
                                      
                                      //GAPRole_TerminateConnection();
                                      uint32 timeout_value = 50;
                                      osal_start_timerEx(MasterSlaveSwitchTaskID, CHECH_LINK_EVT, timeout_value);
                                                
                                      
                                      //for(uint8 a = 0; a< Buffer_Top; a++)
                                            //HalLcdWriteString(bdAddr2Str( mac_buffer[a]), HAL_LCD_LINE_1+a);                                             
                                            
                                            //GAPRole_TerminateConnection();
                                            //osal_set_event(MasterSlaveSwitchTaskID, CHECH_LINK_EVT);
                                      
                                      //while(TRUE != linkDB_State(CurrentConnectionInfo.Handle, LINK_NOT_CONNECTED))
                                       //     LCDPrintText( "linkDB_State",0,PRINT_VALUE);
                                            
	                              //osal_set_event(MasterSlaveSwitchTaskID, MSS_CHANGE_ROLE_EVT);              
	                              break;                       
                     
                        }
                                                                                         
                        if(TRUE == MacBufferSearch(mac_buffer[Buffer_Top])){
                                      Buffer_Top++;
                                      //LCDPrintText("valid value",mac_buffer[Buffer_Top][0],PRINT_VALUE);
                                      
                                      
                        }
			else
			   	      osal_memset(mac_buffer[Buffer_Top], 0, MAC_LEN);

	      		break;
	      	}
                
          case SIMPLEPROFILE_CHAR2:{
                uint8 newValue[SIMPLEPROFILE_CHAR2_LEN];
//          	ExampleService_GetParameter( SIMPLEPROFILE_CHAR2, newValue );
                
                /*Read the client characteristic configuration for a given client.*/
//                GATTServApp_ReadCharCfg(connHandle, gattCharCfg_t *charCfgTbl );

	       	LCDPrintText( "Char2 changed ",0,PRINT_VALUE);
                
          break;
          }
          
          
          case SIMPLEPROFILE_CFG_CHAR2:
                LCDPrintText( "Char2 Config changed ",0,PRINT_VALUE);
                break;

	    default:
	      		break;
	  }
}



/************************************************************************************************
 * @fn      PeripheralProcessGATTMsg
 *
 * @brief   Process GATT messages
 *
 * @return  none
 ************************************************************************************************/
void PeripheralProcessGATTMsg( gattMsgEvent_t *pMsg ){
  
  LCDPrintText("gatt message",pMsg->method,PRINT_VALUE);  
  return;			
}

