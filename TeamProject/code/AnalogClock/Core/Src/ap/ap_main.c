/*
 * ap_main.c
 * Application mode dispatcher.
 */

#include "ap_main.h"
#include "ClockNeedle/ClockNeedle.h"
#include "StopWatch/StopWatch.h"
#include "TimeClock/TimeClock.h"
#include "Display/Display.h"
#include "../driver/button/button.h"
#include "stm32f4xx_hal.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

enum { TIME_CLOCK_MODE, STOP_WATCH_MODE };

typedef struct {
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
} timeSnapshot_t;

static button_t btnMode;
static int modeState = TIME_CLOCK_MODE;
static uint8_t pendingTimeChangeEvents;

static void ap_trackTimeChanges(int mode, const timeSnapshot_t *time,
		int enabled)
{
	static int previousMode = -1;
	static uint8_t previousHour;
	static uint8_t previousMin;
	static uint8_t previousSec;

	if (mode != previousMode || !enabled) {
		previousMode = mode;
		previousHour = time->hour;
		previousMin = time->min;
		previousSec = time->sec;
		return;
	}

	if (time->sec != previousSec) {
		pendingTimeChangeEvents |= AP_TIME_CHANGED_SEC;
	}
	if (time->min != previousMin) {
		pendingTimeChangeEvents |= AP_TIME_CHANGED_MIN;
	}
	if (time->hour != previousHour) {
		pendingTimeChangeEvents |= AP_TIME_CHANGED_HOUR;
	}

	previousHour = time->hour;
	previousMin = time->min;
	previousSec = time->sec;
}

uint8_t ap_takeTimeChangeEvents(void)
{
	uint8_t events = pendingTimeChangeEvents;
	pendingTimeChangeEvents = 0;
	return events;
}

static void ap_timeClockMode(void)
{
	if (button_getState(&btnMode) == ACT_RELEASE) {
		modeState = STOP_WATCH_MODE;
	}
//	TimeClock_task();

	TimeClock_t *pTime = TimeClock_getTimeData();
	timeSnapshot_t time = {
		.hour = pTime->hour,
		.min = pTime->min,
		.sec = pTime->sec
	};
	ap_trackTimeChanges(TIME_CLOCK_MODE, &time, 1);
	TimeDate_t *pDate = TimeClock_getDateData();
	Disp_setTimeClockData((timeClockDate_t *)pDate);
	Disp_setTimeClockTime((timeClockTime_t *)pTime);
	Disp_setMode(modeState);
}

static void ap_stopWatchMode(void)
{
	if (button_getState(&btnMode) == ACT_RELEASE) {
		modeState = TIME_CLOCK_MODE;
	}
//	TimeClock_task();
	StopWatch_task();

	TimeDate_t *pDate = TimeClock_getDateData();
	stopWatch_t *pTime = StopWatch_getTimeData();
	timeSnapshot_t time = {
		.hour = pTime->hour,
		.min = pTime->min,
		.sec = pTime->sec
	};
	ap_trackTimeChanges(STOP_WATCH_MODE, &time,
			StopWatch_getState() == RUN);
	Disp_setTimeClockData((timeClockDate_t *)pDate);
	Disp_setStopWatchTime((stopWatchTime_t *)pTime);
	Disp_setMode(modeState);
	Disp_setStopWatchState(StopWatch_getState());
}

void ap_init(void)
{
	button_init(&btnMode, GPIOC, GPIO_PIN_0);

	StopWatch_init();
	TimeClock_init();
	ClockNeedle_init();
	Disp_init();

	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start_IT(&htim4);
}

void ap_main(void)
{
	TimeClock_task();
	switch (modeState) {
	case TIME_CLOCK_MODE:
		ap_timeClockMode();
		break;
	case STOP_WATCH_MODE:
		ap_stopWatchMode();
		break;
	default:
		modeState = TIME_CLOCK_MODE;
		break;
	}

	uint8_t timeChangeEvents = ap_takeTimeChangeEvents();
	if (timeChangeEvents & AP_TIME_CHANGED_SEC) {
		ClockNeedle_SecStep();
	}

	if (timeChangeEvents & AP_TIME_CHANGED_MIN) {
		ClockNeedle_MinStep();
	}

	if (timeChangeEvents & AP_TIME_CHANGED_HOUR) {
		ClockNeedle_HourStep();
	}
}
