/*
 * distance.c
 *
 *  Created on: 2026. 10. 29.
 *      Author: kccistc
 */

#include "distance.h"
#include <stdbool.h>
#include "../../../driver/HCSR04/HCSR04.h"

extern TIM_HandleTypeDef htim10;

static uint32_t echo_us = 0;
static hcsr04_status_t hcsr04_status = HCSR04_ERR_NOT_STARTED;
static bool start_flag = false;

void distance_init(void){
	HCSR04_init(GPIOC, GPIO_PIN_5, GPIOC, GPIO_PIN_6, &htim10);
}

void distance_task(void) {
	if (HCSR04_getResult(&echo_us) == HCSR04_OK) {
		hcsr04_status = HCSR04_OK;
	}
}

void distance_request(void) {
	start_flag = true;
	HCSR04_start();
}

distance_status_t distance_get(uint16_t *distance_cm) {
	if(start_flag != true) {
		return DISTANCE_NOT_READY;
	}
	if(hcsr04_status != HCSR04_OK) {
		return DISTANCE_IN_PROGRESS;
	}
	start_flag = false;
	*distance_cm = echo_us * 17 / 1000;

	return DISTANCE_OK;
}

void distance_ISR(void){
	HCSR04_echoISR();
}
