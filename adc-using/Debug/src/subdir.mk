################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/adc-using.c \
../src/cr_startup_lpc175x_6x.c \
../src/crp.c 

OBJS += \
./src/adc-using.o \
./src/cr_startup_lpc175x_6x.o \
./src/crp.o 

C_DEPS += \
./src/adc-using.d \
./src/cr_startup_lpc175x_6x.d \
./src/crp.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_CMSIS=CMSISv2p00_LPC17xx -D__LPC17XX__ -D__REDLIB__ -I"
	@echo 'Finished building: $<'
	@echo ' '


