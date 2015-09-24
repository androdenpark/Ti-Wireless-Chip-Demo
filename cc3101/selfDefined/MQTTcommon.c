#include "libemqtt.h"   
#include "simplelink.h"   
#include "z_delay.h"
#include "uart_logger.h"
#include "MQTTcommon.h"

#define UART_PRINT              Report

   
//#define BLOCKING 0
//uint8_t packet_buffer[RCVBUFSIZE];

//int socket_id;

#define SERVER_ADDR             SL_IPV4_VAL(9,186,88,23)                      //server,broker ip addr 
#define SERVER_PORT             1883U                                              //server,broker port 


static int socket_id;

static int send_packet(void* socket_info, const void* buf, unsigned int count)
{
	int fd = *((int*)socket_info);
	return sl_Send(fd, buf, count, 0);
}

int init_socket(mqtt_broker_handle_t* broker)
{
  
        SlSockAddrIn_t  sAddr;
        int             iAddrSize;
        int             iSockID;
        int             iStatus;
        long            lNonBlocking = 0;                   //0 :non-blocking,
        
             //filling the TCP server socket address
    sAddr.sin_family = SL_AF_INET;
    //sAddr.sin_port = sl_Htons((unsigned short)SERVER_PORT);
    //sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)SERVER_ADDR);
        sAddr.sin_port = sl_Htons((unsigned short)1883);
    sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)SL_IPV4_VAL(9,186,88,23));
    
    iAddrSize = sizeof(SlSockAddrIn_t); 
    
    
	//int flag = 1;

        //long  lNonBlocking = BLOCKING;
	int keepalive = 5; // Seconds

	// Create the socket
        
        
	if((socket_id = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0)) < 0){
                UART_PRINT("error at sl_Socket()!\n");
		return -1;
        }
        
        
        // Disable Nagle Algorithm
        //if (setsockopt(socket_id, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) < 0)
     //   #define BLOCKING 0

        if (sl_SetSockOpt(socket_id, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &lNonBlocking, sizeof(lNonBlocking)) < 0){  
          UART_PRINT("error at sl_SetSockOpt()!\n");
		return -2;
        }  


	// Connect the socket
        while(0>(socket_id = sl_Connect(socket_id, ( SlSockAddr_t *)&sAddr, iAddrSize))){
          UART_PRINT("returned else !\n");
          Z_DelayMS(100);
        }
        
        lNonBlocking=1;
        if (sl_SetSockOpt(socket_id, SL_SOL_SOCKET, SL_SO_NONBLOCKING, &lNonBlocking, sizeof(lNonBlocking)) < 0){  
          UART_PRINT("error at sl_SetSockOpt()!\n");
		return -2;
        }  
        
        
        

	// MQTT stuffs
	mqtt_set_alive(broker, keepalive);
	*((int *)(broker->socket_info)) = socket_id;
	broker->send = send_packet;

	return socket_id;
}

int close_socket(mqtt_broker_handle_t* broker)
{
	int fd = *((int*)broker->socket_info);
	return close(fd);
}




int read_packet(int timeout,int socket_id, Tranmit_t *packet_buffer)
{
	if(timeout > 0)
	{
		fd_set readfds;
                
		struct timeval tmv;

		// Initialize the file descriptor set
		FD_ZERO (&readfds);
		FD_SET (socket_id, &readfds);
                int max_fd = socket_id>0? socket_id+1: 1;
		// Initialize the timeout data structure
		tmv.tv_sec = timeout;
		tmv.tv_usec = 0;

		// select returns 0 if timeout, 1 if input available, -1 if error
		while(select(max_fd, &readfds, NULL, NULL, &tmv)<1){
                  UART_PRINT("no available socket fd to read\n\r");
                  //return -2;
                }
	}

	int total_bytes = 0, bytes_rcvd, packet_length;
	memset(packet_buffer->buffer, 0, packet_buffer->len);

	while(total_bytes < 2) // Reading fixed header
	{
          if((bytes_rcvd = sl_Recv(socket_id, (packet_buffer->buffer+total_bytes), packet_buffer->len-total_bytes, 0)) <= 0){
            UART_PRINT("read_packet:sl_Recv(1) error\n\r");
			return -1;
          }
		total_bytes += bytes_rcvd; // Keep tally of total bytes
	}

	packet_length = packet_buffer->buffer[1] + 2; // Remaining length + fixed header length

	while(total_bytes < packet_length) // Reading the packet
	{
          if((bytes_rcvd = sl_Recv(socket_id, (packet_buffer->buffer+total_bytes), packet_buffer->len-total_bytes, 0)) <= 0){
            UART_PRINT("read_packet:sl_Recv(2) error\n\r");
			return -1;
          }
		total_bytes += bytes_rcvd; // Keep tally of total bytes
	}

	return packet_length;
}




