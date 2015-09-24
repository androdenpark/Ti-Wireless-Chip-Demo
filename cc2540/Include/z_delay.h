#ifndef __Z_DELAY_H__
#define __Z_DELAY__	


#include "hal_types.h"

//#include "datatypes.h"

#define PRINT_STRING  1
#define PRINT_VALUE  2  

extern void Z_DelayUS(unsigned int times);



extern void Z_DelayMS(unsigned int times);



extern void LCDPrintText(char *string,uint16 arg1,uint8 flag);
extern void LCDPrintTitle(char *string,uint16 arg1,uint8 flag);
 

extern void Z_DelayS(unsigned int times);

extern void UartInitForTest(void);

/*

extern void Z_MACDispaly(unsigned char *addr);
extern void Z_IPDispaly(unsigned long *addr);

extern void Z_NumDispaly(unsigned long addr,unsigned long num_len);


extern UINT32 sl_BIGtoLITTLE_l( UINT32 val );
extern UINT32 sl_BIGtoLITTLE_S( UINT16 val );


*/

#endif
