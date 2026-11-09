/*
 * StopWatch.c
 *
 *  Created on: 2026. 10. 16.
 *      Author: rhoblack
 */

#include "StopWatch.h"
#include "../../driver/button/button.h"
#include "../../driver/led/led.h"
#include "../../driver/fnd/fnd.h"

typedef enum {STOP, RUN, CLEAR} stopWatchState_t;

static void StopWatch_stop();
static void StopWatch_run();
static void StopWatch_clear();
static void StopWatch_setState(stopWatchState_t s);
static stopWatchState_t StopWatch_getState(void);
static void StopWatch_incCounter(void);
static void StopWatch_clearCounter(void);
static void StopWatch_setCounter(uint16_t c);
static StopWatchData_t StopWatch_getCounter(void);
static void StopWatch_dispTime(void);

static StopWatchData_t stopWatchData;

static stopWatchState_t stopWatch_state;

static uint16_t counter;

static led_t led_run;
static led_t led_stop;

static button_t btn_run_stop;
static button_t btn_clear;

static led_t led_watch, led_stopWatch, led_stop, led_run;

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

static void StopWatch_setState(stopWatchState_t s)
{
	stopWatch_state = s;
}

static stopWatchState_t StopWatch_getState(void)
{
	return stopWatch_state;
}

static void StopWatch_incCounter(void)
{
	counter++;
}

static void StopWatch_clearCounter(void)
{
	counter = 0;
}

static void StopWatch_setCounter(uint16_t c)
{
	counter = c;
}

static StopWatchData_t StopWatch_getCounter(void)
{
	return stopWatchData;
}


void StopWatch_init(void)
{
	stopWatch_state = STOP;
	stopWatchData.hour = 0;
	stopWatchData.min = 0;
	stopWatchData.sec = 0;
	stopWatchData.msec = 0;

	led_init(&led_stopWatch, GPIOC, GPIO_PIN_14);
	led_init(&led_run, GPIOB, GPIO_PIN_13);
	led_init(&led_stop, GPIOC, GPIO_PIN_4);

	button_init(&btn_run_stop, GPIOC, GPIO_PIN_1);
	button_init(&btn_clear, GPIOC, GPIO_PIN_3);

	HAL_TIM_Base_Start_IT(&htim3);

	HAL_TIM_Base_Start_IT(&htim4);
}

void StopWatch_task(void)
{
	switch(stopWatch_state)
	{
	case STOP:
		StopWatch_stop();
		led_off(&led_watch);
		led_on(&led_stopWatch);
		led_off(&led_run);
		led_on(&led_stop);
		break;
	case RUN:
		StopWatch_run();
		led_off(&led_watch);
		led_on(&led_stopWatch);
		led_on(&led_run);
		led_off(&led_stop);
		break;
	case CLEAR:
		StopWatch_clear();
		break;
	default:
		StopWatch_stop();
		break;
	}
}

static void StopWatch_stop()
{
	if (button_getState(&btn_run_stop) == ACT_PUSH) {
		StopWatch_setState(RUN);
	}
	else if (button_getState(&btn_clear) == ACT_RELEASE) {
		StopWatch_setState(CLEAR);
	}

	uint16_t num;
	StopWatchData_t stopWatchData = StopWatch_getCounter();

	num = (stopWatchData.min * 100) + stopWatchData.sec;
	fnd_set_num(num);
}

static void StopWatch_run()
{
	if (button_getState(&btn_run_stop) == ACT_PUSH) {
		StopWatch_setState(STOP);
	}

	StopWatch_dispTime();
//
//	fnd_set_num(num);
//
//	if (num%10 < 5) {
//		fnd_set_dp(FND_DIGIT_2, FND_DP_ON);
//	}
//	else {
//		fnd_set_dp(FND_DIGIT_2, FND_DP_OFF);
//	}
//
//
}

static void StopWatch_clear()
{
	uint16_t num;

	num = 0;

	stopWatchData.hour = 0;
	stopWatchData.min = 0;
	stopWatchData.sec = 0;
	stopWatchData.msec = 0;

	num = (stopWatchData.min * 100) + stopWatchData.sec;
	fnd_set_num(num);
	StopWatch_setState(STOP);
}



void StopWatch_inMSec(void)
{
	if(stopWatchData.msec < 99){
		stopWatchData.msec++;
		return;
	}
	stopWatchData.msec = 0;
	if(stopWatchData.sec < 59){
		stopWatchData.sec++;
		return;
	}
	stopWatchData.sec = 0;
	if(stopWatchData.min < 59){
		stopWatchData.min++;
		return;
	}
	stopWatchData.min = 0;

	if(stopWatchData.hour < 23){
		stopWatchData.hour++;
		return;
	}
	stopWatchData.hour = 0;
}


void StopWatch_ISR(void)
{
	if(stopWatch_state == RUN){
		StopWatch_inMSec();
	}
}


static void StopWatch_dispTime(void)
{
	uint16_t num;
	num = (stopWatchData.min * 100) + stopWatchData.sec;
	fnd_set_num(num);

	if (stopWatchData.msec < 50) {
		fnd_set_dp(FND_DIGIT_2, FND_DP_ON);
	}
	else {
		fnd_set_dp(FND_DIGIT_2, FND_DP_OFF);
	}
}

