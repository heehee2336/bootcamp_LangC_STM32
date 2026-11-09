/*
 * ap_main.c
 *
 *  Created on: 2026. 11. 5.
 *      Author: kccistc
 */

#include "ap_main.h"
#include "service/elevator/elevator.h"
#include "service/elevatorMoving/elevatorMoving.h"
#include "service/temp_humid/temp_humid.h"

#define PHOTO_IT_GPIO_1		GPIOA
#define PHOTO_IT_PIN_1		GPIO_PIN_15
#define PHOTO_IT_GPIO_2		GPIOC
#define PHOTO_IT_PIN_2		GPIO_PIN_10
#define PHOTO_IT_GPIO_3		GPIOC
#define PHOTO_IT_PIN_3		GPIO_PIN_12

extern TIM_HandleTypeDef htim9;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM9)
	{

		elevatorMoving_ISR();
	}
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if(GPIO_Pin == PHOTO_IT_PIN_1) {
		volatile uint16_t floor = FLOOR_1;
		Elevator_ISR(floor);
	}
	if(GPIO_Pin == PHOTO_IT_PIN_2) {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
		volatile uint16_t floor = FLOOR_2;
		Elevator_ISR(floor);

	}
	if(GPIO_Pin == PHOTO_IT_PIN_3) {
		volatile uint16_t floor = FLOOR_3;
		Elevator_ISR(floor);

	}
}

void ap_init(void)
{
	HAL_TIM_Base_Start_IT(&htim9);
	Elevator_init();
}

void ap_main(void)
{
	ap_init();

	while(1) {
		elevatorMoving_task();
		Elevator_task();
		TH_task();


	}
}



















