/*
 * display.c
 *
 *  Created on: 2026. 10. 28.
 *      Author: kccistc
 */



#include "display.h"
#include "../temp_humid/temp_humid.h"
#include "../distance/distance.h"
#include "../../../driver/lcd/lcd.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;

void display_init(void)
{
	Lcd_init(&hi2c1);
}


void display_task(void)
{
	char buff[30];
	dht11_data_t thData;
	static uint32_t prevTime = 0;

	if(HAL_GetTick() - prevTime < 1000){
		return;
	}

	prevTime = HAL_GetTick();


	sprintf(buff, "Dist: %3lu cm   ", distance_get());
	Lcd_writeStringXY(0, 0, buff);

	TH_getTempHumid(&thData);
	sprintf(buff, "T: %2u.%u H:%2u%%", thData.t_int, thData.t_dec, thData.h_int);
	Lcd_writeStringXY(1, 0, buff);

}
