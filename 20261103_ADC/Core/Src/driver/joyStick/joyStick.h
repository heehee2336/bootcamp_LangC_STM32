/*
 * joyStick.h
 *
 *  Created on: 2026. 11. 3.
 *      Author: kccistc
 */

#ifndef SRC_DRIVER_JOYSTICK_JOYSTICK_H_
#define SRC_DRIVER_JOYSTICK_JOYSTICK_H_
#include <stdint.h>

typedef struct {
	uint16_t X_cord;
	uint16_t Y_cord;
} cordinate_XY;

void js_init(void);
void js_getValue(cordinate_XY *value);

#endif /* SRC_DRIVER_JOYSTICK_JOYSTICK_H_ */
