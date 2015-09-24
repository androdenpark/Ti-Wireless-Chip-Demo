

/************************************ INCLUDES****************************************************/

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "linkdb.h"
#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"

#include "gatt.h"

#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"

#include "central.h"
#include "peripheral.h"
#include "gapbondmgr.h"

#include "masterSlaveSwitch.h"
#include "auto_switch_central.h"
#include "Auto_switch_peripheral.h"

#include "OSAL_Tasks.h"

#include "z_delay.h"

#include <stdlib.h>
#include "hal_mcu.h"        //add by zyy


#include "exampleService.h"


uint16 record_switch_times=0;

/*********************************************************************
 * GLOBLE VARIABLES
 */
uint8 MasterSlaveSwitchTaskID;   // Task ID for internal task/event processing

uint8 mac_buffer[BUFFER_LEN][MAC_LEN]={0};      //stores the transfer information

uint8 Buffer_Top = 1;           //intiat value:mac_buffer[0] stores it's own mac, mac_buffer[1] stores all zero means end flag.


ConnectionDeviceInfo_t CurrentConnectionInfo;

uint8 random_num=0;

/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8 rand_tims = 0;



// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "BLE role Switch";
// Initial State
static deviceRole_t deviceRole = ROLE_CENTRAL;

//bool deviceRoleDurationContral = FALSE;       //as the flag of one swich 


#define DEFAULT_DESIRED_CONN_TIMEOUT 50       //100 means 1s, it can not be set too short,otherwise when it is in central role, it can not be connected correctly

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void DeviceSwitchPolicy(void);

//char *bdAddr2Str( uint8 *pAddr );

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Bond Manager Callbacks
static gapBondCBs_t masterSlaveSwitch_BondMgrCBs =
{
  NULL,                     // Passcode callback (not used by application)
  NULL                      // Pairing / Bonding state Callback (not used by application)
};

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
void MasterSlaveSwitch_Init( uint8 task_id ){

	MasterSlaveSwitchTaskID = task_id;
        
        //initiat globle variables
        CurrentConnectionInfo.Handle = 0;  
        CurrentConnectionInfo.MacAddr = NULL;

	// Set the GAP Characteristics
	GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );                   
	 //add by zyy:
	GAP_SetParamValue( TGAP_CONN_EST_SUPERV_TIMEOUT, DEFAULT_DESIRED_CONN_TIMEOUT);

	// Setup the GAP Bond Manager
	uint32 passkey = 0; // passkey "000000"
	uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
	uint8 mitm = TRUE;
	uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
	uint8 bonding = TRUE;
	GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
	GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
	GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
	GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
	GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );	
        
        GAPBondMgr_Register( &masterSlaveSwitch_BondMgrCBs );

	
	// Initialize GATT attributes
	GGS_AddService( GATT_ALL_SERVICES );            // GAP
				  	 
	GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
				  
	DevInfo_AddService();
        
        ENABLE_WATCHDOG();        //enable the watchdog timer to monitor the software running status
        //SwitchPeripheral_Init();
        //SwitchCentral_Init();
        
	osal_set_event( MasterSlaveSwitchTaskID, MSS_START_DEVICE_EVT );
        
        UartInitForTest();
        
//        uint8 rand_number;
//        while(1){
//        rand_number=Onboard_rand();
//        HalLcdWriteStringValue("rand_number",rand_number,10,HAL_LCD_LINE_2);
//        HalLcdWriteStringValue("rand_number",rand_number%10,10,HAL_LCD_LINE_3);
//        Z_DelayMS(2000);
//        }
        
	
	
}


/**************************************************************************************** 
 * @fn      MasterSlaveSwitch_ProcessEvent
 *
 * @brief   Master Slave Switch Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 *****************************************************************************************/
uint16 MasterSlaveSwitch_ProcessEvent( uint8 task_id, uint16 events )
{
  VOID task_id; // OSAL required parameter that isn't used in this function

  	if ( events & SYS_EVENT_MSG ){
	//		LCDPrintText("SYS_EVENT_MSG",0,PRINT_STRING); 
    			uint8 *pMsg;
    			if (NULL !=  (pMsg = osal_msg_receive( MasterSlaveSwitchTaskID ))  ) {
                                                if(GATT_MSG_EVENT ==((osal_event_hdr_t *)pMsg)->event ){
                                                            if(deviceRole == ROLE_CENTRAL)
		  							simpleBLECentralProcessGATTMsg( (gattMsgEvent_t *) pMsg );
                                                            else
                                                                        PeripheralProcessGATTMsg((gattMsgEvent_t *) pMsg); 
                                                }
						else
									LCDPrintText("Undef MSG",((osal_event_hdr_t *)pMsg)->event,PRINT_VALUE);  
						
			osal_msg_deallocate( pMsg );				
    			}
				

			return (events ^ SYS_EVENT_MSG);

    			
  	}


  	if ( events & MSS_START_DEVICE_EVT ){			                          
                        SwitchChange_Init();//initiat all the task except the main
                        
                        BroardcastTimeRecord = 0;//intiat the monitor 
                        
                        //LCDPrintText("haha",testnum,PRINT_VALUE);
                        //HalLcdWriteStringValue("haha:",testnum,10,HAL_LCD_LINE_3);
                        
                        if(deviceRole == ROLE_PERIPHERAL){ 
                              deviceRole = ROLE_CENTRAL;    //switch to central ,as it is in ROLE_PERIPHERAL state last time;
                              LCDPrintText("Central: ",Buffer_Top,PRINT_VALUE);
                              //HalLcdWriteStringValue("Central:",Buffer_Top,10,HAL_LCD_LINE_2);
                              //osal_start_timerEx(MasterSlaveSwitchTaskID, DEVICE_SWITCH_EVT, CENTROL_ROLE_TOTOL_TIME);
                              CentralClientInitiat();
                        }
                        else{ 
                              deviceRole = ROLE_PERIPHERAL;
                              LCDPrintText("Peripheral: ",Buffer_Top,PRINT_VALUE);
                              //HalLcdWriteStringValue("Peripheral:",Buffer_Top,10,HAL_LCD_LINE_2);
                              //osal_start_timerEx(MasterSlaveSwitchTaskID, DEVICE_SWITCH_EVT, ROLE_TOTOL_TIME);
                              PeripheralServerInitiat();
                        }
				
    			return ( events ^ MSS_START_DEVICE_EVT );
  	}

  
  	if (events & MSS_CHANGE_ROLE_EVT){			
			
			//osal_stop_timerEx(MasterSlaveSwitchTaskID, DEVICE_SWITCH_EVT);                                               
                        DeviceSwitchPolicy();                        			
			return ( events ^ MSS_CHANGE_ROLE_EVT );         
	
  	}


	/*if(events & MSS_BROADCAST_EVT){					
			osal_set_event(MasterSlaveSwitchTaskID, MSS_CHANGE_ROLE_EVT);
			uint8 adv_enabled_status = FALSE;
    			GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &adv_enabled_status ); // To avoid auto-reenabling of advertising
    			LCDPrintText("no scan request",0,PRINT_STRING);                    										
			return ( events ^ MSS_BROADCAST_EVT );
	}*/
        
        
        
        if(events & CHECH_LINK_EVT){					
                       if(TRUE != linkDB_State(CurrentConnectionInfo.Handle, LINK_CONNECTED))
                              osal_set_event(MasterSlaveSwitchTaskID, MSS_CHANGE_ROLE_EVT);
                       else{
                              uint32 timeout_value = 50;
                              osal_start_timerEx(MasterSlaveSwitchTaskID, CHECH_LINK_EVT, timeout_value);
                              LCDPrintText( "linkDB_State",0,PRINT_VALUE);
                       }
		       return ( events ^ CHECH_LINK_EVT );
	}
        
       /* if(events & PERIOD_DETECH_EVT){
                      //DO nothing but to let the system to feed the watchdog 
                  if(deviceRole == ROLE_PERIPHERAL){
                          BroardcastTimeRecord++;
                          if( BroardcastTimeRecord >= ((random_num%10)+5) ){     //broadcasting time interval:(random_num%10)+5*200                                    
                                    uint8 adv_enabled_status = FALSE;
                                    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &adv_enabled_status ); // To avoid auto-reenabling of advertising
                                    LCDPrintText("no scan request",0,PRINT_STRING); 
                                    BroardcastTimeRecord = 0;
                                    osal_stop_timerEx(MasterSlaveSwitchTaskID,PERIOD_DETECH_EVT);
                                    osal_set_event(MasterSlaveSwitchTaskID, MSS_CHANGE_ROLE_EVT);
                                    return ( events ^ PERIOD_DETECH_EVT ); 
                          }
                  }
                  
                  else{
                        BroardcastTimeRecord++;
                        if(BroardcastTimeRecord >= ((random_num%10)/deviceRole+5)){
                                GAPCentralRole_CancelDiscovery();     //stop discoverying  
                                BroardcastTimeRecord = 0;
                                LCDPrintText("no scan result",0,PRINT_STRING); 
                                osal_stop_timerEx(MasterSlaveSwitchTaskID,PERIOD_DETECH_EVT);
                                osal_set_event(MasterSlaveSwitchTaskID, MSS_CHANGE_ROLE_EVT);
                                return ( events ^ PERIOD_DETECH_EVT ); 
                        }
                  }
                  
                  uint32 timeout_value = 200;
                  osal_start_timerEx(MasterSlaveSwitchTaskID, PERIOD_DETECH_EVT, timeout_value);
                  return ( events ^ PERIOD_DETECH_EVT );  
                  
        }*/
        
        if(events & PERIOD_DETECH_EVT){       //DO nothing but to let the system to feed the watchdog                      
               BroardcastTimeRecord++;
               if(BroardcastTimeRecord >= ((random_num%10)/deviceRole+5)){
                              osal_stop_timerEx(MasterSlaveSwitchTaskID,PERIOD_DETECH_EVT);
                              
                              if(deviceRole == ROLE_CENTRAL){                                  
                                      GAPCentralRole_CancelDiscovery();     //stop discoverying  
                                      LCDPrintText("no scan result",0,PRINT_STRING);
                              }
                              else{
                                    uint8 adv_enabled_status = FALSE;
                                    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &adv_enabled_status ); // To avoid auto-reenabling of advertising
                                    LCDPrintText("no scan request",0,PRINT_STRING);                        
                              }
                              
                                     
                              osal_set_event(MasterSlaveSwitchTaskID, MSS_CHANGE_ROLE_EVT);
                              return ( events ^ PERIOD_DETECH_EVT ); 
               }
                      
                LCDPrintText("PERIOD_DETECH_EVT ",0,PRINT_STRING);
               uint32 timeout_value = 200;
               osal_start_timerEx(MasterSlaveSwitchTaskID, PERIOD_DETECH_EVT, timeout_value);
               return ( events ^ PERIOD_DETECH_EVT );  
    
        }
                                           			                                                           
	else
			LCDPrintText("undefined events",1,PRINT_STRING);          
		
  	// Discard unknown events
  	return 0;
}




/*********************************************************************
 * @fn      bdAddr2Str
 *
 * @brief   Convert Bluetooth address to string. Only needed when
 *          LCD display is used.
 *
 * @return  none
 */
char *bdAddr2Str( uint8 *pAddr )
{
  uint8       i;
  char        hex[] = "0123456789ABCDEF";
  static char str[B_ADDR_STR_LEN];
  char        *pStr = str;

  *pStr++ = '0';
  *pStr++ = 'x';

  // Start from end of addr
  pAddr += B_ADDR_LEN;

  for ( i = B_ADDR_LEN; i > 0; i-- )
  {
    *pStr++ = hex[*--pAddr >> 4];
    *pStr++ = hex[*pAddr & 0x0F];
  }

  *pStr = 0;

  return str;
}





/*********************************************************************/

static void DeviceSwitchPolicy(void){
    
  //clear the watchdog time to begin a new start, 
  //ConnetingFlag=0;
  //WD_KICK();
         
    //clear screen
    for(uint8 i = HAL_LCD_LINE_2; i <= HAL_LCD_LINE_8; ++i) 
	  HalLcdWriteString ("  ", i);
   

/*    for(uint8 i = 0; i<rand_tims; ++i)
	  random_num = rand()%10;

	  rand_tims++;
          rand_tims %= 50;
 */                       
    random_num=Onboard_rand()%10;     //generate random number
    
 /*   if(deviceRole == ROLE_CENTRAL)
		deviceRole = ROLE_PERIPHERAL;
    else
		deviceRole = ROLE_CENTRAL;
 */
                        
    // Z_DelayMS(rand_num*100);
    uint32 timeout_value =300;
    osal_start_timerEx(MasterSlaveSwitchTaskID, MSS_START_DEVICE_EVT, timeout_value);    
    
    record_switch_times++;
    HalLcdWriteStringValue("switch times:",record_switch_times,10,HAL_LCD_LINE_5);
    //HalLcdWriteStringValue("BroardcastTime:",BroardcastTimeRecord,10,HAL_LCD_LINE_6);
    
}