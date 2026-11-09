/*
 * StopWatch.c
 *
 *  Created on: 2026. 10. 16.
 *      Author: kccistc
 */
#include "Stopwatch_Watch.h"

#include "../../driver/button/button.h"
#include "../../driver/led/led.h"
#include "../../driver/fnd/fnd.h"

typedef enum {W, SW} system_mode_t;
typedef enum {STOP, RUN, CLEAR} stopwatch_t;
typedef enum {NOMAL, TIMER, UP} watch_t;
typedef enum {READY, START} timer_t;

static system_mode_t system_state;
static void StopWatch_task(void);
static void Watch_task(void);


static void StopWatch_setState(stopwatch_t s);
static stopwatch_t StopWatch_getState(void);
static void StopWatch_incCounter(void);
static void Stopwatch_clearCounter(void);
static void Stopwatch_setCounter(uint16_t c);
static uint16_t Stopwatch_getCounter(void);

static void Watch_setState(watch_t s);
static watch_t Watch_getState(void);
static void Watch_incCounter(void);
static void Watch_timer_decCounter(void);
static void Watch_clearCounter(void);
static void Watch_clearCounter(void);
static uint16_t Watch_getCounter(void);

static void StopWatch_stop(void);
static void StopWatch_run(void);
static void StopWatch_clear(void);
static void Watch_nomal(void);
static void Watch_timer(void);
static void Watch_up(void);

static void led_stopwatch_state(void);
static void led_watch_state(void);
static void led_stop_state(void);
static void led_run_state(void);
static void led_clear_state(void);
static void led_timer_stop(void);

static stopwatch_t StopWatch_state;
static watch_t Watch_state;
static timer_t Timer_state;
static uint16_t counter_sw;
static uint16_t counter_w;
static uint16_t counter_t;
static uint16_t watch_min = 0;
static uint16_t watch_sec = 0;
static uint16_t timer_sec = 0;
static uint16_t timer_min = 0;
static uint32_t prevTime_stopwatch = 0;
static uint32_t prevTime_watch = 0;
static uint32_t prevTime_timer = 0;
static watch_t up_setting_state = 0;

static led_t led_run;
static led_t led_stop;
static led_t led_watch;
static led_t led_stopwatch;

static button_t btn_mode;
static button_t btn_1;
static button_t btn_2;

//extern TIM_HandleTypeDef htim3;

static void StopWatch_setState(stopwatch_t s)
{
	StopWatch_state = s;
}

static void Watch_setState(watch_t s)
{
	Watch_state = s;
}

static stopwatch_t StopWatch_getState(void)
{
	return StopWatch_state;
}

static watch_t Watch_getState(void)
{
	return Watch_state;
}

static void StopWatch_incCounter(void)
{
	counter_sw ++;
}

static void Watch_incCounter(void)
{

	if (watch_min == 60)
	{
		watch_min = 0;
	} else {
		if (watch_sec == 60)
		{
			watch_min ++;
			watch_sec = 0;

		} else watch_sec ++;
		counter_w = (watch_min *100) + watch_sec;
	}
}

static void Watch_timer_incCounter(void)
{

	if (timer_min == 60)
	{
		timer_min = 0;
	} else {
		if (timer_sec == 60)
		{
			timer_min ++;
			timer_sec = 0;

		} else timer_sec ++;
		counter_t = (timer_min *100) + timer_sec;
	}
}

static void Watch_timer_decCounter(void)
{
	if(timer_sec > 0)
	{
		timer_sec --;

	} else
	{
		if (timer_min > 0)
		{
			timer_min --;
			timer_sec = 59;
		} else
		{
			timer_min = 0;
			timer_sec = 0;
			led_timer_stop();
		}
	}
	counter_t = (timer_min *100) + timer_sec;
}

static void Stopwatch_clearCounter(void)
{
	counter_sw = 0;
}

static void Watch_clearCounter(void)
{
	counter_w = 0;
}

static void Stopwatch_setCounter(uint16_t c)
{
	counter_sw = c;
}

static void Watch_setCounter(uint16_t c)
{
	counter_w = c;
}

static uint16_t Stopwatch_getCounter(void)
{
	return counter_sw;
}

static uint16_t Watch_getCounter(void)
{
	return counter_w;
}



void System_init(void)
{
	StopWatch_state = STOP;
	Watch_state = NOMAL;

	led_init(&led_watch, GPIOB, GPIO_PIN_15);
	led_init(&led_stopwatch, GPIOB, GPIO_PIN_14);
	led_init(&led_stop, GPIOB, GPIO_PIN_13);
	led_init(&led_run, GPIOC, GPIO_PIN_4);

	button_init(&btn_mode, GPIOC, GPIO_PIN_0);
	button_init(&btn_1, GPIOC, GPIO_PIN_1);
	button_init(&btn_2, GPIOC, GPIO_PIN_3);


	//HAL_TIM_Base_Start_IT(&htim3);
}

void system_task(void)
{
	switch(system_state)
	{
	case W :
		Watch_task();
		if(button_getState(&btn_mode) == ACT_PUSH)
		{
			system_state = SW;
		}
		break;
	case SW :
		StopWatch_task();
		if(button_getState(&btn_mode) == ACT_PUSH)
		{
			system_state = W;
		}
		break;
	}
}

static void StopWatch_task(void)
{
	led_stopwatch_state();
	switch(StopWatch_state)
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

static void Watch_task(void)
{
	led_watch_state();
	switch(Watch_state)
	{
	case NOMAL :
		Watch_nomal();
		break;
	case TIMER :
		Watch_timer();
		break;
	case UP :
		Watch_up();
		break;
	default :
		Watch_nomal();
		break;
	}
}

static void StopWatch_stop(void)
{

	if(button_getState(&btn_1) == ACT_PUSH)
	{
		StopWatch_setState(RUN);
	}
	else if (button_getState(&btn_2) == ACT_RELEASE)
	{
		StopWatch_setState(CLEAR);
	}

	led_stop_state();
	uint16_t num = Stopwatch_getCounter();
	fnd_set_num(num);
}

static void StopWatch_run(void)
{
	if(button_getState(&btn_1) == ACT_PUSH){
		StopWatch_setState(STOP);
	}

	led_run_state();

	uint32_t curTime = HAL_GetTick();
	if(curTime - prevTime_stopwatch < 99){
		return;
	}
	prevTime_stopwatch = curTime;

	uint16_t num = Stopwatch_getCounter();

	if(num == 9999){
		Stopwatch_clearCounter();
	}
	else {
		StopWatch_incCounter();
	}

	num = Stopwatch_getCounter();
	fnd_set_num(num);
}

static void StopWatch_clear(void)
{
	uint16_t num;
	Stopwatch_clearCounter();
	num = Stopwatch_getCounter();
	led_clear_state();
	fnd_set_num(num);
	StopWatch_setState(STOP);
}

static void Watch_nomal(void)
{
	uint16_t num = 0;
	if(button_getState(&btn_1) == ACT_PUSH)
	{
		Watch_setState(TIMER);
	} else if (button_getState(&btn_2) == ACT_PUSH)
	{
		Watch_setState(UP);
		up_setting_state = NOMAL;
	}

	uint32_t curTime = HAL_GetTick();
	if(curTime - prevTime_watch < 99){
		return;
	}
	prevTime_watch = curTime;

	Watch_incCounter();

	num = Watch_getCounter();
	fnd_set_num(num);
}

static void Watch_timer(void)
{
	uint16_t num = counter_t;
	uint32_t curTime = HAL_GetTick();

	switch(Timer_state)
	{
	case READY :
		fnd_set_num(num);
		if (button_getState(&btn_2) == ACT_PUSH)
		{
			Watch_setState(UP);
			up_setting_state = TIMER;
		}
		else if(button_getState(&btn_1) == ACT_PUSH)
		{
			Watch_setState(NOMAL);
		}
		break;
	case START :
		if(curTime - prevTime_timer < 99){
			return;
		}
		prevTime_timer = curTime;
		Watch_timer_decCounter();
		if (counter_t == 0)
			Timer_state = READY;

		num = counter_t;
		fnd_set_num(num);

		break;
	}
}

static void Watch_up(void)
{
	uint16_t num = 0;
	if(button_getState(&btn_1) == ACT_PUSH)
	{
		if (up_setting_state == NOMAL)
		{
			Watch_setState(NOMAL);
		} else if (up_setting_state == TIMER)
		{
			Watch_setState(TIMER);
			up_setting_state = UP;
		}
	} else if (button_getState(&btn_2) == ACT_PUSH)
	{
		if (up_setting_state == NOMAL)
		{
			Watch_incCounter();
			uint16_t num = counter_w;
			fnd_set_num(num);
		}
		else if (up_setting_state == TIMER)
		{
			Timer_state = START;
			Watch_timer_incCounter();
			num = counter_t;
			fnd_set_num(num);
		}
	}


}

static void led_watch_state(void)
{

	led_on(&led_watch);
	led_off(&led_stopwatch);
	led_off(&led_stop);
	led_off(&led_run);
}

static void led_stopwatch_state(void)
{
	led_off(&led_watch);
	led_on(&led_stopwatch);
	led_off(&led_stop);
	led_off(&led_run);
}

static void led_stop_state(void)
{

	led_off(&led_watch);
	led_off(&led_stopwatch);
	led_on(&led_stop);
	led_off(&led_run);;
}

static void led_run_state(void)
{
	led_off(&led_watch);
	led_off(&led_stopwatch);
	led_off(&led_stop);
	led_on(&led_run);
}


static void led_clear_state(void)
{
	led_off(&led_watch);
	led_off(&led_stopwatch);
	led_off(&led_stop);
	led_off(&led_run);
}

static void led_timer_stop(void)
{
	led_on(&led_watch);
	led_on(&led_stopwatch);
	led_on(&led_stop);
	led_on(&led_run);
}
