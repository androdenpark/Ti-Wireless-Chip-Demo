
#ifndef AUTO_SWITCH_CENTRAL_H
#define AUTO_SWITCH_CENTRAL_H

#include "exampleService.h"

#include "hal_types.h"



// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15



typedef struct{
uint8 len;
uint8 handle;
uint16 uuid;
uint8 *value;
}CharacterInfo_t;


        
extern void CentralClientInitiat(void);
extern void simpleBLECentralProcessGATTMsg( gattMsgEvent_t *pMsg );

extern bool MacBufferSearch(uint8 *value);

extern void SwitchCentral_Init( void );














#endif      /*end of file*/