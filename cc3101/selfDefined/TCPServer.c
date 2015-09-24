

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
int BsdTcpServer(unsigned short usPort)
{
    SlSockAddrIn_t  sAddr;
    SlSockAddrIn_t  sLocalAddr;
    int             iCounter;
    int             iAddrSize;
    int             iSockID;
    int             iStatus;
    int             iNewSockID;
    unsigned long            lLoopCount = 0;
    long            lBytesSent = 0;
    long            lNonBlocking = 1;
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
      
       UART_PRINT("error at creating a TCP socket ! \n\r"); 
       
        // error
        return -1;
    }
    
    UART_PRINT("iSockID :"); 
    Z_NumDispaly(iSockID, 2);
        

    iAddrSize = sizeof(SlSockAddrIn_t);

    // binding the TCP socket to the TCP server address
    iStatus = sl_Bind(iSockID, (SlSockAddr_t *)&sLocalAddr, iAddrSize);
    if( iStatus < 0 )
    {
    
    UART_PRINT("error at binding the TCP socket to the TCP server address ! \n\r"); 
     
      // error
    	return -1;
    }
    
    
	UART_PRINT("binding the TCP socket to the TCP server address ok! \n\r"); 
        
        

    // putting the socket for listening to the incoming TCP connection
    iStatus = sl_Listen(iSockID, 0);
    if( iStatus < 0 )
    {
      
      UART_PRINT("error at putting the socket for listening to the incoming TCP connection ! \n\r"); 
    	return -1;
    }

	UART_PRINT("listen end! \n\r"); 

    // setting socket option to make the socket as non blocking
    iStatus = sl_SetSockOpt(iSockID, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &lNonBlocking, sizeof(lNonBlocking));
    iNewSockID = SL_EAGAIN;

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
//	  UART_PRINT(" iNewSockID == SL_EAGAIN! \n\r"); 
       }
       else if( iNewSockID < 0 )
       {
    	  // error
    	   UART_PRINT(" iNewSockID < 0! \n\r"); 
    	   return -1;
       }
    }


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





  
  
Sl_WlanNetworkEntry_t netEntries[20];
 char message[80];




/****no scan*********/
 /* 
#define SL_SCAN_DISABLE  0
sl_WlanPolicySet(SL_POLICY_SCAN,SL_SCAN_DISABLE,0,0);

while(1);
return 0;

*/





    
 UINT8  intervalInSeconds=1;  
    

 while(1){
   Z_DelayS(1); 
       sl_WlanPolicySet(SL_POLICY_SCAN,SL_SCAN_POLICY_EN(1), (unsigned char *)&intervalInSeconds,sizeof(intervalInSeconds));
 }
    
    
     
 
 
    

 while(1){  
    
   

    //Get Scan Result
   UINT8 Index = sl_WlanGetNetworkList(0,20,&netEntries[0]);

  
    for(UINT8 i=0; i< Index; i++)
    {
         snprintf(message, 60, "%d) SSID %s  RSSI %d \n\r",i,netEntries[i].ssid,netEntries[i].rssi);
	UART_PRINT(message); 
        
        for(UINT8 i=0; i< 1000; i++)
        sl_Send(iNewSockID, message, strlen(message), 0 );
        
        
        
        
    }  
    
  Z_DelayS(10);  

  }    
    









    // close the connected socket after receiving from connected TCP client
    sl_Close(iNewSockID);

    // close the listening socket
    sl_Close(iSockID);

    return 0;
}


