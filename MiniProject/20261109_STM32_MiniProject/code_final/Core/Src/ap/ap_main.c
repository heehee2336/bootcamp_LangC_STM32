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
#include "service/distance/distance.h"
#include "service/uart_protocol/uart_protocol.h"

#define SR04_ECHO_PIN		GPIO_PIN_6	// PC6 : HC-SR04 Echo (EXTI6, rising/falling)

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
	// 초음파 Echo : 상승/하강 엣지 시점의 타이머 값을 바로 기록해야 해서 가장 먼저 처리
	if(GPIO_Pin == SR04_ECHO_PIN) {
		dist_ISR();
		return;
	}
	if(GPIO_Pin == PHOTO_IT_PIN_1) {
		// for debug
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
		volatile uint16_t floor = FLOOR_1;
		Elevator_ISR(floor);
	}
	if(GPIO_Pin == PHOTO_IT_PIN_2) {
		// for debug
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
		volatile uint16_t floor = FLOOR_2;
		Elevator_ISR(floor);

	}
	if(GPIO_Pin == PHOTO_IT_PIN_3) {
		// for debug
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
		volatile uint16_t floor = FLOOR_3;
		Elevator_ISR(floor);

	}
}

void ap_init(void)
{
	HAL_TIM_Base_Start_IT(&htim9);
	Elevator_init();
	uartProto_init();	// USART2 수신 시작
}

void ap_main(void)
{
	ap_init();

	while(1) {
		elevatorMoving_task();
		Elevator_task();
		TH_task();
		dist_task();		// 100ms 마다 초음파 측정 반복
		uartProto_task();	// PC 명령 처리


	}
}



















