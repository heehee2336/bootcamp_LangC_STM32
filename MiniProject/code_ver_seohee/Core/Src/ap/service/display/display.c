/*
 * display.c
 *
 *  Created on: 2026. 11. 6.
 *      Author: kccistc
 */


#include "display.h"
#include "../../../driver/lcd/lcd.h"
#include "../../../driver/joyStick/joyStick.h"
#include "../distance/distance.h"
#include "../temp_humid/temp_humid.h"
#include <stdio.h>

typedef enum {
	STOP = 0,
	MOVING_RISING,
	MOVING_FALLING,
	ARRIVING
} disp_elevator_status_t;

extern I2C_HandleTypeDef hi2c1;

uint16_t *floorData;
disp_elevator_status_t e_state;
static uint16_t prevState;

void disp_init(void)
{
	Lcd_init(&hi2c1);
	char buff[30];
	sprintf(buff, "NOW: %d FLOOR", 1);
	Lcd_writeStringXY(0,0, buff);
	prevState = ARRIVING;
}

void disp_tempHumid(void)
{
	static uint32_t prevTime = 0;
	if (HAL_GetTick() - prevTime < 1000) {
		return;
	}
	prevTime = HAL_GetTick();

	char buff[30];
	dht11_data_t thData;

	TH_getTempHumid(&thData);
	sprintf(buff, "T:%2u.%uC", thData.t_int, thData.t_dec);
	Lcd_writeStringXY(1, 0, buff);
}

void disp_distance(void) {
	char buff[30];
	static uint16_t prevDist = 0;
	uint16_t distance_cm = 0;

	if ((distance_get(&distance_cm) == DISTANCE_OK) && (prevDist != distance_cm)) {
		prevDist = distance_cm;
		sprintf(buff, "%3d cm", prevDist);
		Lcd_writeStringXY(0, 0, buff);
	}
}

void disp_joyStick(void)
{
	cordinate_XY jsData;
	js_getValue(&jsData);

	char str[30];
	sprintf(str, "elevator distance: %2d", jsData.Y_cord);
	Lcd_writeStringXY(0,0,str);
}

void disp_task(uint16_t state, uint16_t currentData, uint16_t targetData)
{

	e_state = state;
	switch(e_state)
	{
	case STOP :
		//		char buff_stop[30];
		//		sprintf(buff_stop, "Select floor!");
		if (prevState != e_state) {
			//			Lcd_writeStringXY(0,0, buff_stop);
			//		sprintf(buff_stop, "Select floor!");
			prevState = STOP;
		}
		break;
	case MOVING_RISING :
	case MOVING_FALLING :
		char buff_moving[30];
		sprintf(buff_moving, "Move %dF >>> %dF", currentData + 1, targetData + 1);
		if (prevState != e_state) {
			Lcd_writeStringXY(0,0, buff_moving);
			prevState = (e_state == MOVING_RISING) ? MOVING_RISING : MOVING_FALLING;
		}
		break;
	case ARRIVING :
		char buff_arriving[30];
		sprintf(buff_arriving, "NOW : %d             ", currentData + 1);
		if (prevState != e_state) {
			Lcd_writeStringXY(0,0, buff_arriving);
			prevState = ARRIVING;
		}
		break;
	default :
		e_state = STOP;
		break;
	}
}







































