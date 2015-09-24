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
// main.c - getting started wlan in ap mode
//
//****************************************************************************
//****************************************************************************
//
//! \addtogroup getting_started_ap
//! @{
//
//****************************************************************************
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "rom_map.h"
#include "simplelink.h"
#include "protocol.h"
#include <stdlib.h>
#include <string.h>
#ifdef ewarm
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "osi.h"
#include "interrupt.h"

#include "uart_logger.h"
#include "z_delay.h"

#include "pin.h"
#include "prcm.h"
#include "utils.h"


#define UART_PRINT              Report
#define UNUSED(x)   x = x
#define SPAWN_TASK_PRIORITY     4
//******************************************************************************
//							GLOBAL VARIABLES
//******************************************************************************
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

unsigned int g_uiSimplelinkstarted = 0;
unsigned int g_uiIpObtained = 0;
unsigned int g_uiStaConnected = 0;
unsigned int g_uiIpLeased = 0;
unsigned long g_ulStaIp = 0;
unsigned int g_uiPingPacketsRecv = 0;
unsigned int g_uiPingDone = 0;
unsigned int g_uiMode = 0;

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

//*****************************************************************************
//
//! On Successful completion of Wlan Connect, This function triggers Connection
//! status to be set. 
//!
//! \param  WlanEvent pointer indicating Event type
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(void *pWlanEvents)
{
    switch(((SlWlanEvent_t*)pWlanEvents)->Event)
    {      
        case SL_WLAN_STA_CONNECTED_EVENT :
            g_uiStaConnected = 1;     
            break;
            
        case SL_WLAN_STA_DISCONNECTED_EVENT:
            g_uiStaConnected = 0;     
            break;
        
        default:
            break;
    }
}

//*****************************************************************************
//
//! This function gets triggered when device acquires IP
//! status to be set. When Device is in DHCP mode recommended to use this.
//!
//! \param NetAppEvent Pointer indicating device aquired IP 
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(void *pNetAppEvent)
{

	switch(((SlNetAppEvent_t*)pNetAppEvent)->Event)
	{
	case SL_NETAPP_IPV4_ACQUIRED:
        g_uiIpObtained = 1;
        break;
          
        case SL_NETAPP_IP_LEASED:
        g_uiIpLeased = 1;
        g_ulStaIp = ((SlNetAppEvent_t*)pNetAppEvent)->EventData.ipLeased.ip_address;
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

    /*                                                                                                  delete by zyy
    //
    // JTAG override for I2C in SWD mode
    //
    if(((HWREG(0x4402F0C8) & 0xFF) == 0x2))
    {
        MAP_PinModeSet(PIN_19,PIN_MODE_2);
        MAP_PinModeSet(PIN_20,PIN_MODE_2);
        HWREG(0x4402E184) |= 0x1;
    }
    */
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
void InitCallback(UINT32 uiParam)
{
    g_uiSimplelinkstarted = 1;
}

//****************************************************************************
//
//!	\brief device will try to ping the machine that has just connected to the
//!		   device.
//!
//!	\param  ulIpAddr is the ip address of the station which has connected to
//!			device
//!
//!	\return 0 if ping is successful, -1 for error
//
//****************************************************************************
int PingTest(unsigned long ulIpAddr)
{  
    SlPingStartCommand_t PingParams;
    SlPingReport_t PingReport;
    PingParams.PingIntervalTime = 1000;
    PingParams.PingSize = 20;
    PingParams.PingRequestTimeout = 3000;
    PingParams.TotalNumberOfAttempts = 3;
    PingParams.Flags = 0;
    PingParams.Ip = ulIpAddr; /* STA's ip address
                                        which is our AP address */
    /* Check for LAN connection */
    sl_NetAppPingStart((SlPingStartCommand_t*)&PingParams, SL_AF_INET,
                 (SlPingReport_t*)&PingReport, SimpleLinkPingReport);
    while (!g_uiPingDone)
    {
    }
    if (g_uiPingPacketsRecv)
    {
        /* LAN connection is successful */
        return(0);
    }
    else
    {
        return(-1);
    }    
}
//****************************************************************************
//
//!	\brief start simplelink, wait for the sta to connect to the device and 
//!        run the ping test for that sta
//!
//!	\param  pvparameters is the pointer to the list of parameters that can be
//!         passed to the task while creating it
//!
//!	\return None
//
//****************************************************************************
void WlanAPMode( void *pvParameters )
{   
    int iTestResult = 0;
    unsigned long ulIpAddr = 0;
    unsigned long ulIP;
    unsigned long ulSubnetMask;
    unsigned long ulDefaultGateway;
    unsigned long ulDNSServer;
    unsigned char ucDHCP;
    sl_Start(NULL,NULL,InitCallback);
    while (g_uiIpObtained == 0)
    {
       //looping till ip is acquired
      Z_DelayS(1);
      UART_PRINT("g_uiIpObtained == 0 now \n\r");   
       
    }
    
    SL_STA_IPV4_ADDR_GET(&ulIP,&ulSubnetMask,&ulDefaultGateway,&ulDNSServer,
                       &ucDHCP);
    while((g_uiIpLeased == 0) || (g_uiStaConnected == 0))
    {
        //wating for the STA to connect
    }
    
    ulIpAddr = g_ulStaIp;
    iTestResult = PingTest(ulIpAddr);
    UNUSED(ulIP);
    UNUSED(ulSubnetMask);
    UNUSED(ulDefaultGateway);
    UNUSED(ulDNSServer);
    UNUSED(ucDHCP);
    UNUSED(iTestResult);
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
    InitTerm();         //set the parameters of uart0
    //
    // Enable the SYSTICK interrupt
    //
    IntEnable(FAULT_SYSTICK);
	//
	// Start the SimpleLink Host
	//
    VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
	//
	// Start the WlanAPMode task
	//
    osi_TaskCreate( WlanAPMode,
    				(const signed char*)"wireless LAN in AP mode",
    				2048, NULL, 1, NULL );
	//
	// Start the task scheduler
	//
    osi_start();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
