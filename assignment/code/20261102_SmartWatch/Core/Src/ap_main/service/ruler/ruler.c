/*
 * ruler.c
 *
 *  Created on: 2026. 11. 2.
 *      Author: kccistc
 */
#include "ruler.h"
#include "../../../driver/HCSR04/HCSR04.h"
#include "../../../driver/button/button.h"
#include "../../../driver/lcd/lcd.h"
#include <stdbool.h>
#include <stdio.h>

#define	TRIG_PORT 		GPIOC
#define	TRIG_PIN 		GPIO_PIN_5
#define	ECHO_PORT 		GPIOC
#define	ECHO_PIN 		GPIO_PIN_6
#define RULER_TIMER 	&htim10
#define BTN_RULER_PORT 	GPIOC
#define BTN_RULER_PIN 	GPIO_PIN_1

extern TIM_HandleTypeDef htim10;

static uint32_t echo_us = 0;
static bool start_flag = false;
static hcsr04_status_t hcsr04_status;
static button_t btn_ruler;

void ruler_init(void) {
	HCSR04_init(TRIG_PORT, TRIG_PIN, ECHO_PORT, ECHO_PIN, RULER_TIMER);
	button_init(&btn_ruler, BTN_RULER_PORT, BTN_RULER_PIN);
}

static void ruler_request(void) {
	hcsr04_status_t state_start = HCSR04_start();
	if (state_start != HCSR04_ERR_BUSY) {
		start_flag = true;
	}
}


void ruler_disp(uint16_t distance) {
	char buff[30];
	sprintf(buff, "distance: %4dcm      ", distance);
	Lcd_writeStringXY(1, 0, buff);
}



void ruler_task(void) {
	if (button_getState(&btn_ruler) == ACT_RELEASE) {
		ruler_request();
	}

	hcsr04_status = HCSR04_getResult(&echo_us);

	if (start_flag != true) {
		return ;
	}
	if (hcsr04_status != HCSR04_OK) {
		return ;
	}
	start_flag = false;
	uint16_t distance_cm = echo_us * 17 / 1000;
	ruler_disp(distance_cm);
}


void ruler_ISR() {
	HCSR04_echoISR();
}
















