/*
 * ap_main.c
 *
 *  Created on: 2026. 10. 16.
 *      Author: rhoblack
 */

#include "ap_main.h"
#include "StopWatch/StopWatch.h"
#include "../driver/fnd/fnd.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM3) {
		fnd_show_number();
	}
}


int ap_main(void)
{
	StopWatch_init();

	while(1) // super loop
	{
		StopWatch_task();
		//fnd_show_number();
	}
	return 0;
}
