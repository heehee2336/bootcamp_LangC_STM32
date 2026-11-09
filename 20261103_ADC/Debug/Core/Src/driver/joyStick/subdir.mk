################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/driver/joyStick/joyStick.c 

OBJS += \
./Core/Src/driver/joyStick/joyStick.o 

C_DEPS += \
./Core/Src/driver/joyStick/joyStick.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/driver/joyStick/%.o Core/Src/driver/joyStick/%.su Core/Src/driver/joyStick/%.cyclo: ../Core/Src/driver/joyStick/%.c Core/Src/driver/joyStick/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-driver-2f-joyStick

clean-Core-2f-Src-2f-driver-2f-joyStick:
	-$(RM) ./Core/Src/driver/joyStick/joyStick.cyclo ./Core/Src/driver/joyStick/joyStick.d ./Core/Src/driver/joyStick/joyStick.o ./Core/Src/driver/joyStick/joyStick.su

.PHONY: clean-Core-2f-Src-2f-driver-2f-joyStick

