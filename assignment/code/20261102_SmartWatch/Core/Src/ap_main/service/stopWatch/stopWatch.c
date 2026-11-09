/*
 * stopWatch.c
 *
 *  Created on: 2026. 11. 2.
 *      Author: kccistc
 */


#include "stopWatch.h"
#include "../../../driver/lcd/lcd.h"
#include "../../../driver/button/button.h"
#include <stdbool.h>
#include <stdio.h>

#define BTN_RUNSTOP_PORT GPIOC
#define BTN_RUNSTOP_PIN GPIO_PIN_1
#define BTN_CLEAR_PORT GPIOC
#define BTN_CLEAR_PIN GPIO_PIN_3


extern TIM_HandleTypeDef htim3;

typedef struct {
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t msec;
} stopWatch_t;

typedef enum {STOP, RUN, CLEAR} stopWatch_state_t;


static stopWatch_t stopWatch_data = {0};
static stopWatch_state_t stopWatch_state = STOP;

static bool stopWatch_runFlag = false;

static button_t btnRunStop;
static button_t btnClear;

static void stopWatch_clear(void);


static void stopWatch_incMsec(void) {
	stopWatch_data.msec++;
	if (stopWatch_data.msec < 100) return;
	stopWatch_data.msec = 0;
	stopWatch_data.sec++;
	if (stopWatch_data.sec < 60) return;
	stopWatch_data.sec = 0;
	stopWatch_data.min++;
	if (stopWatch_data.min < 60) return;
	stopWatch_data.min = 0;
	stopWatch_data.hour++;
	if (stopWatch_data.hour < 24) return;
	stopWatch_data.hour = 0;
}


//static void StopWatch_incMSec_professor_ver(void) {
//	if (stopWatch_time.msec < 100 - 1) {
//		stopWatch_time.msec++;
//		return;
//	}
//	stopWatch_time.msec = 0;
//	if (stopWatch_time.sec < 60 - 1) {
//		stopWatch_time.sec++;
//		return;
//	}
//	stopWatch_time.sec = 0;
//	if (stopWatch_time.min < 60 - 1) {
//		stopWatch_time.min++;
//		return;
//	}
//	stopWatch_time.min = 0;
//	if (stopWatch_time.hour < 24 - 1) {
//		stopWatch_time.hour++;
//		return;
//	}
//	stopWatch_time.hour = 0;
//}

void stopWatch_init(void) {
	button_init(&btnRunStop, BTN_RUNSTOP_PORT, BTN_RUNSTOP_PIN);
	button_init(&btnClear, BTN_CLEAR_PORT, BTN_CLEAR_PIN);
	HAL_TIM_Base_Start_IT(&htim3);
	stopWatch_clear();
}


void stopWatch_disp(void) {
	char buff[30];
	sprintf(buff, "[SW] %02d:%02d:%02d:%02d", stopWatch_data.hour, stopWatch_data.min,
			stopWatch_data.sec, stopWatch_data.msec);
	Lcd_writeStringXY(1, 0, buff);
}



static void stopWatch_stop(void) {
//	static uint8_t prevMsec = 0;
//	static uint8_t prevSec = 0;
//	if ((stopWatch_data.msec == prevMsec) && (stopWatch_data.sec == prevSec)) return;
//	prevMsec = stopWatch_data.msec;
//	prevSec = stopWatch_data.sec;
//	stopWatch_disp();
}

static void stopWatch_run(void) {
	if (stopWatch_runFlag != true) {
		return;
	}
	stopWatch_runFlag = false;
	stopWatch_disp();
}

static void stopWatch_clear(void) {
	stopWatch_data.hour = 0;
	stopWatch_data.min = 0;
	stopWatch_data.sec = 0;
	stopWatch_data.msec = 0;
	stopWatch_disp();
}



void stopWatch_task(void) {
	switch(stopWatch_state) {
	case STOP:
		if (button_getState(&btnRunStop) == ACT_PUSH) {
			stopWatch_state = RUN;
		}
		if (button_getState(&btnClear) == ACT_RELEASE) {
			stopWatch_state = CLEAR;
		}
		stopWatch_stop();
		break;
	case RUN:
		if (button_getState(&btnRunStop) == ACT_PUSH) {
			stopWatch_state = STOP;
		}
		stopWatch_run();
		break;
	case CLEAR:
		stopWatch_state = STOP;
		stopWatch_clear();
		break;
	default:
		break;
	}
}

void stopWatch_ISR(void) {
	if (stopWatch_state == RUN) {
		stopWatch_runFlag = true;
		stopWatch_incMsec();
	}
}






















