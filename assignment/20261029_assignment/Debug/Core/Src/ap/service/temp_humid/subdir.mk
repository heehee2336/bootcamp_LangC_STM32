################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ap/service/temp_humid/temp_humid.c 

OBJS += \
./Core/Src/ap/service/temp_humid/temp_humid.o 

C_DEPS += \
./Core/Src/ap/service/temp_humid/temp_humid.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/ap/service/temp_humid/%.o Core/Src/ap/service/temp_humid/%.su Core/Src/ap/service/temp_humid/%.cyclo: ../Core/Src/ap/service/temp_humid/%.c Core/Src/ap/service/temp_humid/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-ap-2f-service-2f-temp_humid

clean-Core-2f-Src-2f-ap-2f-service-2f-temp_humid:
	-$(RM) ./Core/Src/ap/service/temp_humid/temp_humid.cyclo ./Core/Src/ap/service/temp_humid/temp_humid.d ./Core/Src/ap/service/temp_humid/temp_humid.o ./Core/Src/ap/service/temp_humid/temp_humid.su

.PHONY: clean-Core-2f-Src-2f-ap-2f-service-2f-temp_humid

