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
-include Core/Src/driver/stepMotor/subdir.mk
-include Core/Src/driver/lcd/subdir.mk
-include Core/Src/driver/joyStick/subdir.mk
-include Core/Src/driver/buzzer/subdir.mk
-include Core/Src/driver/button/subdir.mk
-include Core/Src/driver/HCSR04/subdir.mk
-include Core/Src/driver/DHT11/subdir.mk
-include Core/Src/common/delay/subdir.mk
-include Core/Src/ap/service/temp_humid/subdir.mk
-include Core/Src/ap/service/elevatorMoving/subdir.mk
-include Core/Src/ap/service/elevator/subdir.mk
-include Core/Src/ap/service/distance/subdir.mk
-include Core/Src/ap/service/display/subdir.mk
-include Core/Src/ap/subdir.mk
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


BUILD_ARTIFACT_NAME := project_elevator
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
project_elevator.elf \

MAP_FILES += \
project_elevator.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
project_elevator.list \


# All Target
all: main-build

# Main-build Target
main-build: project_elevator.elf secondary-outputs

# Tool invocations
project_elevator.elf project_elevator.map: $(OBJS) $(USER_OBJS) D:\LeeSH\stm\project_elevator\STM32F411RETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "project_elevator.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"D:\LeeSH\stm\project_elevator\STM32F411RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="project_elevator.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

project_elevator.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "project_elevator.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) default.size.stdout project_elevator.elf project_elevator.list project_elevator.map
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
