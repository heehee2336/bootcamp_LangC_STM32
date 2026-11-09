/*
 * display.c
 *
 *  Created on: 2026. 11. 6.
 *      Author: kccistc
 */


#include "display.h"
#include "../../../driver/lcd/lcd.h"
#include "../../../driver/joyStick/joyStick.h"
#include "../distance/distance.h"
#include "../temp_humid/temp_humid.h"
#include <stdio.h>
#include <stdbool.h>

typedef enum {
	STOP = 0,
	MOVING_RISING,
	MOVING_FALLING,
	ARRIVING
} disp_elevator_status_t;

extern I2C_HandleTypeDef hi2c1;

uint16_t *floorData;
disp_elevator_status_t e_state;
static uint16_t prevState;

// NORMAL 화면 갱신용
static bool normal_redraw = true;		// true면 다음 disp_task()에서 두 줄 모두 다시 그림
static uint16_t prevFloor = 0xFFFF;
static uint8_t prevTempInt = 0xFF;
static uint8_t prevTempDec = 0xFF;

void disp_init(void)
{
	Lcd_init(&hi2c1);
	char buff[30];
	sprintf(buff, "NOW: %d FLOOR", 1);
	Lcd_writeStringXY(0,0, buff);
	prevState = ARRIVING;
}

void disp_tempHumid(void)
{
	static uint32_t prevTime = 0;
	if (HAL_GetTick() - prevTime < 1000) {
		return;
	}
	prevTime = HAL_GetTick();

	char buff[30];
	dht11_data_t thData;

	TH_getTempHumid(&thData);
	sprintf(buff, "T:%2u.%uC", thData.t_int, thData.t_dec);
	Lcd_writeStringXY(1, 0, buff);
}

// LCD 첫 줄에 "DIST: 12.3 cm" 형식으로 0.1cm 단위 표시 (200ms 마다 갱신)
void disp_distance(void) {
	static uint32_t prevTime = 0;
	if (HAL_GetTick() - prevTime < 200) {
		return;
	}
	prevTime = HAL_GetTick();

	char buff[30];
	uint16_t distance_mm = 0;

	if (distance_get_mm(&distance_mm) == DISTANCE_OK) {
		sprintf(buff, "DIST:%3u.%u cm   ", distance_mm / 10, distance_mm % 10);
	} else {
		sprintf(buff, "DIST: --- cm    ");	// 아직 측정값 없음 / 에코 없음
	}
	Lcd_writeStringXY(0, 0, buff);
}

// EMERGENCY 화면
//  1번째 줄: "!!!EMERGENCY!!!" 0.5초마다 깜빡임 (LCD 한 줄이 16칸이라 공백을 줄임)
//  2번째 줄: "FLOOR:1-2 7.4cm"  현재 층(또는 구간) + 거리
void disp_emergency(void)
{
	static uint32_t blinkTime = 0;
	static bool blinkOn = false;
	static uint32_t infoTime = 0;

	uint32_t now = HAL_GetTick();

	if (now - blinkTime >= 500) {
		blinkTime = now;
		blinkOn = !blinkOn;
		Lcd_writeStringXY(0, 0, blinkOn ? "!!!EMERGENCY!!! " : "                ");
	}

	if (now - infoTime >= 200) {
		infoTime = now;
		char buff[30];
		uint16_t mm = 0;
		const char *zone = distance_zoneToString(distance_getFloorZone());

		if (distance_get_mm(&mm) == DISTANCE_OK) {
			sprintf(buff, "FLOOR:%s%2u.%ucm ", zone, mm / 10, mm % 10);
		} else {
			sprintf(buff, "FLOOR:--- --.-cm");
		}
		Lcd_writeStringXY(1, 0, buff);
	}
}

// EMERGENCY -> NORMAL 로 돌아갈 때 NORMAL 화면을 다시 그리도록 표시
void disp_exitEmergency(uint16_t currentFloor)
{
	(void)currentFloor;
	normal_redraw = true;		// NORMAL 화면 두 줄을 다음 disp_task()에서 다시 그림
	prevState = ARRIVING;
}

// NORMAL -> EMERGENCY 로 갈 때 호출: 나중에 NORMAL로 돌아오면 다시 그리도록 표시
void disp_enterEmergency(void)
{
	normal_redraw = true;
}

void disp_joyStick(void)
{
	cordinate_XY jsData;
	js_getValue(&jsData);

	char str[30];
	sprintf(str, "elevator distance: %2d", jsData.Y_cord);
	Lcd_writeStringXY(0,0,str);
}

// NORMAL 화면
//  (0,0) "NOW : nF"       현재 층
//  (1,0) "TEMP: 24.5C"    온도
// 값이 바뀌었을 때만 LCD에 쓴다 (I2C 전송 줄이기)
void disp_task(uint16_t state, uint16_t currentData, uint16_t targetData)
{
	(void)state;
	(void)targetData;

	char buff[30];
	dht11_data_t thData;
	TH_getTempHumid(&thData);

	if (normal_redraw || currentData != prevFloor) {
		sprintf(buff, "NOW : %dF        ", currentData + 1);
		Lcd_writeStringXY(0, 0, buff);
		prevFloor = currentData;
	}

	if (normal_redraw || thData.t_int != prevTempInt || thData.t_dec != prevTempDec) {
		sprintf(buff, "TEMP: %2d.%dC     ", thData.t_int, thData.t_dec);
		Lcd_writeStringXY(1, 0, buff);
		prevTempInt = thData.t_int;
		prevTempDec = thData.t_dec;
	}

	normal_redraw = false;
}
