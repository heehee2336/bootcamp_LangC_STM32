/*
 * elevator.h
 *
 *  Created on: 2026. 11. 5.
 *      Author: kccistc
 */

#ifndef SRC_AP_SERVICE_ELEVATOR_ELEVATOR_H_
#define SRC_AP_SERVICE_ELEVATOR_ELEVATOR_H_

#include <stdint.h>
#include <stdbool.h>

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

// UART 상태 조회용
typedef struct {
	uint8_t mode;				// elevator_mode_t
	uint8_t emergency_state;	// 0 MANUAL, 1 ESCAPE
	uint8_t state;				// elevator_state_t
	uint8_t current_floor;		// FLOOR_1 ~ FLOOR_3 (0 ~ 2)
	uint8_t target_floor;
} elevator_status_t;

// PC 수동 조작 방향
typedef enum {
	MOVE_CMD_STOP = 0,
	MOVE_CMD_UP,
	MOVE_CMD_DOWN
} elevator_move_t;

void Elevator_init(void);
void Elevator_getStatus(elevator_status_t *status);
bool Elevator_requestFloor(uint16_t floor);		// NORMAL & IDLE 일 때만 true
bool Elevator_requestEmergency(void);			// NORMAL 일 때만 true
bool Elevator_requestEscape(void);				// EMERGENCY 수동 조작 중일 때만 true
bool Elevator_requestManualMove(elevator_move_t move);	// EMERGENCY 수동 조작 중일 때만 true
void Elevator_task(void);
void Elevator_ISR(uint16_t floor);

#endif /* SRC_AP_SERVICE_ELEVATOR_ELEVATOR_H_ */
