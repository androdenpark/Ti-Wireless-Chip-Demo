/******************************************************************************
*
*   Copyright (C) 2013 Texas Instruments Incorporated
*
*   All rights reserved. Property of Texas Instruments Incorporated.
*   Restricted rights to use, duplicate or disclose this code are
*   granted through contract.
*
*   The program may not be used without the written permission of
*   Texas Instruments Incorporated or against the terms and conditions
*   stipulated in the agreement under which this program has been supplied,
*   and under no circumstances can it be used with non-TI connectivity device.
*
******************************************************************************/
//****************************************************************************
//
// main.c - getting started wlan in sta mode
//
//****************************************************************************
//****************************************************************************
//
//! \addtogroup getting_started_sta
//! @{
//
//****************************************************************************
#include "hw_types.h"
#include "hw_ints.h"
#include "rom_map.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "simplelink.h"
#include "protocol.h"
#include "uart_logger.h"
#include <stdlib.h>
#include <string.h>
#ifdef ewarm
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "osi.h"
#include "interrupt.h"

#include "pin.h"
#include "prcm.h"
#include "utils.h"
#include "systick.h"
#include "wlan.h"
#include "pinmux.h"
#include "netcfg.h"
#include "z_delay.h"


#include "libemqtt.h"

/*********** from tcp_socket**********/
#include "stdlib.h"
#include "string.h"

#define BUF_SIZE 1400
#define IP_ADDR         SL_IPV4_VAL(9,168,88,87)
#define PORT_NUM      1883    // 
/*********** from tcp_socket**********/




#define UART_PRINT              Report

#define SSID_NAME               "heatonli_mac"

//#define SECURITY_TYPE           SL_SEC_TYPE_OPEN         
#define SECURITY_TYPE           SL_SEC_TYPE_OPEN 

#define SECURITY_KEY            "passw0rd"
#define WEP_KEY_ID              1
    
#define UNUSED(x)   x = x
#define SPAWN_TASK_PRIORITY     4
//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//****************************************************************************
void vAssertCalled( const char *pcFile, unsigned long ulLine );
void vApplicationIdleHook();
void SimpleLinkWlanEventHandler(void *pWlanEvents);
void SimpleLinkNetAppEventHandler(void *pNetAppEvent);
void SimpleLinkPingReport(SlPingReport_t *pPingReport);
void WlanConnect();
void InitCallback();
int PingTest(unsigned long ulDefaultGateway);
void WlanStationMode( void *pvParameters);


void taskTest2(void *pvParameters);
void taskTest1(void *pvParameters);


/*********** from tcp_socket**********/
int BsdTcpClient(unsigned short usPort);

int BsdTcpServer(unsigned short usPort);
/*********** from tcp_socket**********/

















//******************************************************************************
//			GLOBAL VARIABLES
//******************************************************************************
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

unsigned int g_uiSimplelinkstarted = 0;
unsigned int g_uiIpObtained = 0;
unsigned int g_uiConnectionStatus = 0;
unsigned long g_uiPingPacketsRecv = 0;
unsigned int g_uiPingDone = 0;





static unsigned long clientTaskHandle = 0;
static unsigned long ServerTaskHandle = 0;








/*********** from tcp_socket**********/

char g_cBsdBuf[BUF_SIZE];

/*********** from tcp_socket**********/

//*****************************************************************************
//
//! Application defined hook (or callback) function - assert
//!
//! \param  none
//! 
//! \return none
//!
//*****************************************************************************
void
vAssertCalled( const char *pcFile, unsigned long ulLine )
{
	while(1)
	{
	}
}

//*****************************************************************************
//
//! Application defined idle task hook
//! 
//! \param  none
//! 
//! \return none
//!
//*****************************************************************************
void
vApplicationIdleHook( void)
{

}

//****************************************************************************
//
//!	\brief This function handles WLAN events
//!
//! \param  pWlanEvents is the event passed to the handler
//!
//! \return None
//
//****************************************************************************
void SimpleLinkWlanEventHandler(void *pWlanEvents)
{
    switch(((SlWlanEvent_t*)pWlanEvents)->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
           g_uiConnectionStatus = 1;
           break;
        case SL_WLAN_DISCONNECT_EVENT:
           g_uiConnectionStatus = 0;
           break;
        default:
          break;
    }
}

//****************************************************************************
//
//!	\brief This function handles events for IP address acquisition via DHCP
//!		   indication
//!
//! \param  pNetAppEvent is the event passed to the Handler
//!
//! \return None
//
//****************************************************************************
void SimpleLinkNetAppEventHandler(void *pNetAppEvent)
{
	SlNetAppEvent_t *pNetApp = (SlNetAppEvent_t *)pNetAppEvent;

	switch( pNetApp->Event )
	{
	case SL_NETAPP_IPV4_ACQUIRED:
          g_uiIpObtained = 1;
          break;
	default:
		break;
	}
}

//****************************************************************************
//
//!	\brief call back function for the ping test
//!
//!	\param  pPingReport is the pointer to the structure containing the result
//!         for the ping test
//!
//!	\return None
//
//****************************************************************************
void SimpleLinkPingReport(SlPingReport_t *pPingReport)
{
	g_uiPingDone = 1;
	g_uiPingPacketsRecv = pPingReport->PacketsReceived;
	
	UART_PRINT("this ping end now :"); 
	Z_NumDispaly(g_uiPingPacketsRecv,2);
	 
}

//*****************************************************************************
//
//! This function gets triggered when HTTP Server receives Application
//! defined GET and POST HTTP Tokens.
//!
//! \param pHttpServerEvent Pointer indicating http server event
//! \param pHttpServerResponse Pointer indicating http server response
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkHttpServerCallback(void *pvParam1, void *pvParam2)
{
}

//*****************************************************************************
//
//! Mandatory MCU Initialization Routine
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
void MCUInit(void)
{
    unsigned long ulRegVal;

    //
    // DIG DCDC NFET SEL and COT mode disable
    //
    HWREG(0x4402F010) = 0x30031820;
    HWREG(0x4402F00C) = 0x04000000;

    UtilsDelay(32000);

    //
    // ANA DCDC clock config
    //
    HWREG(0x4402F11C) = 0x099;
    HWREG(0x4402F11C) = 0x0AA;
    HWREG(0x4402F11C) = 0x1AA;

    //
    // PA DCDC clock config
    //
    HWREG(0x4402F124) = 0x099;
    HWREG(0x4402F124) = 0x0AA;
    HWREG(0x4402F124) = 0x1AA;

    //
    // Enable RTC
    //
    if(MAP_PRCMSysResetCauseGet()== PRCM_POWER_ON)
    {
        HWREG(0x4402F804) = 0x1;
    }

    //
    // TD Flash timing configurations in case of MCU WDT reset
    //

    if((HWREG(0x4402D00C) & 0xFF) == 0x00000005)
    {
        HWREG(0x400F707C) |= 0x01840082;
        HWREG(0x400F70C4)= 0x1;
        HWREG(0x400F70C4)= 0x0;
    }

    //
    // JTAG override for I2C in SWD mode
    //
    if(((HWREG(0x4402F0C8) & 0xFF) == 0x2))
    {
        MAP_PinModeSet(PIN_19,PIN_MODE_2);
        MAP_PinModeSet(PIN_20,PIN_MODE_2);
        HWREG(0x4402E184) |= 0x1;
    }

    //
    // Take I2C semaphore,##IMPROTANT:REMOVE IN PG1.32 DEVICES##
    //
    ulRegVal = HWREG(COMMON_REG_BASE + COMMON_REG_O_I2C_Properties_Register);
    ulRegVal = (ulRegVal & ~0x3) | 0x1;
    HWREG(COMMON_REG_BASE + COMMON_REG_O_I2C_Properties_Register) = ulRegVal;

    //
    // Take GPIO semaphore##IMPROTANT:REMOVE IN PG1.32 DEVICES##
    //
    ulRegVal = HWREG(COMMON_REG_BASE + COMMON_REG_O_GPIO_properties_register);
    ulRegVal = (ulRegVal & ~0x3FF) | 0x155;
    HWREG(COMMON_REG_BASE + COMMON_REG_O_GPIO_properties_register) = ulRegVal;

    //
    // Change UART pins(55,57) mode to PIN_MODE_0 if they are in PIN_MODE_1(NWP mode)
    //
    if((MAP_PinModeGet(PIN_55)) == PIN_MODE_1)
    {
        MAP_PinModeSet(PIN_55,PIN_MODE_0);
    }
    if((MAP_PinModeGet(PIN_57)) == PIN_MODE_1)
    {
        MAP_PinModeSet(PIN_57,PIN_MODE_0);
    }
    
     //
    // Enable Peripheral Clocks 
    //
    MAP_PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);

    //
    // Configure PIN_55 for UART0 UART0_TX
    //
    MAP_PinTypeUART(PIN_55, PIN_MODE_3);

    //
    // Configure PIN_57 for UART0 UART0_RX
    //
    MAP_PinTypeUART(PIN_57, PIN_MODE_3);
    
    InitTerm();         //set the parameters of uart0

}

//****************************************************************************
//
//!	\brief sl_start call back function confirming the simplelink start
//!
//!	\param  None
//!
//!	\return None
//
//****************************************************************************
void InitCallback(UINT32 uiParams)
{
    g_uiSimplelinkstarted = 1;
}
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************


static unsigned long IntTimes = 0;

void sysTickCallBack(void){  
  
  IntTimes++;  
}

void sysTickInit(void){
SysTickEnable();
SysTickIntEnable();
SysTickIntRegister(sysTickCallBack);
SysTickPeriodSet(0xffff);
}

//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************
//
//!	\brief pings to the default gateway and ip address of domain "www.ti.com"
//!
//! This function pings to the default gateway to ensure the wlan cannection,
//! then check for the internet connection, if present then get the ip address
//! of Domain name "www.ti.com" and pings to it
//!
//!	\param  ulDefaultGateway is the default gateway for the ap to which the
//!         device is connected
//!
//!	\return -1 for unsuccessful LAN connection, -2 for problem with internet
//!         conection and 0 for succesful ping to the Domain name
//
//****************************************************************************
int PingTest(unsigned long ulDefaultGateway)
{
    int iStatus = 0;
    SlPingStartCommand_t PingParams;
    SlPingReport_t PingReport;
    unsigned long ulIpAddr;
	char *pinghostname = "www.sina.com.cn";
//	unsigned short pinghostname_len = strlen(pinghostname);
	
    // Set the ping parameters
	PingParams.PingIntervalTime = 100;
	PingParams.PingSize = 5;
	PingParams.PingRequestTimeout = 1000;
	PingParams.TotalNumberOfAttempts = 10;
	PingParams.Flags = 0;//?if flags parameter is set to 0, ping will report back once all requested pings are done 
	PingParams.Ip = SL_AF_INET;
    // Fill the GW IP address, which is our AP address
	//PingParams.Ip = ulDefaultGateway;
	PingParams.Ip =SL_IPV4_VAL(192,168,1,1);
    // Check for LAN connection 
    sl_NetAppPingStart((SlPingStartCommand_t*)&PingParams, SL_AF_INET,
                 (SlPingReport_t*)&PingReport, SimpleLinkPingReport);

	
    while (!g_uiPingDone)
	{
		//looping till ping is running
              Z_DelayS(1);
    
        UART_PRINT("i am waiting ping end now \n\r"); 
    
	}
	
    
    g_uiPingDone = 0;


	if(g_uiPingPacketsRecv)
	{

	UART_PRINT("We were able to ping now \n\r"); 
	
		g_uiPingPacketsRecv = 0;
		// We were able to ping the AP

		/* Check for Internet connection */
		/* Querying for ti.com IP address */
		iStatus = sl_NetAppDnsGetHostByName(pinghostname, strlen(pinghostname), &ulIpAddr, SL_AF_INET);
		if (iStatus < 0)
		{

		UART_PRINT("Problem with Internet connection \n\r"); 
		
			// LAN connection is successful
			// Problem with Internet connection
			return -2;
		}


		// Replace the ping address to match ti.com IP address
		PingParams.Ip = ulIpAddr;
                
                UART_PRINT(pinghostname); 

		UART_PRINT(" ip is :  "); 
		Z_IPDispaly(&ulIpAddr);
                
                return 0;  ///not to ping  www.ti.com

		// Try to ping www.ti.com
		sl_NetAppPingStart((SlPingStartCommand_t*)&PingParams, SL_AF_INET,
                     (SlPingReport_t*)&PingReport, SimpleLinkPingReport);

		while (!g_uiPingDone)
		{
			//looping till ping is running
		}
        
        if (g_uiPingPacketsRecv)
		{
			// LAN connection is successful
			// Internet connection is successful
			g_uiPingPacketsRecv = 0;
			return 0;
		}
		else
		{
			// LAN connection is successful
			// Problem with Internet connection
			return -2;
		}
                

    }
	else
	{
		// Problem with LAN connection
		UART_PRINT(" Problem with LAN connection \n\r"); 
		return -1;
	}
}

//****************************************************************************
//
//!	\brief Connecting to a WLAN Accesspoint
//!
//!	This function connects to the required AP (SSID_NAME) with Security
//! parameters specified in te form of macros at the top of this file
//!
//!	\param[in] 			        None
//!
//!	\return	                    None
//!
//!	\warning	If the WLAN connection fails or we don't aquire an IP address,
//!				We will be stuck in this function forever.
//
//****************************************************************************
static char wlanConnectStatus = 0;

void WlanConnect()
{
   SlSecParams_t secParams;
   secParams.Key = SECURITY_KEY;
   secParams.KeyLen = strlen(SECURITY_KEY);
   secParams.Type = SECURITY_TYPE;

  // sl_WlanConnect(SSID_NAME, strlen(SSID_NAME), 0, &secParams, 0);
   
   sl_WlanConnect(SSID_NAME, strlen(SSID_NAME), 0, NULL, 0);

   while ((g_uiConnectionStatus == 0) || (g_uiIpObtained == 0))
   {
       //looping till ip is acquired
   }
   
   wlanConnectStatus = 1;
   UART_PRINT("connected to Ap   \n\r"); 
}

//****************************************************************************
//
//!	\brief start simplelink, connect to the ap and run the ping test
//!
//! This function starts the simplelink, connect to the ap and start the ping
//! test on the default gateway for the ap
//!
//!	\param  pvparameters is the pointer to the list of parameters that can be
//!         passed to the task while creating it
//!
//!	\return None
//
//****************************************************************************
//void WlanStationMode( void *pvParameters )

static unsigned long MyIP=0;



void WlanStationMode( void *pvParameters )
{
    int iTestResult = 0;
    unsigned long ulIP = 0;
    unsigned long ulSubMask = 0;
    unsigned long ulDefaultGateway = 0;
    unsigned long ulDNSServer = 0;
    unsigned char ucDHCP = 0;
    char cMode;
    char* buff="hhhhhh\n\r";
    unsigned char currentMacAddress[SL_MAC_ADDR_LEN];
    
    //UART_PRINT(" in WlanStationMode  \n\r"); 
    
/*  for(int a = 0; a<5; ++a){
    Z_DelayS(30);
    *buff=a+'0';
    UART_PRINT(buff);   
    }
 */   
    
    

    
    
    
    
    
    
 
    
    
  char deviceRole = ROLE_STA;  
    
  deviceRole = sl_Start(NULL,NULL,InitCallback);
 /* deviceRole = sl_Start(NULL,NULL,NULL);
    
    if(deviceRole < 0 ){
      UART_PRINT("sl_Start error  \n\r");
      return;
    }
    else if(deviceRole == ROLE_STA)
        UART_PRINT("in station mode  \n\r"); 
          else{
            UART_PRINT("in wrong mode  \n\r"); 
            return;
          }
 */     
    
 /*  if((cMode = sl_Start(NULL,NULL,NULL)) != ROLE_STA){
      UART_PRINT("hellow  \n\r"); 
      
      *buff=cMode+'0';
      UART_PRINT(buff);
      *buff=ROLE_STA+'0';
      UART_PRINT(buff);
      
 //     SL_WLAN_SET_MODE_STA();
    }
     
    UART_PRINT("hellow i am in station mode now \n\r"); 
    
  */   
    
 while(!g_uiSimplelinkstarted)
 {
        //looping till simplelink starts
  Z_DelayS(1);
  // ; 
   UART_PRINT("i am starting now \n\r"); 
 } 
             
   UART_PRINT("i am started   \n\r");          
             
    // Connecting to WLAN AP - Set with static parameters defined at the top
	// After this call we will be connected and have IP address */
    WlanConnect();
  

  
    
   UART_PRINT("i'm connected! \n\r"); 
    
    //get mac addr from s-flash
    
    SL_MAC_ADDR_GET(currentMacAddress);   
    Z_MACDispaly(currentMacAddress);
    
    
    SL_STA_IPV4_ADDR_GET(&ulIP,&ulSubMask,&ulDefaultGateway,&ulDNSServer,
                       &ucDHCP);
    
    
    
    Z_IPDispaly(&ulIP);
    MyIP=ulIP;
    Z_IPDispaly(&ulSubMask);
    Z_IPDispaly(&ulDefaultGateway);

    
    
    
    /*
    
    UNUSED(ulIP);
    UNUSED(ulSubMask);
    UNUSED(ulDNSServer);
    UNUSED(ucDHCP);
    */
    
   // iTestResult = PingTest(ulDefaultGateway);
  //  UNUSED(iTestResult);
    
 //BsdTcpServer(PORT_NUM); 
    
 //mqtt_pub();
    UART_PRINT("WlanStationMode\r\n"); 
    vTaskDelete(NULL);
    UART_PRINT("WlanStationMode\r\n"); 
 return;
 
 
    
    
    
    
    
}




//****************************************************************************
//
//!	\brief Opening a TCP server side socket and receiving data
//!
//!	This function opens a TCP socket in Listen mode and waits for an incoming
//!	TCP connection.
//! If a socket connection is established then the function will try to read
//!	1000 TCP packets from the connected client.
//!
//! \param[in] 		 port number on which the server will be listening on
//!
//! \return	         0 on success, -1 on error.
//!
//!	\note            This function will wait for an incoming connection till
//!					 one is established
//
//****************************************************************************

static char serverCreatOK = 0;

void taskServer( void *pvParameters ){
BsdTcpServer(PORT_NUM);
UART_PRINT("taskServer over\r\n"); 
vTaskDelete(NULL);
}

void taskClient( void *pvParameters ){
BsdTcpClient(PORT_NUM);
UART_PRINT("taskClient over\r\n"); 
vTaskDelete(NULL);
}


int BsdTcpClient(unsigned short usPort)
{
  
      UART_PRINT("BsdTcpClient\r\n"); 
  
  while(serverCreatOK == 0) {
        //looping till simplelink starts
  Z_DelayMS(10);
  // ; 
   UART_PRINT("serverCreatOK == 0 \n\r"); 
 } 
 
 UART_PRINT("serverCreatOK == 1\r\n"); 
 
 
    int             iCounter;
    short           sTestBufLen;
    SlSockAddrIn_t  sAddr;
    int             iAddrSize;
    int             iSockID;
    int             iStatus;
    long            lLoopCount = 0;
    long            lBytesSent = 0;
        long            lNonBlocking = 0;                   //0 :non-blocking,

    // filling the buffer
    for (iCounter=0 ; iCounter<BUF_SIZE ; iCounter++)
    {
        g_cBsdBuf[iCounter] = (char)(iCounter % 10);
    }

    sTestBufLen  = BUF_SIZE;

    //filling the TCP server socket address
    sAddr.sin_family = SL_AF_INET;
    sAddr.sin_port = sl_Htons((unsigned short)PORT_NUM);
    sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)SL_IPV4_VAL(192,168,2,5));

    iAddrSize = sizeof(SlSockAddrIn_t);
    

    
    

    // creating a TCP socket
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    if( iSockID < 0 )
    {
        // error
       return -1;
    }
    
    Report("creating a TCP socket yes\r\n");
    
    iStatus = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &lNonBlocking, sizeof(lNonBlocking));
    
    
    Report("connecting to TCP server\r\n");

    // connecting to TCP server
    while(0>sl_Connect(iSockID, ( SlSockAddr_t *)&sAddr, iAddrSize)){
      Report("connecting to TCP server error\r\n");
     // vTaskSuspend(clientTaskHandle);
      vTaskResume((xTaskHandle)&ServerTaskHandle);
      
    }
    /*
    iStatus = sl_Connect(iSockID, ( SlSockAddr_t *)&sAddr, iAddrSize);
    if( iStatus < 0 )
    {
        // error
    	return -1;
    }
    */
    
    Report("connecting to TCP server yes\r\n");
    
    

    // sending 1000 packets to the TCP server
    while (lLoopCount < 1)
    {
    	// sending packet
        iStatus = sl_Send(iSockID, g_cBsdBuf, sTestBufLen, 0 );
 /*       if( iStatus <= 0 )
        {
            // error
        	return -1;
        }
        */
        
vTaskResume((xTaskHandle)&ServerTaskHandle);

        lLoopCount++;
        lBytesSent += iStatus;
    }

    //closing the socket after sending 1000 packets
    sl_Close(iSockID);
    
    Report("closing the socket after sending 1000 packets\r\n");
    
    
    
    
     //filling the TCP server socket address
    sAddr.sin_family = SL_AF_INET;
    sAddr.sin_port = sl_Htons((unsigned short)1883);
    sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)SL_IPV4_VAL(9,186,88,87));

    iAddrSize = sizeof(SlSockAddrIn_t);   
    
    
    // creating a TCP socket
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    if( iSockID < 0 )
    {
        // error
       return -1;
    }
    
    Report("creating a TCP socket yes\r\n");
    
    iStatus = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &lNonBlocking, sizeof(lNonBlocking));
    
    
    Report("connecting to TCP server\r\n");

    // connecting to TCP server
    while(0>sl_Connect(iSockID, ( SlSockAddr_t *)&sAddr, iAddrSize)){
      Report("connecting to TCP server error\r\n");
      
    }
    /*
    iStatus = sl_Connect(iSockID, ( SlSockAddr_t *)&sAddr, iAddrSize);
    if( iStatus < 0 )
    {
        // error
    	return -1;
    }
    */
    
    Report("connecting to TCP server yes\r\n");
    
    
    //filling the TCP server socket address
    sAddr.sin_family = SL_AF_INET;
    sAddr.sin_port = sl_Htons((unsigned short)1883);
    sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)SL_IPV4_VAL(9,186,88,87));

    iAddrSize = sizeof(SlSockAddrIn_t);   
    
    
    // creating a TCP socket
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    if( iSockID < 0 )
    {
        // error
       return -1;
    }
    
    Report("creating a TCP socket yes\r\n");
    
    lNonBlocking =1;
    iStatus = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &lNonBlocking, sizeof(lNonBlocking));
    
    
    Report("connecting to TCP server\r\n");

    // connecting to TCP server
    
    while(0>sl_Connect(iSockID, ( SlSockAddr_t *)&sAddr, iAddrSize)){
      Report("connecting to TCP server error\r\n");
      
    }
    /*
    iStatus = sl_Connect(iSockID, ( SlSockAddr_t *)&sAddr, iAddrSize);
    if( iStatus < 0 )
    {
        // error
    	return -1;
    }
    */
    
    Report("connecting to TCP server yes\r\n");
    
    
    
    
    
    

    return 0;
}




int BsdTcpServer(unsigned short usPort)
{
  
    UART_PRINT("BsdTcpServer\r\n"); 
    while( wlanConnectStatus == 0);
  
  
  
    SlSockAddrIn_t  sAddr;
    SlSockAddrIn_t  sLocalAddr;
    int             iCounter;
    int             iAddrSize;
    int             iSockID;
    int             iStatus;
    int             iNewSockID;
    unsigned long            lLoopCount = 0;
    long            lBytesSent = 0;
    long            lNonBlocking = 0;                   //0 :non-blocking,
    int             iTestBufLen;

    // filling the buffer
    for (iCounter=0 ; iCounter<BUF_SIZE ; iCounter++)
    {
        g_cBsdBuf[iCounter] = (char)(iCounter % 10) + '0';
    }

    iTestBufLen  = BUF_SIZE;

    //filling the TCP server socket address
    sLocalAddr.sin_family = SL_AF_INET;
   sLocalAddr.sin_port = sl_Htons((unsigned short)usPort);
   sLocalAddr.sin_addr.s_addr = 0;
   
//	sLocalAddr.sin_port = usPort;
//	sLocalAddr.sin_addr.s_addr = SL_IPV4_VAL(192,168,1,101);
    

   

   
    // creating a TCP socket
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    if( iSockID < 0 )
    {
      
      // UART_PRINT("error at creating a TCP socket ! \n\r"); 
       
        // error
        return -1;
    }
    
 //   UART_PRINT("iSockID :"); 
 //   Z_NumDispaly(iSockID, 2);
        

    iAddrSize = sizeof(SlSockAddrIn_t);

    // binding the TCP socket to the TCP server address
    iStatus = sl_Bind(iSockID, (SlSockAddr_t *)&sLocalAddr, iAddrSize);
    if( iStatus < 0 )
    {
    
 //   UART_PRINT("error at binding the TCP socket to the TCP server address ! \n\r"); 
     
      // error
    	return -1;
    }
    
    
//	UART_PRINT("binding the TCP socket to the TCP server address ok! \n\r"); 
        
        

    // putting the socket for listening to the incoming TCP connection
    iStatus = sl_Listen(iSockID, 0);
    if( iStatus < 0 )
    {
      
 //     UART_PRINT("error at putting the socket for listening to the incoming TCP connection ! \n\r"); 
    	return -1;
    }

//	UART_PRINT("listen end! \n\r"); 

    // setting socket option to make the socket as non blocking
    iStatus = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &lNonBlocking, sizeof(lNonBlocking));
    iNewSockID = SL_EAGAIN;

    char uttMessage[50]={0};
       snprintf(uttMessage,sizeof(uttMessage),"IntTimes:%d, TickValue:%d,\n\r",IntTimes, SysTickValueGet());
       UART_PRINT(uttMessage); 
 
 serverCreatOK = 1;
 
UART_PRINT(" waiting for an incoming TCP connection! \n\r"); 



    // waiting for an incoming TCP connection
    while( iNewSockID < 0 )
    {
    	// accepts a connection form a TCP client, if there is any
    	// otherwise returns SL_EAGAIN
       iNewSockID = sl_Accept(iSockID, ( struct SlSockAddr_t *)&sAddr, (SlSocklen_t*)&iAddrSize);
       if( iNewSockID == SL_EAGAIN )
       {
         UtilsDelay(10000);
         
             char uttMessage[50]={0};
       snprintf(uttMessage,sizeof(uttMessage),"IntTimes:%d, TickValue:%d,\n\r",IntTimes, SysTickValueGet());
       UART_PRINT(uttMessage); 
       
 //      vTaskResume((xTaskHandle)&clientTaskHandle);
       vTaskSuspend((xTaskHandle)&ServerTaskHandle);
       
//	  UART_PRINT(" iNewSockID == SL_EAGAIN! \n\r"); 
       }
       /*
       else if( iNewSockID < 0 )
       {
    	  // error
    	   UART_PRINT(" iNewSockID < 0! \n\r"); 
    	   return -1;
       }*/
    }
    
                 char utttMessage[50]={0};
       snprintf(utttMessage,sizeof(utttMessage),"IntTimes:%d, TickValue:%d,\n\r",IntTimes, SysTickValueGet());
       UART_PRINT(utttMessage); 


	    UART_PRINT("connect succeed the new iSockID :"); 
    		Z_NumDispaly(iSockID, 5);

	unsigned long the_client_ip = sl_BIGtoLITTLE_l( (unsigned long)sAddr.sin_addr.s_addr );


	UART_PRINT("the client ip is :"); 
	Z_IPDispaly(&the_client_ip);


	unsigned short the_client_port = sl_BIGtoLITTLE_S( (unsigned short)sAddr.sin_port );
        


	UART_PRINT("the client port is :"); 
	Z_NumDispaly( (unsigned long)the_client_port,5);
	


/*
UART_PRINT(" waits for 1000 packets from the connected TCP client! \n\r"); 

    // waits for 1000 packets from the connected TCP client
    while (lLoopCount < 1000)
    {
        iStatus = sl_Recv(iNewSockID, g_cBsdBuf, iTestBufLen, 0);
        if( iStatus <= 0 )
		{
			// error
        	return -1;
		}

		lLoopCount++;
		lBytesSent += iStatus;
    }
  */

/*

    // sending 3 packets to the TCP server
    while (lLoopCount < 3)
    {
    	// sending packet
 //       iStatus = sl_Send(iNewSockID, g_cBsdBuf, iTestBufLen, 0 );

	char *send_buffer = "hellow i am cc3200 , welcome to wifi world !\n\r";

	 iStatus = sl_Send(iNewSockID, send_buffer, strlen(send_buffer), 0 );
        if( iStatus <= 0 )
        {
        	UART_PRINT("error at sending packet\n\r"); 
		 Z_NumDispaly(lLoopCount,5);
            // error
        	return -1;
        }

        lLoopCount++;
        lBytesSent += iStatus;




    }

/*
//#define SL_POLICY_CONNECTION    (0x10)
//#define SL_POLICY_SCAN          (0x20)
//#define SL_POLICY_PM            (0x30)    
    
    
//    while(1){
//   Z_DelayS(1);
//    char OutMessage[50]={0};
//    snprintf(OutMessage,sizeof(OutMessage),"IntTimes:%d, TickValue:%d,\n\r",IntTimes, SysTickValueGet());
//    sl_Send(iNewSockID, OutMessage, strlen(OutMessage), 0 );        
//    }   
    
    
 
    
    char *setbuffer= "\n set policy \n\r";
sl_Send(iNewSockID, setbuffer, strlen(setbuffer), 0 );

char recBuffer[2] = {0};
char numBuffer[15]={0};

while(1){
  
char sl_policy = '0';
char set_prar = '0';
char OutMessage[50] = {0};
char num = 0;
                
                
                
                while(1){
                  
                  char temp_num = sl_Recv(iNewSockID, recBuffer, sizeof(recBuffer), 0);                 
                 
                  if(temp_num>0)
                    num += temp_num;
                  
                  if(num == 1)
                      sl_policy = recBuffer[0]-'0';
                      
                  
                  if(num == 3)
                    set_prar = recBuffer[0]-'0';
                  
                  if(num>3 && (recBuffer[0]=='\r' || recBuffer[0]=='\n'))
                    break;
                    
                  
                  if(num>3){
                    num = 0;
                  
                  char *send_buffer= "\nplease input again!!!\n\r";
                    sl_Send(iNewSockID, send_buffer, strlen(send_buffer), 0 );}
                  
                  
                  snprintf(numBuffer,sizeof(numBuffer),"num:%d\n\r",num);
                  UART_PRINT(numBuffer);
                   
                }
                
                char *send_buffer= "\nfinished input!\n\r";
                sl_Send(iNewSockID, send_buffer, strlen(send_buffer), 0 );
                
                



                switch(sl_policy*16){
                    case SL_POLICY_CONNECTION://1 x 
                          snprintf(OutMessage,sizeof(OutMessage),"CONN= first:%d, second:%d,\n\r",sl_policy, set_prar);
                          UART_PRINT(OutMessage);
                          break;
                          
                    case SL_POLICY_SCAN://2 x 
                          snprintf(OutMessage,sizeof(OutMessage),"SCAN= first:%d, second:%d,\n\r",sl_policy, set_prar);
                          if(set_prar == 0)
                                  sl_WlanPolicySet(SL_POLICY_SCAN,0,0,0);
                          else
                                  sl_WlanPolicySet(SL_POLICY_SCAN,1,(unsigned char *)&set_prar,sizeof(set_prar));
                          
                          UART_PRINT(OutMessage);
                          break;
                          
                case SL_POLICY_PM://3 x : x=0(normal),x=1(latency),x=2(low),x=3(always on),
                          snprintf(OutMessage,sizeof(OutMessage),"PM= first:%d, second:%d,\n\r",sl_policy, set_prar);
                          UART_PRINT(OutMessage);
                          sl_WlanPolicySet(SL_POLICY_PM , set_prar, NULL,0);
                          break;
                                                   
                case 0x00://0 x :finished set 
                          break;
                          
                case 0x40:{//4xx: hibernate
                                  char *send_buffer= "\nsl_hibernate!\n\r";
                sl_Send(iNewSockID, send_buffer, strlen(send_buffer), 0 );
                   sl_Stop(0);
                   break;}
                   
                case 0x50:{//4xx: hibernate
                                  char *send_buffer= "\ndevice_hibernate!\n\r";
                sl_Send(iNewSockID, send_buffer, strlen(send_buffer), 0 );
                   PRCMHibernateEnter();
                   break;}
                          
                    default:
                          snprintf(OutMessage,sizeof(OutMessage),"Error= first:%d, second:%d,\n\r",sl_policy, set_prar);
                          UART_PRINT(OutMessage);
                          break;
                            
                }
                
                
                if(sl_policy == 0)
                  break;

}





  
  
Sl_WlanNetworkEntry_t netEntries[20];
 char message[80];




/****no scan*********/
 /* 
#define SL_SCAN_DISABLE  0
sl_WlanPolicySet(SL_POLICY_SCAN,SL_SCAN_DISABLE,0,0);

while(1);
return 0;







    
 UINT8  intervalInSeconds=1;  
    

 while(1){
        Z_DelayS(1); 
        sl_WlanPolicySet(SL_POLICY_SCAN,SL_SCAN_POLICY_EN(1), (unsigned char *)&intervalInSeconds,sizeof(intervalInSeconds));
 }
    
    
     
*/ 
 
 /*
char *noticebuffer= "\n set send message time \n\r";
sl_Send(iNewSockID, noticebuffer, strlen(noticebuffer), 0 );

 
while(1){
                  
                  char temp_num = sl_Recv(iNewSockID, recBuffer, sizeof(recBuffer), 0);                 
                 
                  if(temp_num>0)
                    break;                  
                   
}
                
    

 while(1){  
    
   

    //Get Scan Result
   UINT8 Index = sl_WlanGetNetworkList(0,20,&netEntries[0]);

  
    for(UINT8 i=0; i< Index; i++)
    {
         snprintf(message, 60, "%d) SSID %s  RSSI %d \n\r",i,netEntries[i].ssid,netEntries[i].rssi);
	//UART_PRINT(message); 
        sl_Send(iNewSockID, message, strlen(message), 0 );
        
        
        
        
    }  
    
  Z_DelayS(recBuffer[0]-'0');  

  }    
    









    // close the connected socket after receiving from connected TCP client
    sl_Close(iNewSockID);

    // close the listening socket
    sl_Close(iSockID);

    return 0;
    
    */
}




//*****************************************************************************
//							MAIN FUNCTION
//*****************************************************************************
void main()
{
  
  
    
#if defined(ewarm)
    IntVTableBaseSet((unsigned long)&__vector_table);
#endif

    //
    // Board Initialization
    //
    MCUInit();
    //
    
    
    // Enable the SYSTICK interrupt
    //
    IntEnable(FAULT_SYSTICK);
    
    sysTickInit();
    
    
    
	//
	// Start the SimpleLink Host
	//
    VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
	//
   // WlanStationMode(NULL);
      
  //  UART_PRINT("hellow i am in\n\r");
    
	// Start the WlanStationMode task
	//

    if(OSI_OK !=osi_TaskCreate( WlanStationMode,
    				(const signed char*)"WlanStationMode task",
    				2048, NULL, 3, NULL ))
      UART_PRINT("WlanStationMode failed\n\r");   
        
   if(OSI_OK != osi_TaskCreate( taskServer,
    				(const signed char*)"taskServer",
    				2048, NULL, 2, (OsiTaskHandle)&ServerTaskHandle ) )
     UART_PRINT("taskServer failed\n\r");
    
    if(OSI_OK != osi_TaskCreate( taskClient,
    				(const signed char*)"taskClient",
    				2048, NULL, 1, (OsiTaskHandle)&clientTaskHandle ))
      UART_PRINT("taskClient failed\n\r");
    
    
 /*   
    if(OSI_OK != osi_TaskCreate( taskTest1,
    				(const signed char*)"taskTest1",
    				2048, NULL, 1, NULL ))
      UART_PRINT("taskTest1 failed\n\r"); 
    
    if(OSI_OK != osi_TaskCreate( taskTest2,
    				(const signed char*)"taskTest2",
    				2048, NULL, 1, NULL ))
      UART_PRINT("taskTest1 failed\n\r"); 
*/       
    
	//
	// Start the task scheduler
	//
    //osi_start();
    vTaskStartScheduler();
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************



void taskTest1(void *pvParameters){
  
/*  for(char i = 0;i<5;i++){
    
      Z_DelayS(1);
  // ; 
   UART_PRINT("taskTest1\n\r");
  }*/
  
  vTaskDelete(NULL);
}


void taskTest2(void *pvParameters){
  
 /* for(char i = 0;i<5;i++){
    
      Z_DelayS(1);
  // ; 
   UART_PRINT("taskTest2\n\r");
  }*/
 
  vTaskDelete(NULL);
  
}









