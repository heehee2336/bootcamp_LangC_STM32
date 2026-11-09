/*
 * HCSR.c
 *
 *  Created on: 2026. 10. 28.
 *      Author: kccistc
 */

#include "HCSR04.h"
#include "../../common/delay/delay.h"
#include <stdbool.h>

#define RISE_TIMEOUT_US		5
#define FALL_TIMEOUT_US 	50

static GPIO_TypeDef *trig_port;
static uint16_t trig_pin;
static GPIO_TypeDef *echo_port;
static uint16_t echo_pin;

TIM_HandleTypeDef *sr04_htim;

volatile sr04_state_t st;	//volatile : 변경하지 말라는 뜻
volatile uint32_t rise_cnt;
volatile uint32_t fall_cnt;

uint32_t start_ms;

static bool echo_is_high(void);
static void HCSR04_echoISR(void);
static uint32_t timer_diff(uint32_t start, uint32_t end);


void HCSR04_init(GPIO_TypeDef *t_port, uint16_t t_pin,
		GPIO_TypeDef *e_port, uint16_t e_pin,
		TIM_HandleTypeDef *htim)
{
	trig_port = t_port;
	trig_pin  = t_pin;
	echo_port = e_port;
	echo_pin  = e_pin;
	sr04_htim = htim;

	delay_init();
	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
	HAL_TIM_Base_Start(sr04_htim);
	st = STATE_IDLE;
}

hcsr04_status_t HCSR04_start(void)
{
	if(st == STATE_WAIT_RISE || st == STATE_WAIT_FALL) {
		return HCSR04_ERR_BUSY;
	}
	if(echo_is_high()) {
		return HCSR04_ERR_BUSY;
	}
	st = STATE_WAIT_RISE;
	start_ms = HAL_GetTick();

	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_SET);
	delay_us(10);
	HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);

	return HCSR04_OK;
}

static bool echo_is_high(void)	//bool 이기 때문에 return으로 참이면 1, 거짓이면 0 반환함.
{
	return (HAL_GPIO_ReadPin(echo_port, echo_pin) == GPIO_PIN_SET);
}

static void HCSR04_echoISR(void) //ISR : interrupt service routine(function)
{
	uint32_t cnt = __HAL_TIM_GET_COUNTER(sr04_htim);	//sr04_htim에 연결된 값을 counter 해서 반환하는 함수
	bool high = echo_is_high();

	if((st == STATE_WAIT_RISE) && high) {
		rise_cnt = cnt;
		st = STATE_WAIT_FALL;
	}
	else if ((st == STATE_WAIT_FALL) && !high) {
		fall_cnt = cnt;
		st = STATE_DONE;
	}
}

static uint32_t timer_diff(uint32_t start, uint32_t end) {

	uint32_t arr = __HAL_TIM_GET_AUTORELOAD(sr04_htim);

	if(end >= start)
	{
		return end - start;
	} else {
		return (arr - start) + end + 1;
	}

}

hcsr04_status_t HCSR04_getResult(uint32_t *echo_us)
{
	uint32_t elapsed_ms = HAL_GetTick() - start_ms;

	switch (st)
	{
	case STATE_WAIT_RISE:
		if(elapsed_ms <= RISE_TIMEOUT_US) {
			return HCSR04_IN_RPOGRESS;
		}
		st = STATE_IDLE;
		return HCSR04_ERR_NO_ECHO;
	case STATE_WAIT_FALL:
		if(elapsed_ms <= FALL_TIMEOUT_US) {
			return HCSR04_IN_RPOGRESS;
		}
		st = STATE_IDLE;
		return HCSR04_ERR_NO_ECHO;
	case STATE_DONE:
		uint32_t us = timer_diff(rise_cnt, fall_cnt);
		st = STATE_IDLE;
		*echo_us = us;
		return HCSR04_OK;
		break;
	case STATE_IDLE:
	default :
		return HCSR04_ERR_NO_STARTED;
	}
}
