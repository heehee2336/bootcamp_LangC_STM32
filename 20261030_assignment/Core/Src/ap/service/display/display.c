/*
 * display.c
 *
 *  Created on: 2026. 10. 28.
 *      Author: rhoblack
 */


#include "display.h"
#include "../temp_humid/temp_humid.h"
#include "../distance/distance.h"
#include "../StopWatch/StopWatch.h"
#include "../TimeClock/TimeClock.h"
#include "../../../driver/lcd/lcd.h"
#include "../../../driver/fnd/fnd.h"
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

enum {TIMECLOCK_MODE, STOPWATCH_MODE};
enum {STOPWATCH_RUN, STOPWATCH_STOP};

stopWatchTime_t *pStopWatchTime;
timeClockTime_t *pTimeClockTime;
timeClockDate_t *pTimeClockDate;


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_6) {
		dist_ISR();
	}
}

void display_init(void)
{
	Lcd_init(&hi2c1);
}

void disp_tempHumid(void)
{
	TH_init();

	static uint32_t prevTime = 0;
	if (HAL_GetTick() - prevTime < 1000) {
		return;
	}
	prevTime = HAL_GetTick();

	char buff[30];
	dht11_data_t thData;

	TH_getTempHumid(&thData);
	sprintf(buff, "T:%2u.%uC H:%2u%%", thData.t_int, thData.t_dec, thData.h_int);
	Lcd_writeStringXY(1, 0, buff);
}

void disp_distance(void) {
	dist_init();

	char buff[30];
	static uint16_t prevDist = 0;
	uint16_t distance_cm = 0;

	if ((distance_get(&distance_cm) == DISTANCE_OK) && (prevDist != distance_cm)) {
		prevDist = distance_cm;
		sprintf(buff, "%3d cm", prevDist);
		Lcd_writeStringXY(1, 0, buff);
	}
}

void disp_date(void)
{
	TimeClock_init();

	static timeClockDate_t prevDate = {0};

	if (!memcmp(&prevDate, pTimeClockDate, sizeof(timeClockDate_t)))
	{
		return;
	}
	memcpy(&prevDate, pTimeClockDate, sizeof(timeClockDate_t));

	char dateStr[30];
	char *str_day[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
	sprintf(dateStr, "20%02d-%02d-%02d [%s]", pTimeClockDate -> year ,pTimeClockDate -> month ,
			pTimeClockDate -> date, str_day[pTimeClockDate -> day - 1] );
	Lcd_writeStringXY(1,0,dateStr);

}

void disp_sw(void)
{
	StopWatch_init();

	StopWatch_task();

	stopWatch_t *pStopWatchTime = StopWatch_getTimeData();
	disp_setStopWatchTime((stopWatchTime_t *)pStopWatchTime);

	uint16_t num;
	num = ((pStopWatchTime->min % 10) * 1000) + (pStopWatchTime->sec * 10) + (pStopWatchTime->msec / 10 % 10);
//	fnd_set_num(num);

//	if (pStopWatchTime->msec < 50) {
//		fnd_set_dp(FND_DIGIT_3, FND_DP_ON);
//	}
//	else {
//		fnd_set_dp(FND_DIGIT_3, FND_DP_OFF);
//	}
//
//	fnd_set_dp(FND_DIGIT_1, FND_DP_ON);
//	fnd_set_dp(FND_DIGIT_2, FND_DP_OFF);

	char stopwatchStr[30];
	sprintf(stopwatchStr, "%02d:%02d:%02d:%02d", pStopWatchTime -> hour,pStopWatchTime -> min ,pStopWatchTime -> sec, pStopWatchTime -> msec);
	Lcd_writeStringXY(1,0,stopwatchStr);

}

void disp_watch(void)
{
	TimeClock_init();

	TimeClock_task();

	TimeClock_t *pTimeClockTime = TimeClock_getTimeDate();
	disp_setTimeClockTime((timeClockTime_t *)pTimeClockTime);

	char timeStr[30];
	sprintf(timeStr, "%02d:%02d:%02d", pTimeClockTime -> hour,pTimeClockTime -> min ,pTimeClockTime -> sec);
	Lcd_writeStringXY(0,0,timeStr);
}

void disp_setTimeClockTime(timeClockTime_t * pTime)
{
	pTimeClockTime = pTime;
	disp_timeClockTime();

	disp_watch();
}


void disp_setStopWatchTime(stopWatchTime_t *pTime)
{
	pStopWatchTime = pTime;
	disp_sw();
}

void disp_setTimeClockDate(timeClockDate_t *pdate)
{
	pTimeClockDate = pdate;
	disp_date();
}

void disp_timeClockTime(void)
{
	if (pTimeClockTime->msec < 50) {
		fnd_set_dp(FND_DIGIT_2, FND_DP_ON);
	}
	else {
		fnd_set_dp(FND_DIGIT_2, FND_DP_OFF);
	}
	fnd_set_dp(FND_DIGIT_1, FND_DP_OFF);
	fnd_set_dp(FND_DIGIT_3, FND_DP_OFF);


	uint16_t num;
	num = (pTimeClockTime->min * 100) + pTimeClockTime->sec;
	fnd_set_num(num);

}

void disp_fnd_ISR(void)
{
	fnd_show_number();
}

void disp_Time_ISR(void)
{
	TimeClock_ISR();
	StopWatch_ISR();
}


