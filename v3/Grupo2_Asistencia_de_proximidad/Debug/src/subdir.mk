################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/buzzer.c \
../src/cr_startup_lpc175x_6x.c \
../src/crp.c \
../src/dac.c \
../src/dma.c \
../src/gpio.c \
../src/hcsr04.c \
../src/leds.c \
../src/main.c \
../src/proximity_assistant.c \
../src/sound_ctrl.c \
../src/timer.c \
../src/transistor.c 

C_DEPS += \
./src/buzzer.d \
./src/cr_startup_lpc175x_6x.d \
./src/crp.d \
./src/dac.d \
./src/dma.d \
./src/gpio.d \
./src/hcsr04.d \
./src/leds.d \
./src/main.d \
./src/proximity_assistant.d \
./src/sound_ctrl.d \
./src/timer.d \
./src/transistor.d 

OBJS += \
./src/buzzer.o \
./src/cr_startup_lpc175x_6x.o \
./src/crp.o \
./src/dac.o \
./src/dma.o \
./src/gpio.o \
./src/hcsr04.o \
./src/leds.o \
./src/main.o \
./src/proximity_assistant.o \
./src/sound_ctrl.o \
./src/timer.o \
./src/transistor.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -DCORE_M3 -D__USE_CMSIS=CMSISv2p00_LPC17xx -D__LPC17XX__ -D__REDLIB__ -I"/Users/agustin/Downloads/TP_Integrador-4-ED3/Grupo2_Asistencia_de_proximidad/firmware/inc" -I"/Users/agustin/Downloads/CMSISv2p00_LPC17xx/inc" -I"/Users/agustin/Downloads/CMSISv2p00_LPC17xx/Drivers/inc" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m3 -mthumb -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/buzzer.d ./src/buzzer.o ./src/cr_startup_lpc175x_6x.d ./src/cr_startup_lpc175x_6x.o ./src/crp.d ./src/crp.o ./src/dac.d ./src/dac.o ./src/dma.d ./src/dma.o ./src/gpio.d ./src/gpio.o ./src/hcsr04.d ./src/hcsr04.o ./src/leds.d ./src/leds.o ./src/main.d ./src/main.o ./src/proximity_assistant.d ./src/proximity_assistant.o ./src/sound_ctrl.d ./src/sound_ctrl.o ./src/timer.d ./src/timer.o ./src/transistor.d ./src/transistor.o

.PHONY: clean-src

