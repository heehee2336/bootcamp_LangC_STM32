/*
 * ap_main.c
 *
 *  Created on: 2026. 10. 27.
 *      Author: kccistc
 */

#include "ap_main.h"
#include "../common/delay/delay.h"
#include "../driver/DHT11/DHT11.h"
#include "../driver/lcd/lcd.h"
#include "display/display.h"

void ap_init(void) {
	Disp_main();
	HAL_Delay(2000);
}

void ap_main(void) {
	ap_init();
}
