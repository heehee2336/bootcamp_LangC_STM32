/*
 * Display.c
 *
 *  Created on: 2026. 10. 20.
 *      Author: rhoblack
 */

#include "Display.h"
#include "../../driver/fnd/fnd.h"
#include "../../driver/led/led.h"
#include "../../driver/lcd/lcd.h"
#include <stdio.h>
#include <string.h>

enum {TIMECLOCK_MODE, STOPWATCH_MODE};
enum {STOPWATCH_RUN, STOPWATCH_STOP};

extern I2C_HandleTypeDef hi2c1;
stopWatchTime_t *pStopWatchTime;
timeClockTime_t *pTimeClockTime;
timeClockDate_t *pTimeClockDate;

led_t ledTimeClock, ledStopWatch;
led_t ledStopWatchRun, ledStopWatchStop;

void Disp_init(void)
{
	led_init(&ledTimeClock, GPIOB, GPIO_PIN_15);
	led_init(&ledStopWatch, GPIOB, GPIO_PIN_14);
	led_init(&ledStopWatchRun, GPIOC, GPIO_PIN_4);
	led_init(&ledStopWatchStop, GPIOB, GPIO_PIN_13);
	Lcd_init(&hi2c1);
}

void disp_setMode(int mode)
{
	led_off(&ledStopWatchRun);
	led_off(&ledStopWatchStop);
	if (mode == TIMECLOCK_MODE){
		led_on(&ledTimeClock);
		led_off(&ledStopWatch);
		//		Lcd_writeNibble(0x30);
		//		Lcd_writeNibble(0x20);
		//			Lcd_writeByte(0x28); // Fucntion set 4bit
		//			Lcd_writeByte(0x08); // display off
		//			Lcd_writeByte(0x01); // display clear
		//			Lcd_writeByte(0x06); // entry mode set
		//			Lcd_writeByte(0x0C); // display on
	}
	else {
		led_off(&ledTimeClock);
		led_on(&ledStopWatch);
		//		Lcd_writeNibble(0x30);
		//		Lcd_writeNibble(0x20);
		//			Lcd_writeByte(0x28); // Fucntion set 4bit
		//			Lcd_writeByte(0x08); // display off
		//			Lcd_writeByte(0x01); // display clear
		//			Lcd_writeByte(0x06); // entry mode set
		//			Lcd_writeByte(0x0C); // display on

	}
}

void disp_setTimeClockTime(timeClockTime_t * pTime)
{
	pTimeClockTime = pTime;
	disp_timeClockTime();

}


void Disp_setStopWatchTime(stopWatchTime_t *pTime)
{
	pStopWatchTime = pTime;
	disp_sw();

}

void disp_sw(void)
{
	uint16_t num;
	num = ((pStopWatchTime->min % 10) * 1000) + (pStopWatchTime->sec * 10) + (pStopWatchTime->msec / 10 % 10);
	fnd_set_num(num);
	char swStr[30];
	sprintf(swStr, "%02d:%02d:%02d:%02d     ", pStopWatchTime->hour, pStopWatchTime->min, pStopWatchTime->sec, pStopWatchTime->msec);
	Lcd_writeStringXY(0, 0, swStr);

	char dateStr[30];
	const char *strDay[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
	if (pTimeClockDate != 0 && pTimeClockDate->day >= 1 && pTimeClockDate->day <= 7) {
		sprintf(dateStr, "20%02d-%02d-%02d [%s]", pTimeClockDate->year, pTimeClockDate->month, pTimeClockDate->date, strDay[pTimeClockDate->day - 1]);
		Lcd_writeStringXY(1, 0, dateStr);
	}
	if (pStopWatchTime->msec < 50) {
		fnd_set_dp(FND_DIGIT_3, FND_DP_ON);
		fnd_set_dp(FND_DIGIT_1, FND_DP_ON);
	}
	else {
		fnd_set_dp(FND_DIGIT_3, FND_DP_OFF);
		fnd_set_dp(FND_DIGIT_1, FND_DP_OFF);
	}


	fnd_set_dp(FND_DIGIT_2, FND_DP_OFF);

}

void disp_timeClockTime(void)
{
	uint16_t num;
	num = (pTimeClockTime->min * 100) + pTimeClockTime->sec;
	fnd_set_num(num);
	char timeStr[30];
	sprintf(timeStr, "%02d:%02d:%02d        ", pTimeClockTime->hour, pTimeClockTime->min, pTimeClockTime->sec);
	Lcd_writeStringXY(0, 0, timeStr);

	char dateStr[30];
	const char *strDay[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
	sprintf(dateStr, "20%02d-%02d-%02d [%s]", pTimeClockDate->year, pTimeClockDate->month, pTimeClockDate->date, strDay[pTimeClockDate->day-1]);
	Lcd_writeStringXY(1, 0, dateStr);


	if (pTimeClockTime->msec < 50) {
		fnd_set_dp(FND_DIGIT_2, FND_DP_ON);
	}
	else {
		fnd_set_dp(FND_DIGIT_2, FND_DP_OFF);
	}
	fnd_set_dp(FND_DIGIT_1, FND_DP_OFF);
	fnd_set_dp(FND_DIGIT_3, FND_DP_OFF);

	//	sprintf(timeStr, "%02d:%02d%02d", pTimeClockTime->hour, pTimeClockTime->min, pTimeClockTime->sec);
	//	Lcd_writeStringXY(1, 0, timeStr);
}


void disp_setStopWatchState(int state)
{
	led_off(&ledTimeClock);
	led_on(&ledStopWatch);

	if (state == STOPWATCH_RUN){
		led_on(&ledStopWatchRun);
		led_off(&ledStopWatchStop);
	}
	else {
		led_off(&ledStopWatchRun);
		led_on(&ledStopWatchStop);
	}

}


void disp_date()
{
	static timeClockDate_t prevDate = {0};

	if(!memcmp(&prevDate, pTimeClockDate, sizeof(timeClockDate_t))){
		return;
	}
	memcpy(&prevDate, pTimeClockDate, sizeof(timeClockDate_t));

}
void Disp_setTimeClockData(timeClockDate_t *pDate)
{
	pTimeClockDate = pDate;
	disp_date();
}

void disp_ISR(void)
{
	fnd_show_number();
}
















