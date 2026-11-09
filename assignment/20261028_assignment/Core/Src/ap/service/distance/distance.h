/*
 * distance.h
 *
 *  Created on: 2026. 10. 29.
 *      Author: kccistc
 */

#ifndef SRC_AP_SERVICE_DISTANCE_DISTANCE_H_
#define SRC_AP_SERVICE_DISTANCE_DISTANCE_H_
#include <stdint.h>

void distance_init(void);
void distance_task(void);
uint32_t distance_get(void);

#endif /* SRC_AP_SERVICE_DISTANCE_DISTANCE_H_ */
