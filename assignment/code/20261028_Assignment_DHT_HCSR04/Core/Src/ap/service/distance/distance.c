/*
 * distance.c
 *
 *  Created on: 2026. 10. 28.
 *      Author: kccistc
 */
#include "distance.h"

#define sr04_trigger_port GPIOC
#define sr04_trigger_pin GPIO_PIN_5
#define sr04_echo_port GPIOC
#define sr04_echo_pin GPIO_PIN_6

static uint32_t dist_cm;

extern TIM_HandleTypeDef htim10;

void distance_init(void)
{
	HCSR04_init(sr04_trigger_port, sr04_trigger_pin, sr04_echo_port, sr04_echo_pin, &htim10);
}


void distance_task(void)
{
	static uint32_t prevTime = 0;
	uint32_t echo_us;

	hcsr04_status_t st = HCSR04_getResult(&echo_us);
	if (st == HCSR04_OK) {
		dist_cm = echo_us / 58;          // TIM10 1tick = 1us
	}
	if (st == HCSR04_IN_PROGRESS) return;

	if (HAL_GetTick() - prevTime < 200) return;
	prevTime = HAL_GetTick();
	HCSR04_start();
}

uint32_t distance_getCm(void) { return dist_cm; }
