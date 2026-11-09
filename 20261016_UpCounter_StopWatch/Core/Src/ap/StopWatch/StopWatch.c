/*
 * StopWatch.c
 *
 *  Created on: 2026. 10. 16.
 *      Author: kccistc
 */
#include "StopWatch.h"
#include "../../driver/button/button.h"
#include "../../driver/led/led.h"
#include "../../driver/fnd/fnd.h"

typedef enum {STOP, RUN, CLEAR} stopwatch_t;

static void StopWatch_setState(stopwatch_t s);
static stopwatch_t StopWatch_getState(void);
static void StopWatch_incCounter(void);
static void StopWatch_clearCounter(void);
static void StopWatch_setCounter(uint16_t c);
static uint16_t StopWatch_getCounter(void);

static void StopWatch_stop(void);
static void StopWatch_run(void);
static void StopWatch_clear(void);

static stopwatch_t stopWatch_state;
static uint16_t counter;


static led_t led_stop;
static led_t led_run;

static button_t btn_run_stop;
static button_t btn_clear;

extern TIM_HandleTypeDef htim3;

static void StopWatch_setState(stopwatch_t s)
{
	stopWatch_state = s;
}

static stopwatch_t StopWatch_getState(void)
{
	return stopWatch_state;
}

static void StopWatch_incCounter(void)
{
	counter ++;
}

static void StopWatch_clearCounter(void)
{
	counter = 0;
}

static void StopWatch_setCounter(uint16_t c)
{
	counter = c;
}

static uint16_t StopWatch_getCounter(void)
{
	return counter;
}



void StopWatch_init(void)
{
	stopWatch_state = STOP;
	counter = 0;

	led_init(&led_stop, GPIOB, GPIO_PIN_13);
	led_init(&led_run, GPIOC, GPIO_PIN_4);

	button_init(&btn_run_stop, GPIOC, GPIO_PIN_1);
	button_init(&btn_clear, GPIOC, GPIO_PIN_3);

	HAL_TIM_Base_Start_IT(&htim3);
}

void StopWatch_task(void)
{
	switch(stopWatch_state)
	{
	case STOP :
		StopWatch_stop();
		break;
	case RUN :
		StopWatch_run();
		break;
	case CLEAR :
		StopWatch_clear();
		break;
	default :
		StopWatch_stop();
		break;
	}
}

static void StopWatch_stop(void)
{


	if(button_getState(&btn_run_stop) == ACT_PUSH){
		StopWatch_setState(RUN);
	}
	else if (button_getState(&btn_clear) == ACT_RELEASE)
	{
		StopWatch_setState(CLEAR);
	}

	uint16_t num = StopWatch_getCounter();
	fnd_set_num(num);
}

static void StopWatch_run(void)
{
	if(button_getState(&btn_run_stop) == ACT_PUSH){
		StopWatch_setState(STOP);
	}

	static uint32_t prevTime = 0;
	uint32_t curTime = HAL_GetTick();
	if(curTime - prevTime < 99){
		return;
	}
	prevTime = curTime;

	uint16_t num = StopWatch_getCounter();

	if(num == 9999){
		StopWatch_clearCounter();
	}
	else {
		StopWatch_incCounter();
	}

	num = StopWatch_getCounter();
	fnd_set_num(num);
}

static void StopWatch_clear(void)
{
	uint16_t num;
	StopWatch_clearCounter();
	num = StopWatch_getCounter();

	fnd_set_num(num);
	StopWatch_setState(STOP);
}
