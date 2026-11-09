/*
 * ruler.h
 *
 *  Created on: 2026. 11. 2.
 *      Author: kccistc
 */

#ifndef SRC_AP_MAIN_SERVICE_RULER_RULER_H_
#define SRC_AP_MAIN_SERVICE_RULER_RULER_H_
#include <stdint.h>


void ruler_init(void);
void ruler_disp(uint16_t distance);
void ruler_task(void);
void ruler_ISR();

#endif /* SRC_AP_MAIN_SERVICE_RULER_RULER_H_ */
