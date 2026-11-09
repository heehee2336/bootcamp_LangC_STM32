/*
 * distance.c
 *
 *  Created on: 2026. 10. 29.
 *      Author: kccistc
 */

#include "distance.h"
#include "../../../driver/HCSR04/HCSR04.h"


#define t_port 	GPIOC
#define t_pin 	GPIO_PIN_5
#define e_port 	GPIOC
#define e_pin 	GPIO_PIN_6

extern TIM_HandleTypeDef htim10;
static hcsr04_status_t st;

uint32_t distance = 0;
uint32_t echo_us;

void distance_init(void)
{
	HCSR04_init(t_port, t_pin, e_port, e_pin, &htim10);
}

void distance_task(void){

	static uint32_t prevTime = 0;
	uint32_t echo_us;

	st = HCSR04_getResult(&echo_us);
	if (st == HCSR04_OK) {
		distance = echo_us / 58;          // TIM10 1tick = 1us
	}
	if (st == HCSR04_IN_RPOGRESS) return;

	if (HAL_GetTick() - prevTime < 200) return;
	prevTime = HAL_GetTick();
	HCSR04_start();
}

uint32_t distance_get(void) {
	return distance;
}
