#include "usart_print.h"
//private funtions list
void ms_delay(int ms){
	while (ms-- > 0) {
      volatile int x=5971;
      while (x-- > 0)
         __asm("nop");
   }
}

void usart_printf2(char *buffer, int char_no){
while (char_no != 0){
while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
USART_SendData(USART1,*buffer);
buffer++;
char_no--;
}
}



void USART_putc(char c){
while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
USART_SendData(USART1,c);
}

void USART_puts(const char *s){
 int i;
 for(i=0;s[i]!='\0';i++) USART_putc(s[i]);
}


//public funtions list
/*
void *_sbrk(ptrdiff_t increment)
{
extern char _end;
static char *heap_end = 0;
char *prev_heap_end;
char register *stack_ptr asm("sp");
if (heap_end == 0) {
heap_end = &_end;
}
prev_heap_end = heap_end;
if ((heap_end + increment) > stack_ptr) {
USART_puts("sp=");
//PUTP(stack_ptr);
USART_puts("\n");
USART_puts("heap=");
//PUTP(heap_end);
USART_puts("\n");
USART_puts("sbrk: no more heap\n");
//errno = ENOMEM;

return (void*)-1;
}
heap_end += increment;
return prev_heap_end;
}
*/

void usart_initialization(){
	USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
  

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
  //GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
  
  /* Configure USART1 pins:  Rx and Tx ----------------------------*/
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  

  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);
  
  USART_Cmd(USART1,ENABLE);
}

void usart_printf(const char *format, ...) {
 va_list list;
 va_start(list, format);
 int len = vsnprintf(0, 0, format, list);
 char *s; 
s = (char *)malloc(len + 1);
 vsprintf(s, format, list);
USART_puts(s);
 free(s);
 va_end(list);
 return;
}