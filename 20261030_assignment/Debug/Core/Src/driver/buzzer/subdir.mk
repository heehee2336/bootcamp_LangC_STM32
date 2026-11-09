################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/driver/buzzer/buzzer.c 

OBJS += \
./Core/Src/driver/buzzer/buzzer.o 

C_DEPS += \
./Core/Src/driver/buzzer/buzzer.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/driver/buzzer/%.o Core/Src/driver/buzzer/%.su Core/Src/driver/buzzer/%.cyclo: ../Core/Src/driver/buzzer/%.c Core/Src/driver/buzzer/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-driver-2f-buzzer

clean-Core-2f-Src-2f-driver-2f-buzzer:
	-$(RM) ./Core/Src/driver/buzzer/buzzer.cyclo ./Core/Src/driver/buzzer/buzzer.d ./Core/Src/driver/buzzer/buzzer.o ./Core/Src/driver/buzzer/buzzer.su

.PHONY: clean-Core-2f-Src-2f-driver-2f-buzzer

