# LIS3DSH_bringup
This project tracks the progress for writing the driver for LIS3DSH accelerometer sensor for STM32F4discovery board.

Objectives:

1. Write SPI interface code for interacting with the driver. DONE
2. Create stm32f4_discovery_lis3dsh.h file for defining the register address similar to that of stm32f4_discovery_lis302dl.h . DONE
3. Create stm32f4_discovery_lis3dsh.c file for defining low level functions for interacting with the sensor. DONE
4. Create support for printf function using usart so that values provided by sensor can be verified. DONE
5. Get values from the ssensor in the main program . DONE
6. Create portable code for any SPI and USART . PENDING 

