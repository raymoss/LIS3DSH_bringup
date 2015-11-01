#include "stm32f4_discovery_lis3dsh.h"

__IO uint32_t  LIS3DSHTimeout = LIS3DSH_FLAG_TIMEOUT;   

/* Read/Write command */
#define READWRITE_CMD              ((uint8_t)0x80) 
/* Multiple byte read/write command */ 
#define MULTIPLEBYTE_CMD           ((uint8_t)0x40)
/* Dummy Byte Send by the SPI Master device in order to generate the Clock to the Slave device */
#define DUMMY_BYTE                 ((uint8_t)0x00)
static uint8_t LIS3DSH_SendByte(uint8_t byte);
static void LIS3DSH_LowLevel_Init(void);
float sensitivity = 0.0;
void LIS3DSH_Init(LIS3DSH_InitTypeDef *LIS3DSH_InitStruct)
{
  uint8_t ctrl = 0x00;
  
  /* Configure the low level interface ---------------------------------------*/
  LIS3DSH_LowLevel_Init();
  
  /* Configure MEMS: data rate, power mode, full scale, self test and axes */
  ctrl = (uint8_t) (LIS3DSH_InitStruct->Block_data_update | LIS3DSH_InitStruct->Axes_Enable | \
    LIS3DSH_InitStruct->Power_Mode);
  usart_printf("The value of ctrl reg4 = %x\r\n",ctrl);
  /* Write value to MEMS CTRL_REG1 regsister */
  LIS3DSH_Write(&ctrl, LIS3DSH_CTRL_REG4_ADDR, 1);
/*Initialising control reg 5*/
	ctrl = 0x00;
	ctrl=(uint8_t)(LIS3DSH_InitStruct->Anti_Alising_bandwidth | LIS3DSH_InitStruct->Full_scale_selection | \
			LIS3DSH_InitStruct->Self_test_enable | LIS3DSH_InitStruct->SPI_mode_selection);
usart_printf("The value of ctrl reg5 = %x\r\n",ctrl);
        LIS3DSH_Write(&ctrl , LIS3DSH_CTRL_REG5_ADDR , 1);
}

void LIS3DSH_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite)
{
  /* Configure the MS bit: 
       - When 0, the address will remain unchanged in multiple read/write commands.
       - When 1, the address will be auto incremented in multiple read/write commands.
  */
  if(NumByteToWrite > 0x01)
  {
    WriteAddr |= (uint8_t)MULTIPLEBYTE_CMD;
  }
  /* Set chip select Low at the start of the transmission */
  LIS3DSH_CS_LOW();
  
  /* Send the Address of the indexed register */
  LIS3DSH_SendByte(WriteAddr);
  /* Send the data that will be written into the device (MSB First) */
  while(NumByteToWrite >= 0x01)
  {
    LIS3DSH_SendByte(*pBuffer);
    NumByteToWrite--;
    pBuffer++;
  }
  
  /* Set chip select High at the end of the transmission */ 
  LIS3DSH_CS_HIGH();
}
void LIS3DSH_Read(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{  
  if(NumByteToRead > 0x01)
  {
    ReadAddr |= (uint8_t)(READWRITE_CMD);
  }
  else
  {
    ReadAddr |= (uint8_t)READWRITE_CMD;
  }
  /* Set chip select Low at the start of the transmission */
  LIS3DSH_CS_LOW();
  
  /* Send the Address of the indexed register */
  LIS3DSH_SendByte(ReadAddr);
  
  /* Receive the data that will be read from the device (MSB First) */
  while(NumByteToRead > 0x00)
  {
    /* Send dummy byte (0x00) to generate the SPI clock to LIS3DSH (Slave device) */
    *pBuffer = LIS3DSH_SendByte(DUMMY_BYTE);
    NumByteToRead--;
    pBuffer++;
  }
  
  /* Set chip select High at the end of the transmission */ 
  LIS3DSH_CS_HIGH();
}
void LIS3DSH_ReadACC(int32_t* out)
{
  uint8_t buffer[6];
  uint8_t crtl, i = 0x00;
   
  LIS3DSH_Read(&crtl, LIS3DSH_CTRL_REG5_ADDR, 1);  
  LIS3DSH_Read(buffer, LIS3DSH_OUT_XL_ADDR, 6);
  
  switch(crtl & LIS3DSH__FULLSCALE_SELECTION)
{
/* FS bit = 000 ==> Sensitivity typical value = 0.06milligals/digit*/
case LIS3DSH_FULLSCALE_2:
sensitivity = LIS3DSH_SENSITIVITY_0_06G;
break;
/* FS bit = 001 ==> Sensitivity typical value = 0.12milligals/digit*/
case LIS3DSH_FULLSCALE_4:
sensitivity = LIS3DSH_SENSITIVITY_0_12G;
break;
/* FS bit = 010 ==> Sensitivity typical value = 0.18milligals/digit*/
case LIS3DSH_FULLSCALE_6:
sensitivity = LIS3DSH_SENSITIVITY_0_18G;
break;
/* FS bit = 011 ==> Sensitivity typical value = 0.24milligals/digit*/
case LIS3DSH_FULLSCALE_8:
sensitivity = LIS3DSH_SENSITIVITY_0_24G;
break;
/* FS bit = 100 ==> Sensitivity typical value = 0.73milligals/digit*/
case LIS3DSH_FULLSCALE_16:
sensitivity = LIS3DSH_SENSITIVITY_0_73G;
break;
default:
break;
}
float valueinfloat=0.0;
for(i=0; i<3; i++)
{
valueinfloat = ((buffer[2*i+1] << 8) + buffer[2*i]) * sensitivity;
out[i] = (int16_t)valueinfloat;
}
}
static void LIS3DSH_LowLevel_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

  /* Enable the SPI periph */
  RCC_APB2PeriphClockCmd(LIS3DSH_SPI_CLK, ENABLE);

  /* Enable SCK, MOSI and MISO GPIO clocks */
  RCC_AHB1PeriphClockCmd(LIS3DSH_SPI_SCK_GPIO_CLK | LIS3DSH_SPI_MISO_GPIO_CLK | LIS3DSH_SPI_MOSI_GPIO_CLK, ENABLE);

  /* Enable CS  GPIO clock */
  RCC_AHB1PeriphClockCmd(LIS3DSH_SPI_CS_GPIO_CLK, ENABLE);
  
  /* Enable INT1 GPIO clock */
  RCC_AHB1PeriphClockCmd(LIS3DSH_SPI_INT1_GPIO_CLK, ENABLE);
  
  /* Enable INT2 GPIO clock */
  RCC_AHB1PeriphClockCmd(LIS3DSH_SPI_INT2_GPIO_CLK, ENABLE);

  GPIO_PinAFConfig(LIS3DSH_SPI_SCK_GPIO_PORT, LIS3DSH_SPI_SCK_SOURCE, LIS3DSH_SPI_SCK_AF);
  GPIO_PinAFConfig(LIS3DSH_SPI_MISO_GPIO_PORT, LIS3DSH_SPI_MISO_SOURCE, LIS3DSH_SPI_MISO_AF);
  GPIO_PinAFConfig(LIS3DSH_SPI_MOSI_GPIO_PORT, LIS3DSH_SPI_MOSI_SOURCE, LIS3DSH_SPI_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  /* SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = LIS3DSH_SPI_SCK_PIN;
  GPIO_Init(LIS3DSH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /* SPI  MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  LIS3DSH_SPI_MOSI_PIN;
  GPIO_Init(LIS3DSH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /* SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin = LIS3DSH_SPI_MISO_PIN;
  GPIO_Init(LIS3DSH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /* SPI configuration -------------------------------------------------------*/
  SPI_I2S_DeInit(LIS3DSH_SPI);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_Init(LIS3DSH_SPI, &SPI_InitStructure);

  /* Enable SPI1  */
  SPI_Cmd(LIS3DSH_SPI, ENABLE);

  /* Configure GPIO PIN for Lis Chip select */
  GPIO_InitStructure.GPIO_Pin = LIS3DSH_SPI_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(LIS3DSH_SPI_CS_GPIO_PORT, &GPIO_InitStructure);

  /* Deselect : Chip Select high */
  GPIO_SetBits(LIS3DSH_SPI_CS_GPIO_PORT, LIS3DSH_SPI_CS_PIN);
  
  /* Configure GPIO PINs to detect Interrupts */
  GPIO_InitStructure.GPIO_Pin = LIS3DSH_SPI_INT1_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(LIS3DSH_SPI_INT1_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = LIS3DSH_SPI_INT2_PIN;
  GPIO_Init(LIS3DSH_SPI_INT2_GPIO_PORT, &GPIO_InitStructure);
}
static uint8_t LIS3DSH_SendByte(uint8_t byte)
{
  /* Loop while DR register in not emplty */
  LIS3DSHTimeout = LIS3DSH_FLAG_TIMEOUT;
  while (SPI_I2S_GetFlagStatus(LIS3DSH_SPI, SPI_I2S_FLAG_TXE) == RESET)
  {
    if((LIS3DSHTimeout--) == 0) return LIS3DSH_TIMEOUT_UserCallback();
  }
  
  /* Send a Byte through the SPI peripheral */
  SPI_I2S_SendData(LIS3DSH_SPI, byte);
  
  /* Wait to receive a Byte */
  LIS3DSHTimeout = LIS3DSH_FLAG_TIMEOUT;
  while (SPI_I2S_GetFlagStatus(LIS3DSH_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
    if((LIS3DSHTimeout--) == 0) return LIS3DSH_TIMEOUT_UserCallback();
  }
  
  /* Return the Byte read from the SPI bus */
  return (uint8_t)SPI_I2S_ReceiveData(LIS3DSH_SPI);
}
