/*
 * display.h
 *
 *  Created on: 2026. 11. 6.
 *      Author: kccistc
 */

#ifndef SRC_AP_SERVICE_DISPLAY_DISPLAY_H_
#define SRC_AP_SERVICE_DISPLAY_DISPLAY_H_

#include <stdint.h>

void disp_init(void);
void disp_task(uint16_t state, uint16_t currentData, uint16_t targetData);


#endif /* SRC_AP_SERVICE_DISPLAY_DISPLAY_H_ */
