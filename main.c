#include "main.h"

#define TESTRESULT_ADDRESS         0x080FFFFC
#define ALLTEST_PASS               0x00000000
#define ALLTEST_FAIL               0x55555555

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment = 4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
//__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;
  
uint16_t PrescalerValue = 0;

__IO uint32_t TimingDelay;
__IO uint8_t DemoEnterCondition = 0x00;
__IO uint8_t UserButtonPressed = 0x00;
LIS3DSH_InitTypeDef  LIS3DSH_InitStruct;
void TIM4_Config(void);
__IO int8_t X_Offset, Y_Offset, Z_Offset  = 0x00;
uint8_t Buffer[6];
int main(void){
/*Initialising the structure for the accelerometer*/
/* TIM4 channels configuration */
  //setbuf(stdout, NULL);  
  TIM4_Config();
    
    /* Disable all Timer4 channels */
    TIM_CCxCmd(TIM4, TIM_Channel_1, DISABLE);
    TIM_CCxCmd(TIM4, TIM_Channel_2, DISABLE);
    TIM_CCxCmd(TIM4, TIM_Channel_3, DISABLE);
    TIM_CCxCmd(TIM4, TIM_Channel_4, DISABLE);





 /* MEMS configuration */
    LIS3DSH_InitStruct.Power_Mode = LIS3DSH_DATARATE_100;
    LIS3DSH_InitStruct.Block_data_update = LIS3DSH_BDU_NO_UPDATE;
    LIS3DSH_InitStruct.Axes_Enable = LIS3DSH_XYZ_ENABLE;
    LIS3DSH_InitStruct.Anti_Alising_bandwidth = LIS3DSH_FILTER_BW_800;
    LIS3DSH_InitStruct.Full_scale_selection = LIS3DSH_FULLSCALE_2;
    LIS3DSH_InitStruct.Self_test_enable = LIS3DSH_SELFTEST_NORMAL;
    LIS3DSH_InitStruct.SPI_mode_selection = LIS3DSH_SERIALINTERFACE_4WIRE;
    LIS3DSH_Init(&LIS3DSH_InitStruct);
    
    /* Required delay for the MEMS Accelerometre: Turn-on time = 3/Output data Rate 
    = 3/100 = 30ms */
    ms_delay(30);
    
    DemoEnterCondition = 0x01;
    /* MEMS High Pass Filter configuration 
    LIS3DSH_FilterStruct.HighPassFilter_Data_Selection = LIS3DSH_FILTEREDDATASELECTION_OUTPUTREGISTER;
    LIS3DSH_FilterStruct.HighPassFilter_CutOff_Frequency = LIS3DSH_HIGHPASSFILTER_LEVEL_1;
    LIS3DSH_FilterStruct.HighPassFilter_Interrupt = LIS3DSH_HIGHPASSFILTERINTERRUPT_1_2;
    LIS3DSH_FilterConfig(&LIS3DSH_FilterStruct);  */
    
    LIS3DSH_Read(Buffer, LIS3DSH_OUT_XL_ADDR, 6);
    X_Offset = Buffer[0];
    Y_Offset = Buffer[2];
    Z_Offset = Buffer[4];
// Disable STDOUT buffering. Otherwise nothing will be printed
   // before a newline character or when the buffer is flushed.
   // This MUST be done before any writes to STDOUT to have any effect...
   
   usart_initialization();
   usart_printf("Hello world\r\n");
   usart_printf("X_offset = %d\r\n",X_Offset);

   for(;;){
LIS3DSH_Read(Buffer, LIS3DSH_OUT_XL_ADDR, 6);
    X_Offset = Buffer[0];
    Y_Offset = Buffer[2];
    Z_Offset = Buffer[4];	
usart_printf("X_offset = %d\r\n",X_Offset);
	usart_printf("Y_offset = %d\r\n",Y_Offset);
	usart_printf("Z_offset = %d\r\n",Z_Offset);
ms_delay(1000);	
}
      
   
   return 0;

}
static void TIM4_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  
  /* --------------------------- System Clocks Configuration -----------------*/
  /* TIM4 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  
  /* GPIOD clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  /*-------------------------- GPIO Configuration ----------------------------*/
  /* GPIOD Configuration: Pins 12, 13, 14 and 15 in output push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* Connect TIM4 pins to AF2 */  
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4); 
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_TIM4);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_TIM4); 
  
    /* -----------------------------------------------------------------------
    TIM4 Configuration: Output Compare Timing Mode:
    
    In this example TIM4 input clock (TIM4CLK) is set to 2 * APB1 clock (PCLK1), 
    since APB1 prescaler is different from 1 (APB1 Prescaler = 4, see system_stm32f4xx.c file).
      TIM4CLK = 2 * PCLK1  
      PCLK1 = HCLK / 4 
      => TIM4CLK = 2*(HCLK / 4) = HCLK/2 = SystemCoreClock/2
         
    To get TIM4 counter clock at 2 KHz, the prescaler is computed as follows:
       Prescaler = (TIM4CLK / TIM1 counter clock) - 1
       Prescaler = (168 MHz/(2 * 2 KHz)) - 1 = 41999
                                        
    To get TIM4 output clock at 1 Hz, the period (ARR)) is computed as follows:
       ARR = (TIM4 counter clock / TIM4 output clock) - 1
           = 1999
                    
    TIM4 Channel1 duty cycle = (TIM4_CCR1/ TIM4_ARR)* 100 = 50%
    TIM4 Channel2 duty cycle = (TIM4_CCR2/ TIM4_ARR)* 100 = 50%
    TIM4 Channel3 duty cycle = (TIM4_CCR3/ TIM4_ARR)* 100 = 50%
    TIM4 Channel4 duty cycle = (TIM4_CCR4/ TIM4_ARR)* 100 = 50%
    
    ==> TIM4_CCRx = TIM4_ARR/2 = 1000  (where x = 1, 2, 3 and 4).
  
    Note: 
     SystemCoreClock variable holds HCLK frequency and is defined in system_stm32f4xx.c file.
     Each time the core clock (HCLK) changes, user had to call SystemCoreClockUpdate()
     function to update SystemCoreClock variable value. Otherwise, any configuration
     based on this variable will be incorrect.    
  ----------------------------------------------------------------------- */ 
  
  
  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 2000) - 1;
  
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = TIM_ARR;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  
  /* Enable TIM4 Preload register on ARR */
  TIM_ARRPreloadConfig(TIM4, ENABLE);
  
  /* TIM PWM1 Mode configuration: Channel */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = TIM_CCR;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  
  /* Output Compare PWM1 Mode configuration: Channel1 */
  TIM_OC1Init(TIM4, &TIM_OCInitStructure);
  TIM_CCxCmd(TIM4, TIM_Channel_1, DISABLE);
  
  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
  
  /* Output Compare PWM1 Mode configuration: Channel2 */
  TIM_OC2Init(TIM4, &TIM_OCInitStructure);
  TIM_CCxCmd(TIM4, TIM_Channel_2, DISABLE);
  
  TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
    
  /* Output Compare PWM1 Mode configuration: Channel3 */
  TIM_OC3Init(TIM4, &TIM_OCInitStructure);
  TIM_CCxCmd(TIM4, TIM_Channel_3, DISABLE);
  
  TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
  
  /* Output Compare PWM1 Mode configuration: Channel4 */
  TIM_OC4Init(TIM4, &TIM_OCInitStructure);
  TIM_CCxCmd(TIM4, TIM_Channel_4, DISABLE);
  
  TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
  
  /* TIM4 enable counter */
  TIM_Cmd(TIM4, ENABLE);
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in 10 ms.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

/**
  * @brief  This function handles the test program fail.
  * @param  None
  * @retval None
  */
void Fail_Handler(void)
{
  /* Erase last sector */ 
  FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
  /* Write FAIL code at last word in the flash memory */
  FLASH_ProgramWord(TESTRESULT_ADDRESS, ALLTEST_FAIL);
  
  while(1)
  {
    /* Toggle Red LED */
    STM_EVAL_LEDToggle(LED5);
    Delay(5);
  }
}

/**
  * @brief  MEMS accelerometre management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t LIS3DSH_TIMEOUT_UserCallback(void)
{
  /* MEMS Accelerometer Timeout error occured during Test program execution */
  if (DemoEnterCondition == 0x00)
  {
    /* Timeout error occured for SPI TXE/RXNE flags waiting loops.*/
    Fail_Handler();    
  }
  /* MEMS Accelerometer Timeout error occured during Demo execution */
  else
  {
    while (1)
    {   
    }
  }
  return 0;  
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/******************* (C) COPYRIGHT 2011 STMicroelectronics ***/
