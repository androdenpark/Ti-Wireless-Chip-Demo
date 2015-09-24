
#include "uart_logger.h"
#include <stdlib.h>
#include <string.h>
#include "z_delay.h"
#include "datatypes.h"



#define UART_PRINT              Report




void Z_DelayUS(unsigned int times){
  unsigned int a,count;
  
  for(count = 0; count < times; ++count)
  
      for(a = 0; a<11; ++a)
      ; 
}


void Z_DelayMS(unsigned int times){
  
  for(unsigned int count = 0; count < times; ++count)
      Z_DelayUS(1000);
}


void Z_DelayS(unsigned int times){
  
  
  for(unsigned int count = 0; count < times; ++count)
      Z_DelayMS(1000);
}



void Z_MACDispaly(unsigned char *addr){


  char *string="00-";
  char *addr_1 = string;
 
  if(addr == NULL)return;

	for(char times = 0; times < 6; times++){
//	  MAC_DISPLAY_TRANSFORM(addr + times,addr_1);

	addr_1 = string;
          
	if((*addr_1 = *(addr + times)/16) >= 10)
        *addr_1 = *addr_1-10 +'a';
        
	else 
          *addr_1 = *addr_1+'0';
        
        addr_1++;
   	
	if((*addr_1 = *(addr + times)%16) >= 10) 
          *addr_1 = *addr_1-10 +'a';       
	else 
          *addr_1 = *addr_1+'0';
        
        addr_1++;

	if(times==5)
          *addr_1 ='\0';
          
	  UART_PRINT(string); 
	}
	
  UART_PRINT("\n\r");
	
}



void Z_IPDispaly(unsigned long *addr){


  unsigned char source_data,high_flag = 0;
  char string[5]={0};
  char *addr_1 = string;
 
  if(addr == NULL)return;

	for(char times = 0; times < 4; times++){
//	  IP_DISPLAY_TRANSFORM((*addr >>(3-times)*8)&&0XFF,addr_1);
	addr_1 = string;
	high_flag = 0;
          
        source_data =((*addr) >>(3-times)*8)&0xff;
          
        if((*addr_1 = source_data/100 +'0') > '0'){
        addr_1++;
        high_flag = 1;
        }
          	
	if(((*addr_1 = (source_data%100)/10 +'0') > '0') ||high_flag)
        addr_1++;  
        
	*addr_1 = source_data%10 + '0';
        addr_1++;
		
	if(times<3){
	*addr_1 = '.';
        addr_1++;
        }
        
        *addr_1 = '\0'; 
          
	UART_PRINT(string); 
	}
	
  UART_PRINT("\n\r");
	
}



void Z_NumDispaly(unsigned long addr,unsigned long num_len){
  unsigned char high_flag = 0;
  char string[10]={0};
//  unsigned long data =  addr;
  unsigned long i = 1;
  unsigned long a = 0;
  
  
//UART_PRINT("the  value  :     "); 



for(UINT8 b=0; b< num_len; b++)
		i *= 10;


for( ; i >=10; i  /= 10 ){
	if((string[a] = (addr%i/(i /10)+'0') ) > '0')
	high_flag = 1;

	a  += high_flag;			
}

UART_PRINT(string); 
UART_PRINT("    \n\r");

}








UINT32 sl_BIGtoLITTLE_l( UINT32 val )
{
  UINT32 i =  val ; 
  char *p = (char *)&i;  
  UINT8 temp_val;

temp_val = p[0];
p[0]  = p[3]  ;
p[3] =  temp_val ;

temp_val = p[1];
p[1] = p[2]   ;
p[2] =   temp_val  ;


return i;

}


UINT32 sl_BIGtoLITTLE_S( UINT16 val )
{
  UINT16 i =  val ; 
  char *p = (char *)&i;  

UINT8 temp_val = p[0];

p[0]  = p[1] ;
p[1] = temp_val ;
return i;
}


