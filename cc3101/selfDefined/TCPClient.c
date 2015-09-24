
/****************************************************************************
****************************************************************************/

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


#include "TCPClient.h"

/****************************************************************************/

#define SERVER_ADDR             SL_IPV4_VAL(9,186,57,116)                      //server,broker ip addr
#define TCP_DEFAULT_PORT_NUM                80                                              //default tcp port

static ClientInfo_t ClientInfo[MAX_CLIENT_NUM];

static uint8 CurrentClientNum = 0;


/****************************************************************************/


int BsdTcpClient(ClientInfo_t *SpecificClientInfo)
{
  
  
    if(CurrentClientNum >= MAX_CLIENT_NUM){
      	UART_PRINT("error at creating a client ! \n\r"); 
        // error
       return -1;       
    }
  
     //filling the TCP server socket address
    SlSockAddrIn_t  sAddr;
    
 //   sAddr.sin_family = SL_AF_INET;	
 //   sAddr.sin_port = sl_Htons((unsigned short)TCP_DEFAULT_PORT_NUM);
 //   sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)SERVER_ADDR);
  iAddrSize = sizeof(SlSockAddrIn_t);
  sAddr.sin_family = SL_AF_INET;	
  sAddr.sin_port = SpecificClientInfo.port;
  sAddr.sin_addr.s_addr = SpecificClientInfo.brokerIP;
    
    
    
    // creating a TCP socket
    SpecificClientInfo.socketID = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    if( SpecificClientInfo.socketID < 0 )
    {

	UART_PRINT("error at creating a TCP socket ! \n\r"); 
        // error
       return -1;
    }
    
    
    // connecting to TCP server 
    if( 0 > sl_Connect(SpecificClientInfo.socketID, ( SlSockAddr_t *)&sAddr, iAddrSize))
    {
	UART_PRINT("error at connecting to TCP server ! \n\r"); 
        // error
    	return -1;
    }  

    
 return  1;
 
}
    






    
    int             iCounter;
    short           sTestBufLen;
    
    int             iAddrSize;
    int             iSockID;
    int             iStatus;
    unsigned long            lLoopCount = 0;
    long            lBytesSent = 0;

    // filling the buffer
    for (iCounter=0 ; iCounter<BUF_SIZE ; iCounter++)
    {
        g_cBsdBuf[iCounter] = (char)(iCounter % 10);
    }
    sTestBufLen  = BUF_SIZE;






	//UART_PRINT("iSockID :"); 
	//Z_NumDispaly(iSockID, 2);





	UART_PRINT("  connected to TCP server ok! \n\r"); 

    // sending 1000 packets to the TCP server
    while (lLoopCount < 1000)
    {
    	// sending packet
        iStatus = sl_Send(iSockID, g_cBsdBuf, sTestBufLen, 0 );
        if( iStatus <= 0 )
        {
        	UART_PRINT("error at sending packet"); 
		 Z_NumDispaly(lLoopCount,2);
            // error
        	return -1;
        }

        lLoopCount++;
        lBytesSent += iStatus;
    }

    //closing the socket after sending 1000 packets
    sl_Close(iSockID);

    return 0;
}


