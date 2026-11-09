/*
 * distance.h
 *
 *  Created on: 2026. 10. 29.
 *      Author: kccistc
 */

#ifndef SRC_AP_SERVICE_DISTANCE_DISTANCE_H_
#define SRC_AP_SERVICE_DISTANCE_DISTANCE_H_

#include "../../../driver/HCSR04/HCSR04.h"
#include "../../../driver/button/button.h"
#include <stdbool.h>
#include <stdio.h>

typedef enum {
	DISTANCE_OK, DISTANCE_NOT_READY, DISTANCE_IN_PROGRESS
} distance_status_t;

// 초음파 거리로 판단한 카 위치
typedef enum {
	ZONE_UNKNOWN = 0,	// 측정값 없음
	ZONE_1F,			// d >= 9.3cm
	ZONE_1F_2F,			// 6.3 ~ 9.2cm
	ZONE_2F,			// 5.6 ~ 6.2cm
	ZONE_2F_3F,			// 3.1 ~ 5.5cm
	ZONE_3F				// d <= 3.0cm
} floor_zone_t;


void dist_init();
void dist_task();
void dist_request(void);
distance_status_t distance_get(uint16_t *distance_cm);
distance_status_t distance_get_mm(uint16_t *distance_mm);
floor_zone_t distance_getFloorZone(void);
const char *distance_zoneToString(floor_zone_t zone);
void dist_ISR();
void disp_tempHumid(void);
void disp_distance(void);

#endif /* SRC_AP_SERVICE_DISTANCE_DISTANCE_H_ */
