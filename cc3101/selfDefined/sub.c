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

/* sub.c */

/*
 * This file was forked from libemqtt,
 * developed by Vicente Ruiz Rodr¨ªguez.
 * https://github.com/menudoproblema/libemqtt
 *
 */

#include "libemqtt.h"   
#include "simplelink.h"   
#include "z_delay.h"
#include "uart_logger.h"
#include "MQTTcommon.h"
#include "task.h"
#include "osi.h"
   
#define UART_PRINT              Report
//#define RCVBUFSIZE 256
   
//#define BLOCKING 0
static Tranmit_t packet_buffer;

//static int socket_id;


/**
 * Main routine
 *
 */
static int mqtt_sub(void)
{
	int packet_length,socket_id;
	uint16_t msg_id, msg_id_rcv;
        mqtt_broker_handle_t broker;
        
        packet_buffer.len=BUFSIZE;
        broker.socket_info = (void *)&socket_id;
        

	mqtt_init(&broker, "MQTT_SUB");
	//mqtt_init_auth(&broker, "quijote", "rocinante");
	socket_id = init_socket(&broker);

	// >>>>> CONNECT
	mqtt_connect(&broker);
	// <<<<< CONNACK
        
       // unsigned long pubTaskHandle= getTaskHandle(SUB_TASKID);
       // vTaskPrioritySet( (xTaskHandle)&pubTaskHandle, PUB_TASK_PRIORITY);                   //to degrade sub_task priority to let it as the same of pub_task

      
        
	packet_length = read_packet(1,socket_id,(Tranmit_t *)&packet_buffer);
	if(packet_length < 0)
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
        
        
        
        UART_PRINT("Connected to broker!\n\r");
        
        
        
                if(OSI_OK != osi_TaskCreate( taskPub,
    				(const signed char*)"taskPub",
    				2048, NULL, PUB_TASK_PRIORITY, (OsiTaskHandle)&pubTaskHandle ))
      UART_PRINT("taskPub failed\n\r");

	// Signals after connect MQTT
	//signal(SIGALRM, alive);
	//alarm(keepalive);
	//signal(SIGINT, term);

	// >>>>> SUBSCRIBE
	mqtt_subscribe(&broker, "helloword", &msg_id);
	// <<<<< SUBACK
	packet_length = read_packet(1,socket_id,(Tranmit_t *)&packet_buffer);
	if(packet_length < 0)
	{
		UART_PRINT("Error(%d) on read packet!\n\r");
		return -1;
	}

	if(MQTTParseMessageType(packet_buffer.buffer) != MQTT_MSG_SUBACK)
	{
		UART_PRINT("SUBACK expected!\n\r");
		return -2;
	}

	msg_id_rcv = mqtt_parse_msg_id(packet_buffer.buffer);
	if(msg_id != msg_id_rcv)
	{
		UART_PRINT("%d message id was expected, but %d message id was found!\n\r");
		return -3;
	}

	while(1)
	{
		// <<<<<
		packet_length = read_packet(0,socket_id,(Tranmit_t *)&packet_buffer);
		if(packet_length == -1)
		{
			UART_PRINT("Error(%d) on read packet!\n\r");
			return -1;
		}
		else if(packet_length > 0)
		{
			UART_PRINT("Packet Header: 0x%x...\n\r");
			if(MQTTParseMessageType(packet_buffer.buffer) == MQTT_MSG_PUBLISH)
			{
				uint8_t topic[TOPIC_LEN_MAX], msg[MSG_LEN_MAX];
				uint16_t len;
				len = mqtt_parse_pub_topic(packet_buffer.buffer, topic);
				topic[len] = '\0'; // for printf
				len = mqtt_parse_publish_msg(packet_buffer.buffer, msg);
				msg[len] = '\0'; // for printf
				//UART_PRINT("%s %s\n\r", topic, msg);
                                UART_PRINT(topic);
                                UART_PRINT("\n\r");
                                UART_PRINT(msg);
                                UART_PRINT("\n\r");
			}
		}

	}
	return 0;
}





void taskSub( void *pvParameters ){
mqtt_sub();
UART_PRINT("taskSub over\r\n"); 
vTaskDelete(NULL);
}



