/*
 * ap_main.c
 *
 *  Created on: 2026. 10. 28.
 *      Author: rhoblack
 */

#include "ap_main.h"
#include "service/temp_humid/temp_humid.h"
#include "service/display/display.h"
#include "../driver/button/button.h"
#include "../driver/PiezoBuzer/PiezoBuzer.h"
#include "service/distance/distance.h"
#include <stdio.h>

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_6) {
		dist_ISR();
	}
}

int ap_main(void) {

	button_t btn_dist;

	dist_init();
	TH_init();
	PB_init();
	display_init();

	button_init(&btn_dist, GPIOC, GPIO_PIN_0);

	while (1) {
		if (button_getState(&btn_dist) == ACT_RELEASE) {
			dist_request();
		}

		dist_task();
		TH_task();
		PB_task();
		display_task();
	}

	return 0;
}
