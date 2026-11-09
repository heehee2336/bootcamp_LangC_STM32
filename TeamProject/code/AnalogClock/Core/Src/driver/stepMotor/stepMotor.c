/*
 * stepMotor.c
 *
 *  Created on: 2026. 10. 23.
 *      Author: kccistc
 */

#include "stepMotor.h"
#include "../button/button.h"
#include "stm32f4xx_hal.h"

extern TIM_HandleTypeDef htim9;


typedef enum {
	SEC_ROUND, MIN_ROUND, HOUR_ROUND
}align_State_t;

align_State_t alignStateMode;

static uint8_t sec_align = 0;
static uint8_t min_align = 0;
uint16_t count = 0;
uint16_t count_set = 0;

button_t btn_up;
button_t btn_down;

stepMotor_pin_t stepMotorPinSec = { .port = { GPIOA, GPIOA, GPIOA, GPIOB },
		.pin = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_4, GPIO_PIN_0 } };

stepMotor_pin_t stepMotorPinMin = { .port = { GPIOC, GPIOC, GPIOC, GPIOA },
		.pin = { GPIO_PIN_8, GPIO_PIN_6, GPIO_PIN_5, GPIO_PIN_12 } };

stepMotor_pin_t stepMotorPinHour = {
		.port = {GPIOA, GPIOB, GPIOB, GPIOB},
		.pin = {GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_2, GPIO_PIN_1}
};

stepMotor_t motor_sec = { .pin = &stepMotorPinSec, .stepCounter = 0 };

stepMotor_t motor_min = { .pin = &stepMotorPinMin, .stepCounter = 0 };

stepMotor_t motor_hour = {
		.pin = &stepMotorPinHour,
		.stepCounter = 0
};

static uint8_t step_table[STEP_COUNT] = { 0x01, 0x03, 0x02, 0x06, 0x04, 0x0c,
		0x08, 0x09 };

static void StepMotor_writePattern(stepMotor_t *m, uint8_t step) {
	for (int i = 0; i < 4; i++) {
		GPIO_PinState s = (step & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET;
		HAL_GPIO_WritePin(m->pin->port[i], m->pin->pin[i], s);
	}
}

void StepMotor_stepPhase(stepMotor_t *m, stepMotor_dir_t dir) {
	if (dir == STEP_MOTOR_DIR_CW) {
		m->stepCounter = (m->stepCounter + STEP_COUNT - 1) % STEP_COUNT;
	} else {
		m->stepCounter = (m->stepCounter + 1) % STEP_COUNT;
	}
	StepMotor_writePattern(m, step_table[m->stepCounter]);
}

void StepMotor_release(stepMotor_t *m) {
	StepMotor_writePattern(m, 0x00);
}

void motor_button_init(){
	button_init(&btn_up, GPIOC, GPIO_PIN_1);
	button_init(&btn_down, GPIOC, GPIO_PIN_3);

}

void sec_motor_align() {
	motor_button_init();

	for (int i = 0; i < 4096; i++) {
		StepMotor_stepPhase(&motor_sec, STEP_MOTOR_DIR_CW);
		HAL_Delay(2);
		if (button_getState(&btn_up) == ACT_RELEASE) {
			sec_align = 1;
			break;
		}
	}

}

void min_motor_align() {
	motor_button_init();

	for (int i = 0; i < 4096; i++) {
		StepMotor_stepPhase(&motor_min, STEP_MOTOR_DIR_CW);
		HAL_Delay(2);
		if (button_getState(&btn_up) == ACT_RELEASE) {
			min_align = 1;
			break;
		}
	}

}

void hour_motor_align() {
	motor_button_init();

	for (int i = 0; i < 4096; i++) {
		StepMotor_stepPhase(&motor_hour, STEP_MOTOR_DIR_CW);
		HAL_Delay(2);
		if (button_getState(&btn_up) == ACT_RELEASE) {
			break;
		}
	}

}



void StepMotor_task(void) {


	HAL_TIM_Base_Start_IT(&htim9);


	switch (alignStateMode) {
	case SEC_ROUND:
		sec_motor_align();
		alignStateMode = MIN_ROUND;

	case MIN_ROUND:
		min_motor_align();
		alignStateMode = HOUR_ROUND;
	case HOUR_ROUND:
			hour_motor_align();
			break;
//			alignStateMode = HOUR_ROUND;
	default:
		alignStateMode = SEC_ROUND;
		break;
	}


	HAL_Delay(1);
}

