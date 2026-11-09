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

static stepMotor_pin_t stepMotorPinMin = {
		.port = {GPIOC, GPIOC, GPIOC, GPIOA},
		.pin  = {GPIO_PIN_8, GPIO_PIN_6, GPIO_PIN_5, GPIO_PIN_12},
};

static stepMotor_t motorMin = {
		.pin = &stepMotorPinMin,
		.stepCounter = 0
};
//
static stepMotor_pin_t stepMotorPinHour = {
		.port = {GPIOA, GPIOB, GPIOB, GPIOB},
		.pin  = {GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_2, GPIO_PIN_1},
};

static stepMotor_t motorHour = {
		.pin = &stepMotorPinHour,
		.stepCounter = 0
};

enum {
	TOTAL_STEPS = 4096,
	CYCLE_RESOLUTION_SEC = 60,
	CYCLE_RESOLUTION_MIN = 60,
	CYCLE_RESOLUTION_HOUR = 12
};

typedef struct {
	volatile uint32_t pendingSteps;
	uint16_t stepRemainder;
} needleStepState_t;

static needleStepState_t secStepState;
static needleStepState_t minStepState;
static needleStepState_t hourStepState;

static void ClockNeedle_requestStep(needleStepState_t *state,
		uint16_t cycleResolution)
{
	uint32_t accumulatedSteps = state->stepRemainder + TOTAL_STEPS;
	uint32_t steps = accumulatedSteps / cycleResolution;

	state->stepRemainder = accumulatedSteps % cycleResolution;
	state->pendingSteps += steps;
}

void ClockNeedle_init(void)
{
	HAL_TIM_Base_Start_IT(&htim9);
}

void ClockNeedle_ISR(void)
{
	if (secStepState.pendingSteps > 0) {
		StepMotor_stepPhase(&motorSec, STEP_MOTOR_DIR_CW);
		secStepState.pendingSteps--;
	}

	if (minStepState.pendingSteps > 0) {
		StepMotor_stepPhase(&motorMin, STEP_MOTOR_DIR_CW);
		minStepState.pendingSteps--;
	}

	if (hourStepState.pendingSteps > 0) {
		StepMotor_stepPhase(&motorHour, STEP_MOTOR_DIR_CW);
		hourStepState.pendingSteps--;
	}
}

void ClockNeedle_SecStep(void)
{
	ClockNeedle_requestStep(&secStepState, CYCLE_RESOLUTION_SEC);
}

void ClockNeedle_MinStep(void)
{
	ClockNeedle_requestStep(&minStepState, CYCLE_RESOLUTION_MIN);
}

void ClockNeedle_HourStep(void)
{
	ClockNeedle_requestStep(&hourStepState, CYCLE_RESOLUTION_HOUR);
}
