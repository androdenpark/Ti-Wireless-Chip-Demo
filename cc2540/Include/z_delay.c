
//#include "uart_logger.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "z_delay.h"
#include "hal_uart.h"
#include "OSAL.h"

#include "hal_lcd.h"

#define UART_PRINT              Report

#define LCD_START_LINE   HAL_LCD_LINE_3
#define LCD_END_LINE   HAL_LCD_LINE_8

//#define LCD_START_LINE_TITLE   HAL_LCD_LINE_1



static uint8 current_line=LCD_START_LINE; 
//static uint8 current_line_title=LCD_START_LINE_TITLE;


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





void LCDPrintText(char *string,uint16 arg1,uint8 flag){
#if (UART_OUT == TRUE)
  {
char buff=arg1+'0';

HalUARTWrite(HAL_UART_PORT_0, string, strlen(string));
if(buff<'9')
  HalUARTWrite(HAL_UART_PORT_0, &buff, sizeof(char));
HalUARTWrite(HAL_UART_PORT_0, "\n\r", 2);

  }
#else
  {
   if(current_line == LCD_START_LINE )
           for(uint8 i = LCD_START_LINE; i <= LCD_END_LINE; ++i) 
            HalLcdWriteString ("  ", i);
     
   if(flag==PRINT_STRING)
          HalLcdWriteString((char *)string, current_line);
   else 
          HalLcdWriteStringValue((char *)string,arg1,10,current_line);
   
   //assert(!arg1 );   
   
   current_line++;
   
  //Z_DelayMS(500);
   
  if(current_line > LCD_END_LINE)
          current_line = LCD_START_LINE;
   
  }
#endif
 
}



/*****************************Uart functions******************************************/

static void UartEventCallback(uint8 port, uint8 event);

#define 	SBP_UART_PORT		HAL_UART_PORT_0
#define	SBP_UART_FC		FALSE
#define	SBP_UART_FC_THRESHOLD	48
#define	SBP_UART_RX_BUF_SIZE		25
#define	SBP_UART_TX_BUF_SIZE		25
#define	SBP_UART_IDLE_TIMEOUT	6
#define	SBP_UART_INT_ENABLE		TRUE
#define	SBP_UART_BR				HAL_UART_BR_115200


void UartInitForTest(void){

halUARTCfg_t uartConfigPara;

uartConfigPara.configured		= 	TRUE;
uartConfigPara.baudRate		=	SBP_UART_BR;
uartConfigPara.flowControl		=	SBP_UART_FC;
uartConfigPara.flowControlThreshold = SBP_UART_FC_THRESHOLD;
uartConfigPara.rx.maxBufSize	= SBP_UART_RX_BUF_SIZE;
uartConfigPara.tx.maxBufSize	= SBP_UART_TX_BUF_SIZE;
uartConfigPara.idleTimeout		= SBP_UART_IDLE_TIMEOUT;
uartConfigPara.intEnable		= SBP_UART_INT_ENABLE;
uartConfigPara.callBackFunc		=UartEventCallback;

//HalUART_HW_Init(HAL_UART_PORT_0);
HalUARTInit();
HalUARTOpen(SBP_UART_PORT, &uartConfigPara);
}




static void UartEventCallback(uint8 port, uint8 event){

uint8 RxBuffer[ SBP_UART_RX_BUF_SIZE]={0};
uint8 BytesReserved=0;


                        //BytesReserved = Hal_UART_RxBufLen(port);
                        /*
                        switch(event):{
                                                case HAL_UART_RX_FULL:				
                                                        HalUARTRead(port, RxBuffer, BytesReserved);
                                                        RxBuffer[ BytesReserved-1]=0;
                                                        //LCDPrintText(RxBuffer,0,PRINT_STRING);
                                                        
                                                        break;
                                                        
                                                //case HAL_UART_TX_FULL:
                                                        //BytesReserved = Hal_UART_RxBufLen(port);
                                                        //HalUARTWrite(port, RxBuffer, BytesReserved);
                                                        //break;
                                                        
                                                        
                        
                        }
                        */

if(BytesReserved = Hal_UART_RxBufLen(port)){
		HalUARTRead(port, RxBuffer, BytesReserved);

 HalLcdWriteStringValue("haha:",RxBuffer[0],10,HAL_LCD_LINE_3);
}

              //RxBuffer[ SBP_UART_RX_BUF_SIZE-1]=0;
              //LCDPrintText(RxBuffer,0,PRINT_STRING);
              //HalLcdWriteStringValue("haha:",testnum,10,HAL_LCD_LINE_3);
              
              
              
              //if(BytesReserved = Hal_UART_TxBufLen(port))
                              //HalUARTWrite(port, RxBuffer, BytesReserved);
              
              //HalUARTWrite(port, RxBuffer, BytesReserved);

}


























/*
void LCDPrintTitle(char *string,uint16 arg1,uint8 flag){
  
 if(current_line_title == LCD_START_LINE_TITLE )
   for(uint8 i = LCD_START_LINE_TITLE; i <= HAL_LCD_LINE_3; ++i) 
	  HalLcdWriteString ("  ", i);
  
  
 if(flag==PRINT_STRING)
  	HalLcdWriteString ((char *)string, current_line_title);
 else 
  	HalLcdWriteStringValue((char *)string,arg1,10,current_line_title);
 
 current_line_title++;
 
 Z_DelayS(6);
 
if(current_line_title>HAL_LCD_LINE_3)
  current_line_title=LCD_START_LINE_TITLE;

 
}
*/



