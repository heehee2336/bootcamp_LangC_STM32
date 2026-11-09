/*
 * ap_main.c
 *
 *  Created on: 2026. 10. 28.
 *      Author: kccistc
 */


#include "ap_main.h"
#include "service/temp_humid/temp_humid.h"
#include "service/display/display.h"
#include "../driver/HCSR04/HCSR04.h"
#include "service/distance/distance.h"


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_6) {
		HCSR04_echoISR();
	}

}

int ap_main(void)
{
	TH_init();
	display_init();
	distance_init();

	while(1)
	{
		TH_task();
		distance_task();
		display_task();

	}
	return 0;
}
