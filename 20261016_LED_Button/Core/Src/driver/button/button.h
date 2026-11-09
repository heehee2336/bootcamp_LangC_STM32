/*
 * button.h
 *
 *  Created on: 2026. 10. 16.
 *      Author: kccistc
 */

#ifndef SRC_DRIVER_BUTTON_BUTTON_H_
#define SRC_DRIVER_BUTTON_BUTTON_H_

#include "stm32f4xx_hal.h"

typedef enum {
	NO_ACT,			//0
	ACT_PUSH,		//1
	ACT_RELEASE		//2
} button_state_t;

typedef struct {
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_PIN;
	int prevState;
} button_t;

enum {PUSHED = 0, RELEASED};

button_state_t button_getState(button_t *btn);

#endif /* SRC_DRIVER_BUTTON_BUTTON_H_ */
