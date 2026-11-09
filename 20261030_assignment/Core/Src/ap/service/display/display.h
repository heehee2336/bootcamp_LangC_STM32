/*
 * display.h
 *
 *  Created on: 2026. 10. 28.
 *      Author: rhoblack
 */

#ifndef SRC_AP_SERVICE_DISPLAY_DISPLAY_H_
#define SRC_AP_SERVICE_DISPLAY_DISPLAY_H_

#include <stdint.h>

typedef struct {
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t msec;
}stopWatchTime_t;

typedef struct {
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t msec;
}timeClockTime_t;

typedef struct {
	uint8_t day;
	uint8_t date;
	uint8_t month;
	uint8_t year;
}timeClockDate_t;

void display_init(void);
void display_task(void);
void disp_date();
void disp_sw(void);
void disp_watch(void);
void disp_distance(void);
void disp_tempHumid(void);

void disp_setTimeClockTime(timeClockTime_t * pTime);
void disp_timeClockTime(void);
void disp_setStopWatchTime(stopWatchTime_t *pTime);
void disp_setTimeClockDate(timeClockDate_t *pdate);

void disp_fnd_ISR(void);
void disp_Time_ISR(void);

#endif /* SRC_AP_SERVICE_DISPLAY_DISPLAY_H_ */
