#ifndef MQTTcommon_h
#define MQTTcommon_h


#include "libemqtt.h" 

#define BUFSIZE              256U

#define TOPIC_LEN_MAX           20U
#define MSG_LEN_MAX             200U



#define SUB_TASKID              0
#define PUB_TASKID              1

#define SUB_TASK_PRIORITY       1
#define PUB_TASK_PRIORITY       1


typedef struct{
uint8_t buffer[BUFSIZE];
uint16_t len;   
}Tranmit_t;


extern  unsigned long subTaskHandle ;
extern  unsigned long pubTaskHandle ;


extern int init_socket(mqtt_broker_handle_t* broker);
extern int close_socket(mqtt_broker_handle_t* broker);
extern int read_packet(int timeout,int socket_id, Tranmit_t *packet_buffer);


extern void taskPub( void *pvParameters );
extern void taskSub( void *pvParameters );

//extern unsigned long getTaskHandle(char TaskID);

#endif