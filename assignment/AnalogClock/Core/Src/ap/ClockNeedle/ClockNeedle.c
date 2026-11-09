#include "ClockNeedle.h"
#include "../../driver/stepMotor/stepMotor.h"

extern TIM_HandleTypeDef htim9;

static stepMotor_pin_t stepMotorPinSec = {
		.port = {GPIOA, GPIOA, GPIOA, GPIOB},
		.pin  = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_4, GPIO_PIN_0},
};

static stepMotor_t motorSec = {
		.pin = &stepMotorPinSec,
		.stepCounter = 0
};

void ClockNeedle_init(void)
{
	HAL_TIM_Base_Start_IT(&htim9);
}

void ClockNeedle_ISR(void)
{
	StepMotor_stepPhase(&motorSec, STEP_MOTOR_DIR_CW);
}
