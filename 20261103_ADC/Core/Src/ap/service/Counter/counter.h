/*
 * counter.h
 *
 *  Created on: 2026. 11. 3.
 *      Author: kccistc
 */

#ifndef SRC_AP_SERVICE_COUNTER_COUNTER_H_
#define SRC_AP_SERVICE_COUNTER_COUNTER_H_

#include <stdint.h>

void count_init(void);
void count_task(void);

typedef struct {
	int count_X;
	int count_Y;
} count_XY;

typedef enum {
	NORMAL = 0,
	X_MODE,
	Y_MODE,
} upmode_state;

#endif /* SRC_AP_SERVICE_COUNTER_COUNTER_H_ */
