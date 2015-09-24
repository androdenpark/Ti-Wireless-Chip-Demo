#ifndef AUTO_SWITCH_PERIPHERAL_H
#define AUTO_SWITCH_PERIPHERAL_H


extern void PeripheralServerInitiat(void);
extern void PeripheralProcessGATTMsg( gattMsgEvent_t *pMsg );
extern void SwitchPeripheral_Init(void);


extern uint8 BroardcastTimeRecord;

#endif