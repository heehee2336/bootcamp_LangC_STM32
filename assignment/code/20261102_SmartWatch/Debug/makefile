################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/driver/led/subdir.mk
-include Core/Src/driver/lcd/subdir.mk
-include Core/Src/driver/fnd/subdir.mk
-include Core/Src/driver/ds3231/subdir.mk
-include Core/Src/driver/button/subdir.mk
-include Core/Src/driver/StepMotor/subdir.mk
-include Core/Src/driver/HCSR04/subdir.mk
-include Core/Src/driver/DHT11/subdir.mk
-include Core/Src/common/queue/subdir.mk
-include Core/Src/common/delay/subdir.mk
-include Core/Src/ap_main/service/timeClock/subdir.mk
-include Core/Src/ap_main/service/tempHumid/subdir.mk
-include Core/Src/ap_main/service/stopWatch/subdir.mk
-include Core/Src/ap_main/service/ruler/subdir.mk
-include Core/Src/ap_main/service/que/subdir.mk
-include Core/Src/ap_main/service/inputEvent/subdir.mk
-include Core/Src/ap_main/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := 261102_SmartWatch
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
261102_SmartWatch.elf \

MAP_FILES += \
261102_SmartWatch.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
261102_SmartWatch.list \


# All Target
all: main-build

# Main-build Target
main-build: 261102_SmartWatch.elf secondary-outputs

# Tool invocations
261102_SmartWatch.elf 261102_SmartWatch.map: $(OBJS) $(USER_OBJS) D:\AI_Semi_1\261102_SmartWatch\STM32F411RETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "261102_SmartWatch.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"D:\AI_Semi_1\261102_SmartWatch\STM32F411RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="261102_SmartWatch.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

261102_SmartWatch.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "261102_SmartWatch.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) 261102_SmartWatch.elf 261102_SmartWatch.list 261102_SmartWatch.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
