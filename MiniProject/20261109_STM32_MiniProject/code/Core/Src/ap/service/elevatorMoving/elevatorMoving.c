/*
 * ClockNeedle.c
 *
 *  Created on: 2026. 11. 5.
 *      Author: kccistc
 */


#include "elevatorMoving.h"
#include <stdbool.h>

volatile uint8_t isrFlag = 0;
stepMotor_pin_t stepMotorPin = {
		.port = {GPIOA,GPIOA,GPIOA,GPIOB},
		.pin  = {GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_4,GPIO_PIN_0},
};

stepMotor_t motor = {
		.pin = &stepMotorPin,
		.stepCounter = 0
};

stepMotor_dir_t cur_stepMotor_dir = STEP_MOTOR_DIR_CW;
static bool is_elevator_Run = false;

static void elevatorMoving_setFlag(void);
static void elevatorMoving_clearFlag(void);

static void elevatorMoving_setFlag(void)
{
	isrFlag = 1;
}

static void elevatorMoving_clearFlag(void)
{
	isrFlag = 0;
}

void elevatorMoving_setDirCW(void) {
	cur_stepMotor_dir = STEP_MOTOR_DIR_CW;
}

void elevatorMoving_setDirCCW(void) {
	cur_stepMotor_dir = STEP_MOTOR_DIR_CCW;
}

void elevatorMoving_ISR(void)
{
	elevatorMoving_setFlag();
}


void elevatorMoving_task(void)
{
	if (isrFlag == 0){
		return;
	}
	elevatorMoving_clearFlag();
	if (is_elevator_Run == true) {
		StepMotor_stepPhase(&motor, cur_stepMotor_dir);
	} else return;
}

void elevatorMoving_Run(void){
	is_elevator_Run = true;
}

void elevatorMoving_Stop(void){
	is_elevator_Run = false;
	StepMotor_release(&motor);
}










































