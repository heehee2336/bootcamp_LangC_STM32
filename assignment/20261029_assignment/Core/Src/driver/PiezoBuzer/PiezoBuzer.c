/*
 * PiezoBuzer.c
 *
 *  Created on: 2026. 10. 29.
 *      Author: kccistc
 */


#include "PiezoBuzer.h"
#include "../button/button.h"

extern TIM_HandleTypeDef htim1;

button_t btn_dist;
button_t btn_arr;
button_t btn_duty;


uint32_t arr = 50000;
uint32_t arr_up;
uint32_t duty = 0;
uint32_t duty_up;

void PB_init(void) {
	button_init(&btn_arr, GPIOC, GPIO_PIN_1);
	button_init(&btn_duty, GPIOC, GPIO_PIN_3);
}

void PB_task(void) {

	if (button_getState(&btn_arr) == ACT_RELEASE)
	{
		arr = arr - 1000;
		if(arr == 0)
		{
			arr = 1000 -1;
		}
	}
	else if (button_getState(&btn_duty) == ACT_RELEASE)
	{
		duty = duty + 10;
		if (duty == 100)
		{
			duty = 50;
		}
	}

	__HAL_TIM_SET_AUTORELOAD(&htim1, arr);//1kHz의 주파수 생성
	arr_up = __HAL_TIM_GET_AUTORELOAD(&htim1);
	duty_up = (arr_up * duty)/100 - 1;
	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4, duty_up);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
}
