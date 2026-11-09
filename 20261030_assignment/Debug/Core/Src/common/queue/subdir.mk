################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/common/queue/que_buffer.c 

OBJS += \
./Core/Src/common/queue/que_buffer.o 

C_DEPS += \
./Core/Src/common/queue/que_buffer.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/common/queue/%.o Core/Src/common/queue/%.su Core/Src/common/queue/%.cyclo: ../Core/Src/common/queue/%.c Core/Src/common/queue/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-common-2f-queue

clean-Core-2f-Src-2f-common-2f-queue:
	-$(RM) ./Core/Src/common/queue/que_buffer.cyclo ./Core/Src/common/queue/que_buffer.d ./Core/Src/common/queue/que_buffer.o ./Core/Src/common/queue/que_buffer.su

.PHONY: clean-Core-2f-Src-2f-common-2f-queue

