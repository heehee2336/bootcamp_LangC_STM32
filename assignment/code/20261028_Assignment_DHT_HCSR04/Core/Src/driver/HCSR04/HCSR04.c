/*
 * HCSR04.c
 *
 *  Created on: 2026. 10. 28.
 *      Author: kccistc
 */

#include "HCSR04.h"
#include "../../common/delay/delay.h"
#include <stdbool.h>


#define RISE_TIMEOUT_US 5
#define FALL_TIMEOUT_US 50


static GPIO_TypeDef *trig_port;
static uint16_t trig_pin;
static GPIO_TypeDef *echo_port;
static uint16_t echo_pin;

static TIM_HandleTypeDef *sr04_htim;

static volatile sr04_state_t state; // 컴파일러의 과도한 최적화를 막고, 변수의 값이 코드 흐름 밖에서 언제든 변경될 수 있음을 알려주기위해
static volatile uint32_t rise_cnt;
static volatile uint32_t fall_cnt;

uint32_t start_ms;

void HCSR04_init(GPIO_TypeDef *t_port, uint16_t t_pin, GPIO_TypeDef *e_port,
		uint16_t e_pin, TIM_HandleTypeDef *htim) {
	trig_port = t_port;
	trig_pin = t_pin;
	echo_port = e_port;
	echo_pin = e_pin;
	sr04_htim = htim;

	delay_init();
	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
	HAL_TIM_Base_Start(sr04_htim);

	state = STATE_IDLE;
}

hcsr04_status_t HCSR04_start(void) {
	if (state == STATE_WAIT_RISE || state == STATE_WAIT_FALL) {
		return HCSR04_ERR_BUSY;
	}
	if (echo_is_high()) {
		return HCSR04_ERR_BUSY;
	}
	state = STATE_WAIT_RISE;
	start_ms = HAL_GetTick();

	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_SET);
	delay_us(10);
	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);

	return HCSR04_OK;
}

bool echo_is_high(void) {
	return (HAL_GPIO_ReadPin(echo_port, echo_pin) == GPIO_PIN_SET);
}

void HCSR04_echoISR(void) {
	uint32_t cnt = __HAL_TIM_GET_COUNTER(sr04_htim); // sr04_htim timer에 연결된 카운터 값을 읽어오라는 의미
	bool high = echo_is_high();

	if ((state == STATE_WAIT_RISE) && high) { //rising edge에서 인터럽트 발생
		rise_cnt = cnt;
		state = STATE_WAIT_FALL;
	} else if ((state == STATE_WAIT_FALL) && !high) { //falling edge에서 인터럽트 발생
		fall_cnt = cnt;
		state = STATE_DONE;
	}
}

uint32_t timer_diff(uint32_t start, uint32_t end){
	uint32_t arr = __HAL_TIM_GET_AUTORELOAD(sr04_htim);

	if(end >= start){
		return end - start;
	}else{
		return(arr - start) + end + 1;
	}
}


hcsr04_status_t HCSR04_getResult(uint32_t *echo_us) {
	uint32_t elapsed_ms = HAL_GetTick() - start_ms;

	switch (state) {
	case STATE_WAIT_RISE:
		if(elapsed_ms <= RISE_TIMEOUT_US ){
			return HCSR04_IN_PROGRESS;
		}
		state = STATE_IDLE;
		return HCSR04_ERR_NO_ECHO;
	case STATE_WAIT_FALL:
		if(elapsed_ms <= FALL_TIMEOUT_US){
			return HCSR04_IN_PROGRESS;
		}
		state = STATE_IDLE;
		return HCSR04_ERR_NO_ECHO;
	case STATE_DONE:{
		uint32_t us = timer_diff(rise_cnt, fall_cnt);
		state = STATE_IDLE;
		*echo_us = us;
		return HCSR04_OK;
	}
	case STATE_IDLE:
	default:
		return HCSR04_ERR_NOT_STARTED;
	}
}

