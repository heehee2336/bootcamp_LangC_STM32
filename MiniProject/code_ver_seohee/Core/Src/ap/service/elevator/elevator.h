/*
 * elevator.h
 *
 *  Created on: 2026. 11. 5.
 *      Author: kccistc
 */

#ifndef SRC_AP_SERVICE_ELEVATOR_ELEVATOR_H_
#define SRC_AP_SERVICE_ELEVATOR_ELEVATOR_H_

#include <stdint.h>

#define FLOOR_1		0
#define FLOOR_2		1
#define FLOOR_3		2


typedef enum {
	IDLE = 0,
	RISING,
	FALLING,
	ARRIVE
} elevator_state_t;

typedef enum {
	NORMAL,
	EMERGENCY
} elevator_mode_t;

void Elevator_init(void);
void Elevator_task(void);
void Elevator_ISR(uint16_t floor);

#endif /* SRC_AP_SERVICE_ELEVATOR_ELEVATOR_H_ */
