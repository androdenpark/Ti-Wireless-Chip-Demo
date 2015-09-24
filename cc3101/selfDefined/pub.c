/*
 * This file is part of libemqtt.
 *
 * libemqtt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libemqtt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with libemqtt.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *
 * Created by Vicente Ruiz Rodr¨ªguez
 * Copyright 2012 Vicente Ruiz Rodr¨ªguez <vruiz2.0@gmail.com>. All rights reserved.
 *
 */

#include <libemqtt.h>
#include <string.h>   
#include "simplelink.h"   
#include "z_delay.h"
#include "uart_logger.h"
#include "MQTTcommon.h"
   
#define UART_PRINT              Report
   
static Tranmit_t packet_buffer;

//static int socket_id;


 int mqtt_pub(void)
{
	int packet_length,socket_id;
	uint16_t msg_id, msg_id_rcv;
	mqtt_broker_handle_t broker;
        
        packet_buffer.len=BUFSIZE;
        broker.socket_info = (void *)&socket_id;

	mqtt_init(&broker, "avengalvon");
	mqtt_init_auth(&broker, "cid", "campeador");
        
	if(0>init_socket(&broker)){
              UART_PRINT("error at creating socket!\n\r");
              return -1;
        }
        else
              UART_PRINT("creat socket susseed!\n\r");
        
        //while(1);
       
	// >>>>> CONNECT
	if(0>mqtt_connect(&broker))
              UART_PRINT("mqtt_connect error!\n\r");
        
        socket_id=*(int *)broker.socket_info;
	// <<<<< CONNACK
	packet_length = read_packet(1,socket_id,(Tranmit_t *)&packet_buffer);
	if(packet_length < 2)
	{
		UART_PRINT("Error(%d) on read packet!\n\r");
		return -1;
	}

	if(MQTTParseMessageType(packet_buffer.buffer) != MQTT_MSG_CONNACK)
	{
		UART_PRINT("CONNACK expected!\n\r");
		return -2;
	}

	if(packet_buffer.buffer[3] != 0x00)
	{
		UART_PRINT("CONNACK failed!\n\r");
		return -2;
	}
        
        //while(1);
        UART_PRINT("Connected to broker!\n\r");

	// >>>>> PUBLISH QoS 0
	UART_PRINT("Publish: QoS 0\n\r");
	if(0>mqtt_publish(&broker, "helloword", "Example: QoS 0", 0))
                UART_PRINT("Publish QoS 0 error!\n\r");

	// >>>>> PUBLISH QoS 1
	UART_PRINT("Publish: QoS 1\n\r");
	if(0>mqtt_publish_with_qos(&broker, "helloword", "Example: QoS 1", 0, 1, &msg_id))
                UART_PRINT("Publish QoS 1 error!\n\r");

	// <<<<< PUBACK
	packet_length = read_packet(1,socket_id,(Tranmit_t *)&packet_buffer);
	if(packet_length < 0)
	{
		UART_PRINT("Error(%d) on read packet!\n\r");
		return -1;
	}

	if(MQTTParseMessageType(packet_buffer.buffer) != MQTT_MSG_PUBACK)
	{
		UART_PRINT("PUBACK expected!\n\r");
		return -2;
	}

	msg_id_rcv = mqtt_parse_msg_id(packet_buffer.buffer);
	if(msg_id != msg_id_rcv)
	{
		UART_PRINT("%d message id was expected, but %d message id was found!\n\r");
		return -3;
	}

	// >>>>> PUBLISH QoS 2
	UART_PRINT("Publish: QoS 2\n");
	mqtt_publish_with_qos(&broker, "helloword", "Example: QoS 2", 1, 2, &msg_id); // Retain
	// <<<<< PUBREC
	packet_length = read_packet(1,socket_id,(Tranmit_t *)&packet_buffer);
	if(packet_length < 2)
	{
		UART_PRINT("Error(%d) on read packet!\n\r");
		return -1;
	}

	if(MQTTParseMessageType(packet_buffer.buffer) != MQTT_MSG_PUBREC)
	{
		UART_PRINT("PUBREC expected!\n\r");
		return -2;
	}

	msg_id_rcv = mqtt_parse_msg_id(packet_buffer.buffer);
	if(msg_id != msg_id_rcv)
	{
		UART_PRINT("%d message id was expected, but %d message id was found!\n\r");
		return -3;
	}

	// >>>>> PUBREL
	mqtt_pubrel(&broker, msg_id);
	// <<<<< PUBCOMP
	packet_length = read_packet(1,socket_id,(Tranmit_t *)&packet_buffer);
	if(packet_length < 0)
	{
		UART_PRINT("Error(%d) on read packet!\n\r");
		return -1;
	}

	if(MQTTParseMessageType(packet_buffer.buffer) != MQTT_MSG_PUBCOMP)
	{
		UART_PRINT("PUBCOMP expected!\n\r");
		return -2;
	}

	msg_id_rcv = mqtt_parse_msg_id(packet_buffer.buffer);
	if(msg_id != msg_id_rcv)
	{
		UART_PRINT("%d message id was expected, but %d message id was found!\n\r");
		return -3;
	}

	// >>>>> DISCONNECT
	mqtt_disconnect(&broker);
	close_socket(&broker);
	return 0;
}





void taskPub( void *pvParameters ){
mqtt_pub();
UART_PRINT("taskPub over\r\n"); 
vTaskDelete(NULL);
}









/*
uint8_t packet_buffer[RCVBUFSIZE];

int socket_id;

#define SERVER_ADDR             SL_IPV4_VAL(9,186,88,87)                      //server,broker ip addr 

/
int send_packet(void* socket_info, const void* buf, unsigned int count)
{
	int fd = *((int*)socket_info);
	return sl_Send(fd, buf, count, 0);
}

int init_socket(mqtt_broker_handle_t* broker, const char* hostname, short port)
{
  
        SlSockAddrIn_t  sAddr;
        int             iAddrSize;
        int             iSockID;
        int             iStatus;
        long            lNonBlocking = 0;                   //0 :non-blocking,
        
             //filling the TCP server socket address
    sAddr.sin_family = SL_AF_INET;
    sAddr.sin_port = sl_Htons((unsigned short)1883);
    sAddr.sin_addr.s_addr = sl_Htonl((unsigned int)SL_IPV4_VAL(9,186,88,23));
    iAddrSize = sizeof(SlSockAddrIn_t); 
    
    
	//int flag = 1;

        //long  lNonBlocking = BLOCKING;
	int keepalive = 10; // Seconds

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
	broker->socket_info = (void*)&socket_id;
	broker->send = send_packet;

	return 0;
}

int close_socket(mqtt_broker_handle_t* broker)
{
	int fd = *((int*)broker->socket_info);
	return close(fd);
}




int read_packet(int timeout)
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
	memset(packet_buffer, 0, sizeof(packet_buffer));

	while(total_bytes < 2) // Reading fixed header
	{
          if((bytes_rcvd = sl_Recv(socket_id, (packet_buffer+total_bytes), RCVBUFSIZE-total_bytes, 0)) <= 0){
            UART_PRINT("read_packet:sl_Recv(1) error\n\r");
			return -1;
          }
		total_bytes += bytes_rcvd; // Keep tally of total bytes
	}

	packet_length = packet_buffer[1] + 2; // Remaining length + fixed header length

	while(total_bytes < packet_length) // Reading the packet
	{
          if((bytes_rcvd = sl_Recv(socket_id, (packet_buffer+total_bytes), RCVBUFSIZE-total_bytes, 0)) <= 0){
            UART_PRINT("read_packet:sl_Recv(2) error\n\r");
			return -1;
          }
		total_bytes += bytes_rcvd; // Keep tally of total bytes
	}

	return packet_length;
}




*/