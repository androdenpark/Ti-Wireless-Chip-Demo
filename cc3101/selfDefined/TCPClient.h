#ifndef TCPCLIENT_H
#def TCPCLIENT_H

#define MAX_ID_LEN      16
#define MAX_CLIENT_NUM      5


typedef struct{
  
char clientID[MAX_ID_LEN];
uint32 brokerIP;
uint16 port;
int socketID;

}ClientInfo_t;


#endif