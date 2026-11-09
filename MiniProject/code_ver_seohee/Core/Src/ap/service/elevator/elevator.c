/*
 * elevator.c
 *
 *  Created on: 2026. 11. 5.
 *      Author: kccistc
 */


#include "elevator.h"
#include "../../../driver/lcd/lcd.h"
#include "../../../driver/button/button.h"
#include "../../../driver/joyStick/joyStick.h"
#include "../elevatorMoving/elevatorMoving.h"
#include "../display/display.h"
#include "../temp_humid/temp_humid.h"
#include "../distance/distance.h"
#include "../sound/sound.h"
#include <stdbool.h>

typedef enum {
	MIDDLE,
	BOTTOM,
	TOP,
	ESCAPE_EMERGENCY
} elevator_emergency_state_t;

static uint16_t current_floor;
static uint16_t target_floor;
static elevator_state_t elevator_state;
static elevator_emergency_state_t elevator_emergency_state;
static elevator_mode_t elevator_mode;

button_t btn_floor_1;
button_t btn_floor_2;
button_t btn_floor_3;

static bool is_target_floor_arrive[3] = {false};
static uint16_t elevator_blockingMotor_CW;
static uint16_t elevator_blockingMotor_CCW;

static void is_target_floor_Clear(void);
static void Elevator_setTargetFloor();

void Elevator_init(void){

	current_floor = FLOOR_1;
	target_floor = FLOOR_1;
	elevator_state = IDLE;
	elevator_mode = EMERGENCY;
	elevator_emergency_state = MIDDLE;
	elevator_blockingMotor_CW = 0;
	elevator_blockingMotor_CCW = 0;
	is_target_floor_Clear();
	js_init();
	TH_init();
	dist_init();
	sound_init();
	disp_init();
	button_init(&btn_floor_3,GPIOC,GPIO_PIN_3);
	button_init(&btn_floor_2,GPIOC,GPIO_PIN_1);
	button_init(&btn_floor_1,GPIOC,GPIO_PIN_0);
}

static void Elevator_setTargetFloor() {


	target_floor = current_floor;

	if (button_getState(&btn_floor_1) == ACT_RELEASE) {
		target_floor = FLOOR_1;
	}
	else if(button_getState(&btn_floor_2) == ACT_RELEASE){
		target_floor = FLOOR_2;
	}
	else if(button_getState(&btn_floor_3) == ACT_RELEASE){
		target_floor = FLOOR_3;
	}

	if (current_floor < target_floor)
	{
		elevator_state = RISING;
	}
	else if (current_floor > target_floor)
	{
		elevator_state = FALLING;
	}
}

void Elevator_ISR(uint16_t floor)
{
	is_target_floor_arrive[floor] = true;
}

static void is_target_floor_Clear(void)
{
	is_target_floor_arrive[0] = false;
	is_target_floor_arrive[1] = false;
	is_target_floor_arrive[2] = false;
}

static void Elevator_normal_task(void)
{
	switch(elevator_state){

	case IDLE :
		Elevator_setTargetFloor(&current_floor);
		break;
	case RISING :
		elevatorMoving_setDirCW();
		elevatorMoving_Run();
		sound_rising();
		sound_falling();
		if (is_target_floor_arrive[target_floor] == true) {
			current_floor = target_floor;
			elevator_state = ARRIVE;
		}
		break;
	case FALLING :
		elevatorMoving_setDirCCW();
		elevatorMoving_Run();
		if (is_target_floor_arrive[target_floor] == true) {
			current_floor = target_floor;
			elevator_state = ARRIVE;
		}
		break;
	case ARRIVE :
		is_target_floor_Clear();
		elevatorMoving_Stop();
		sound_arriving();
		elevator_state = IDLE;
		break;
	default :
		elevator_state = IDLE;
		break;
	}

	disp_task(elevator_state, current_floor, target_floor);
}

static void Elevator_manual_operation(void) {

	cordinate_XY js_value;
	js_getValue(&js_value);

	if (js_value.Y_cord > 60 && elevator_blockingMotor_CW == 0) {
		elevatorMoving_setDirCW();
		elevatorMoving_Run();
		elevator_blockingMotor_CCW = 0;
	}
	else if (js_value.Y_cord < 40 && elevator_blockingMotor_CCW == 0) {
		elevatorMoving_setDirCCW();
		elevatorMoving_Run();
		elevator_blockingMotor_CW = 0;
	}
	else {
		elevatorMoving_Stop();
	}
}

uint16_t elevator_emergency;

static void Elevator_checkEmergency(void)
{
	dht11_data_t thData;
	TH_getTempHumid(&thData);

	if (thData.t_int >= 28){
		elevator_mode = EMERGENCY;
	}
}
static void Elevator_escapeEmergency(void)
{
	elevatorMoving_setDirCCW();
	elevatorMoving_Run();
	if (is_target_floor_arrive[FLOOR_1] == true) {
		current_floor = FLOOR_1;
		target_floor = FLOOR_1;
		elevator_mode = NORMAL;
		elevator_state = ARRIVE;
		elevator_emergency_state = MIDDLE;
	}

}




static void Elevator_emergency_task(void) {

	uint16_t distance_cm = 0;
	disp_tempHumid();
	sound_emergency();
	switch(elevator_emergency_state) {
	case MIDDLE:
		Elevator_manual_operation();
		if (button_getState(&btn_floor_1) == ACT_RELEASE) {
			is_target_floor_Clear();
			elevator_blockingMotor_CW = 0;
			elevator_blockingMotor_CCW = 0;
			elevator_emergency_state = ESCAPE_EMERGENCY;
		}
		break;
	case BOTTOM:
		elevator_blockingMotor_CCW = 1;
		elevator_emergency_state = MIDDLE;
		break;
	case TOP:
		elevator_blockingMotor_CW = 1;
		elevator_emergency_state = MIDDLE;
		break;
	case ESCAPE_EMERGENCY:
		Elevator_escapeEmergency();
		break;
	default:
		break;
	}
}


void Elevator_task(void)
{
	switch(elevator_mode) {
	case NORMAL:
		Elevator_normal_task();
		Elevator_checkEmergency();
		break;
	case EMERGENCY:
		Elevator_emergency_task();
		break;
	default:
		break;
	}

}
















