/*
 * distance.h
 *
 *  Created on: 2026. 10. 28.
 *      Author: kccistc
 */

#ifndef SRC_AP_SERVICE_DISTANCE_DISTANCE_H_
#define SRC_AP_SERVICE_DISTANCE_DISTANCE_H_

#include "../../../driver/HCSR04/HCSR04.h"

void distance_init(void);
void distance_task(void);
uint32_t distance_getCm(void);


#endif /* SRC_AP_SERVICE_DISTANCE_DISTANCE_H_ */
