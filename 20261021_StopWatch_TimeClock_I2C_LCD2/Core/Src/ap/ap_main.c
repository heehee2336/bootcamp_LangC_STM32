/*
 * ap_main.c
 *
 *  Created on: 2026. 10. 16.
 *      Author: kccistc
 */


#include "ap_main.h"
#include "StopWatch/StopWatch.h"
#include "Display/Display.h"
#include "TimeClock/TimeClock.h"
#include "../driver/fnd/fnd.h"
#include "../driver/button/button.h"
#include "../driver/led/led.h"
#include "../driver/lcd/lcd.h"
#include <stdio.h>

enum{
	TIME_CLOCK_MODE, STOP_WATCH_MODE
};

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

button_t btnMode;
int modeState = TIME_CLOCK_MODE;
char time_buff[30];
char stopwatch_buff[30];
char calendar_buff[30];


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	/* Prevent unused argument(s) compilation warning */

	if(htim->Instance == TIM3){
		//fnd_show_number();
		disp_ISR();

	} else if (htim->Instance == TIM4){
		// timeclock코드
		TimeClock_ISR();

		StopWatch_ISR();

	}

	/* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
	 */
}

void ap_init()
{
	StopWatch_init();
	TimeClock_init();
	Disp_init();
	button_init(&btnMode, GPIOC, GPIO_PIN_0);


	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start_IT(&htim4);
}

void ap_timeClockMode(void)
{
	if(button_getState(&btnMode) == ACT_RELEASE){
		modeState = STOP_WATCH_MODE;
	}

	//TimeClock_task();

	TimeClock_t *pTimeClockTime = TimeClock_getTimeData();

	disp_setTimeClockTime((timeClockTime_t *)pTimeClockTime);
	disp_setMode(modeState);
}

void ap_stopWatchMode(void)
{
	if(button_getState(&btnMode) == ACT_RELEASE){
		modeState = TIME_CLOCK_MODE;
	}
	StopWatch_task();

	stopWatch_t *pStopWatchTime = StopWatch_getTimeData();
	Disp_setStopWatchTime((stopWatchTime_t *)pStopWatchTime);
	disp_setMode(modeState);


	stopWatch_state_t stopWatchState = StopWatch_getState();
	disp_setStopWatchState(stopWatchState);
}


uint32_t counter = 0;
char buff[100];
int ap_main(void)
{

	ap_init();
	while(1){

		TimeClock_t *pTimeClockTime = TimeClock_getTimeData();
		stopWatch_t *pStopWatchTime = StopWatch_getTimeData();

		switch(modeState)
		{
		case TIME_CLOCK_MODE:
			ap_timeClockMode();
			sprintf(time_buff, "Time : %d, %d, %d ,%d", pTimeClockTime->hour,pTimeClockTime->min,
					pTimeClockTime->sec,pTimeClockTime->msec);
			sprintf(calendar_buff, "Time : %d, %d, %d ,%d", pTimeClockTime->hour,pTimeClockTime->min,
					pTimeClockTime->sec,pTimeClockTime->msec);

			Lcd_writeByte(0);
			Lcd_writeStringXY(0,0,time_buff);
			Lcd_writeStringXY(1,0,calendar_buff);
			break;
		case STOP_WATCH_MODE:
			ap_stopWatchMode();
			sprintf(time_buff, "Time : %d, %d, %d ,%d", pTimeClockTime->hour,pTimeClockTime->min,
					pTimeClockTime->sec,pTimeClockTime->msec);
			sprintf(stopwatch_buff, "SW : %d, %d, %d ,%d", pStopWatchTime->hour,pStopWatchTime->min,
					pStopWatchTime->sec,pStopWatchTime->msec);

			Lcd_writeByte(0);
			Lcd_writeStringXY(0,0,time_buff);
			Lcd_writeStringXY(1,0,stopwatch_buff);
			break;
		default:
			break;
		}

		//fnd_show_number();

	}
	return 0;
}
