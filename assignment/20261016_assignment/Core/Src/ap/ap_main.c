/*
 * ap.c
 *
 *  Created on: 2026. 10. 16.
 *      Author: kccistc
 */


#include "ap_main.h"
#include "../driver/fnd/fnd.h"
#include "Stopwatch_Watch/Stopwatch_Watch.h"

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//	if(htim -> Instance == TIM3) {
//		fnd_show_number();
//	}
//}


int ap_main(void){

	System_init();

	while(1){
		System_task();
		fnd_show_number();
	}

	return 0;
}
