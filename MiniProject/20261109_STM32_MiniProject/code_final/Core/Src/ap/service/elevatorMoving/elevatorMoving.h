/*
 * StepMotor.h
 *
 *  Created on: 2026. 11. 5.
 *      Author: kccistc
 */

#ifndef SRC_AP_SERVICE_ELEVATORMOVING_ELEVATORMOVING_H_
#define SRC_AP_SERVICE_ELEVATORMOVING_ELEVATORMOVING_H_

#include "stm32f4xx_hal.h"
#include "../../../driver/stepMotor/stepMotor.h"

void elevatorMoving_task(void);
void elevatorMoving_ISR(void);
void elevatorMoving_Run(void);
void elevatorMoving_Stop(void);
void elevatorMoving_setDirCW(void);
void elevatorMoving_setDirCCW(void);

#endif /* SRC_AP_SERVICE_ELEVATORMOVING_ELEVATORMOVING_H_ */
