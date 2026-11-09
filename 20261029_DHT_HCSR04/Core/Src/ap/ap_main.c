/*
 * ap_main.c
 *
 *  Created on: 2026. 10. 28.
 *      Author: rhoblack
 */


#include "ap_main.h"
#include <stdio.h>
#include "service/temp_humid/temp_humid.h"
#include "service/display/display.h"
#include "service/distance/distance.h"
#include "../driver/button/button.h"
//#include "../driver/HCSR04/HCSR04.h"


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_6) {
		distance_ISR();
	}
}

int ap_main(void)
{
	button_t btn_distance;
//	uint16_t dist_cm;

	TH_init();
	display_init();
	distance_init();

	button_init(&btn_distance, GPIOC, GPIO_PIN_0);

	while(1)
	{
		if(button_getState(&btn_distance) == ACT_RELEASE){
			distance_request();
			printf("distance requested!\n");
		}

		TH_task();
		display_task();
		distance_task();

	}

	return 0;
}
