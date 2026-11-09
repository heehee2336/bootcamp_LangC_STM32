/*
 * display.c
 *
 *  Created on: 2026. 10. 28.
 *      Author: rhoblack
 */


#include "display.h"
#include "../temp_humid/temp_humid.h"
#include "../../../driver/lcd/lcd.h"
#include "../distance/distance.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;

void display_init(void)
{
	char buff[30];
	Lcd_init(&hi2c1);
	sprintf(buff, "Hello STM32!");
	Lcd_writeStringXY(0, 0, buff);
}

void display_THTask(void){
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

void display_distanceTask(void) {

	uint16_t dist_cm;

	if(distance_get(&dist_cm) == DISTANCE_OK) {
		printf("distance : %3u cm\n", dist_cm);
	}
}

void display_task(void)
{
	display_THTask();
	display_distanceTask();
}


