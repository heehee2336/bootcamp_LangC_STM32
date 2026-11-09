/*
 * display.c
 *
 *  Created on: 2026. 10. 27.
 *      Author: kccistc
 */

#include "display.h"
#include "../../driver/lcd/lcd.h"
#include "../../driver/DHT11/DHT11.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;

dht11_data_t dht_data;


static void Disp_init() {
	Lcd_init(&hi2c1);
	dht11_init(GPIOC, GPIO_PIN_8);
}

static void Disp_TempHumid(void) {
//	char tempStr[30];
//	sprintf(tempStr, "temp %2u.%2u C", 11,12);
//	Lcd_writeStringXY(0, 0, tempStr);

	if(dht11_read(&dht_data) != DHT11_OK){
		//printf("DHT11_ERROR\n");
		char errorStr[20] = "DHT11_ERROR";
		Lcd_writeStringXY(0, 0, errorStr);
	} else {
		//printf("temp %2u.%2u C humid %2u.%2u %% \n",
		//				dht_data.t_int, dht_data.t_dec,
		//				dht_data.h_int, dht_data.h_dec);
		char tempStr[30];
		sprintf(tempStr, "temp %2u.%2u C", dht_data.t_int, dht_data.t_dec);
		Lcd_writeStringXY(0, 0, tempStr);

		char humidStr[30];
		sprintf(humidStr, "humid %2u.%2u %%", dht_data.h_int, dht_data.h_dec);
		Lcd_writeStringXY(1, 0, humidStr);
	}
}

void Disp_main(void){
	Disp_init();
	Disp_TempHumid();
}
