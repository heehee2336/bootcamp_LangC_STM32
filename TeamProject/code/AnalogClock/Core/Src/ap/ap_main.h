/*
 * ap_main.h
 *
 *  Created on: 2026. 10. 16.
 *      Author: rhoblack
 */

#ifndef SRC_AP_AP_MAIN_H_
#define SRC_AP_AP_MAIN_H_

#include <stdint.h>

enum {
	AP_TIME_CHANGED_SEC  = (1U << 0),
	AP_TIME_CHANGED_MIN  = (1U << 1),
	AP_TIME_CHANGED_HOUR = (1U << 2)
};

void ap_init(void);
void ap_main(void);
uint8_t ap_takeTimeChangeEvents(void);

#endif /* SRC_AP_AP_MAIN_H_ */
