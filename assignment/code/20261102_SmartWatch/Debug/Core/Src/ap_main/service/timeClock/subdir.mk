################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ap_main/service/timeClock/timeClock.c 

OBJS += \
./Core/Src/ap_main/service/timeClock/timeClock.o 

C_DEPS += \
./Core/Src/ap_main/service/timeClock/timeClock.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/ap_main/service/timeClock/%.o Core/Src/ap_main/service/timeClock/%.su Core/Src/ap_main/service/timeClock/%.cyclo: ../Core/Src/ap_main/service/timeClock/%.c Core/Src/ap_main/service/timeClock/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-ap_main-2f-service-2f-timeClock

clean-Core-2f-Src-2f-ap_main-2f-service-2f-timeClock:
	-$(RM) ./Core/Src/ap_main/service/timeClock/timeClock.cyclo ./Core/Src/ap_main/service/timeClock/timeClock.d ./Core/Src/ap_main/service/timeClock/timeClock.o ./Core/Src/ap_main/service/timeClock/timeClock.su

.PHONY: clean-Core-2f-Src-2f-ap_main-2f-service-2f-timeClock

