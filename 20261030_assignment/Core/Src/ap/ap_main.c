/*
 * ap_main.c
 *
 *  Created on: 2026. 10. 30.
 *      Author: kccistc
 */

#include "ap_main.h"
#include "service/sound/sound.h"
#include "../driver/button/button.h"
#include "../driver/lcd/lcd.h"
#include <stdio.h>
#include "service/display/display.h"
#include "service/sound/sound.h"
#include "../common/queue/que_buffer.h"
#include "service/distance/distance.h"
#include "service/temp_humid/temp_humid.h"

#define BTN_EVENT_Q_SIZE	30

typedef enum {
	BTN_DATE = 0,
	BTN_TEMPHUM,
	BTN_DIST,
	BTN_SW,
	BTN_NORMAL
} btn_event_t;

typedef enum {
	STATE_DATE = 0,
	STATE_TEMPHUM,
	STATE_DIST,
	STATE_SW
} state_t;

extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

enum {TIME_CLOCK_MODE, STOP_WATCH_MODE};

button_t btnMode;
btn_event_t btnEvent[BTN_EVENT_Q_SIZE];
que_buf_t que_btnEvent;
state_t state;
button_t btn_mode;
uint32_t prevTime = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM3) {
		disp_fnd_ISR();
	}
	else if (htim->Instance == TIM4) {
		disp_Time_ISR();
	}
}

int ap_main(void)
{

	btn_event_t btnEvent = BTN_NORMAL;

	display_init();

	button_init(&btn_mode, GPIOC, GPIO_PIN_0);
	que_buf_init(&que_btnEvent, &btnEvent, sizeof(btn_event_t), BTN_EVENT_Q_SIZE);
	sound_init();

	sound_power();

	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start_IT(&htim4);

	while(1) {
		switch(state)
		{
		case STATE_DATE:
			if (button_getState(&btn_mode) == ACT_RELEASE)
			{
				btnEvent = BTN_DATE;
				que_put(&que_btnEvent, &btnEvent);
			}
			que_get(&que_btnEvent, &btnEvent);

			if (btnEvent == BTN_DATE) {
				sound_date();
				state = STATE_TEMPHUM;
			}
			disp_date();
			break;
		case STATE_TEMPHUM:
			if (button_getState(&btn_mode) == ACT_RELEASE)
			{
				btnEvent = BTN_TEMPHUM;
				que_put(&que_btnEvent, &btnEvent);
			}
			que_get(&que_btnEvent, &btnEvent);

			if (btnEvent == BTN_TEMPHUM) {
				sound_hcsr04();
				state = STATE_DIST;
			}
			disp_tempHumid();
			break;
		case STATE_DIST:
			if (button_getState(&btn_mode) == ACT_RELEASE)
			{
				btnEvent = BTN_DIST;
				que_put(&que_btnEvent, &btnEvent);
			}
			que_get(&que_btnEvent, &btnEvent);

			if (btnEvent == BTN_DIST) {
				sound_dht11();
				state = STATE_SW;
			}
			disp_distance();
			break;
		case STATE_SW:
			if (button_getState(&btn_mode) == ACT_RELEASE)
			{
				btnEvent = BTN_SW;
				que_put(&que_btnEvent, &btnEvent);
			}
			que_get(&que_btnEvent, &btnEvent);

			if (btnEvent == BTN_SW) {
				sound_SW();
				state = STATE_DATE;
			}
			disp_sw();
			break;
		default :
			state = STATE_DATE;
			break;
		}
	}
	return 0;
}




