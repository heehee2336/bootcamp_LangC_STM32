/*
 * StopWatch.h
 *
 *  Created on: 2026. 10. 16.
 *      Author: rhoblack
 */

#ifndef SRC_AP_STOPWATCH_STOPWATCH_H_
#define SRC_AP_STOPWATCH_STOPWATCH_H_
#include "stm32f4xx_hal.h"

typedef struct {
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t msec;
} StopWatchData_t;


void StopWatch_init(void);
void StopWatch_ISR(void);
void StopWatch_task(void);

#endif /* SRC_AP_STOPWATCH_STOPWATCH_H_ */
