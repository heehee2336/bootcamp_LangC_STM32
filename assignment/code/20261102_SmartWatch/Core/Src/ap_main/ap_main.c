/*
 * ap_main.c
 *
 *  Created on: 2026. 11. 2.
 *      Author: kccistc
 */

#include "ap_main.h"
#include "service/timeClock/timeClock.h"
#include "service/tempHumid/tempHumid.h"
#include "service/ruler/ruler.h"
#include "service/stopWatch/stopWatch.h"
#include "service/que/que.h"
#include "service/inputEvent/inputEvent.h"

#include "stm32f4xx_hal.h"
#include <stdbool.h>

extern TIM_HandleTypeDef htim3;


typedef enum {
	S_CALENDAR = 0,
	S_TEMP_HUMID,
	S_RULER,
	S_STOPWATCH
} smartWatch_state_t;


smartWatch_state_t smartWatch_state = S_CALENDAR;





void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM3) {
		stopWatch_ISR();
	}
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_6) {
		ruler_ISR();
	}
}





void ap_init(void) {
	timeClock_init();
	tempHumid_init();
	ruler_init();
	stopWatch_init();
	que_init();
	inputEvent_init();
}

int ap_main(void) {
	ap_init();


	event_btn_act_t btn_mode = ACT_PUSHED;
	while(1) {
		timeClock_task();
		inputEvent_task();
		switch(smartWatch_state) {
		case S_CALENDAR:
			if (que_isEmptyMode() == false) {
				que_getMode(&btn_mode);
				if (btn_mode == ACT_RELEASED) {
					smartWatch_state = S_TEMP_HUMID;
				}
			}
			timeClock_calendar_task();
			break;
		case S_TEMP_HUMID:
			if (que_isEmptyMode() == false) {
				que_getMode(&btn_mode);
				if (btn_mode == ACT_RELEASED) {
					ruler_disp(0);
					smartWatch_state = S_RULER;
				}
			}
			tempHumid_task();
			break;
		case S_RULER:
			if (que_isEmptyMode() == false) {
				que_getMode(&btn_mode);
				if (btn_mode == ACT_RELEASED) {
					stopWatch_disp();
					smartWatch_state = S_STOPWATCH;
				}
			}
			ruler_task();
			break;
		case S_STOPWATCH:
			if (que_isEmptyMode() == false) {
				que_getMode(&btn_mode);
				if (btn_mode == ACT_RELEASED) {
					smartWatch_state = S_CALENDAR;
					timeClock_setDateFirstFlag();
				}
			}
			stopWatch_task();
			break;
		default:
			break;
		}
	}

	return 0;
}







