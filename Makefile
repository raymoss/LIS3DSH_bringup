
# Source files 
SRCS= main.c system_stm32f4xx.c stm32f4_discovery_lis3dsh.c

# Binary will be generated with this name (.elf, etc)
PROJ_NAME=LIS3DSH

# Put your STM32F4 library code directory here
STM_COMMON=../STM32F4-Discovery_FW_V1.1.0


# Normally you shouldn't need to change anything below this line!
#######################################################################################

CC=arm-none-eabi-gcc
SIZE=arm-none-eabi-size

#Debug
CFLAGS  = -ggdb -O0 -Wall -Tstm32_flash.ld 

CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -I.

# Include files from STM libraries
CFLAGS += -I$(STM_COMMON)/Libraries/CMSIS/Include -I$(STM_COMMON)/Libraries/CMSIS/ST/STM32F4xx/Include -I$(STM_COMMON)/Utilities/STM32F4-Discovery/
CFLAGS += -I$(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/inc

# add startup file to build
SRCS += $(STM_COMMON)/Libraries/CMSIS/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f4xx.s 
SRCS += stm32f4xx_flash.c
SRCS += stm32f4xx_gpio.c stm32f4xx_rcc.c stm32f4xx_spi.c stm32f4xx_syscfg.c
SRCS += stm32f4xx_tim.c misc.c 
OBJS = $(SRCS:.c=.o)

vpath %.c $(STM_COMMON)/Libraries/STM32F4xx_StdPeriph_Driver/src \
          $(STM_COMMON)/Utilities/STM32F4-Discovery 

SEMIHOSTING_FLAGS = --specs=rdimon.specs -lc -lrdimon 



.PHONY: all

all: $(PROJ_NAME).elf

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(SEMIHOSTING_FLAGS) $(CFLAGS) $^ -o $@ 
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin
clean:
	rm -f $(PROJ_NAME).elf 


#######################################################
# Debugging targets
#######################################################
gdb: all
	arm-none-eabi-gdb $(PROJ_NAME).elf

# Start OpenOCD GDB server (supports semihosting)
openocd: 
	/home/ray/openocd-0.8.0/bin/openocd -f board/stm32f4discovery.cfg 

