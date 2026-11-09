/*
 * distance.c
 *
 *  Created on: 2026. 10. 29.
 *      Author: kccistc
 *
 *  dist_task()를 메인 루프에서 계속 부르면 DIST_PERIOD_MS 마다 스스로 측정을 반복하고,
 *  마지막 정상 측정값을 저장해 둔다.
 *  distance_get() / distance_get_mm()은 그 저장값을 읽기만 하므로 여러 곳에서 불러도 된다.
 */
#include "distance.h"

#define DIST_PERIOD_MS		100		// 측정 주기 (HC-SR04 권장 최소 60ms 이상)
#define DIST_MAX_MM			4000	// HC-SR04 측정 한계 (4m), 이보다 크면 무효값으로 처리
#define DIST_FILTER_SIZE	5		// 중앙값 필터 크기 (최근 5개)
#define DIST_MAX_FAIL		3		// 연속 실패가 이 횟수가 되면 값을 무효 처리

// 층 판단 경계 (mm)  - 실측: 1F 95mm, 2F 58mm, 3F 28mm
#define ZONE_1F_MIN_MM		93		// d >= 93          -> 1F
#define ZONE_1F_2F_MIN_MM	63		// 63 <= d <= 92    -> 1F~2F
#define ZONE_2F_MIN_MM		56		// 56 <= d <= 62    -> 2F
#define ZONE_2F_3F_MIN_MM	31		// 31 <= d <= 55    -> 2F~3F
									// d <= 30          -> 3F

extern TIM_HandleTypeDef htim10;

static bool     is_measuring = false;	// 트리거를 보내고 결과를 기다리는 중
static bool     has_value    = false;	// 저장된 정상값이 있는지
static uint16_t last_mm      = 0;		// 마지막 정상 측정값 (mm)
static uint32_t prev_ms      = 0;		// 마지막 트리거 시각

static uint16_t filter_buf[DIST_FILTER_SIZE];	// 최근 정상 측정값 (원형 버퍼)
static uint8_t  filter_count = 0;				// 버퍼에 들어있는 개수
static uint8_t  filter_idx   = 0;				// 다음에 쓸 위치
static uint8_t  fail_count   = 0;				// 연속 실패 횟수

static void filter_reset(void)
{
	filter_count = 0;
	filter_idx = 0;
}

// 새 값을 넣고 버퍼에 있는 값들의 중앙값을 돌려준다
static uint16_t filter_push(uint16_t mm)
{
	filter_buf[filter_idx] = mm;
	filter_idx = (filter_idx + 1) % DIST_FILTER_SIZE;
	if (filter_count < DIST_FILTER_SIZE) {
		filter_count++;
	}

	// 복사본을 삽입 정렬 후 가운데 값 선택
	uint16_t sorted[DIST_FILTER_SIZE];
	for (uint8_t i = 0; i < filter_count; i++) {
		uint16_t v = filter_buf[i];
		int8_t j = i - 1;
		while (j >= 0 && sorted[j] > v) {
			sorted[j + 1] = sorted[j];
			j--;
		}
		sorted[j + 1] = v;
	}
	return sorted[filter_count / 2];
}

void dist_init() {
	HCSR04_init(GPIOC, GPIO_PIN_5, GPIOC, GPIO_PIN_6, &htim10);	// Trig PC5, Echo PC6
	is_measuring = false;
	has_value = false;
	fail_count = 0;
	filter_reset();
	prev_ms = HAL_GetTick();
}

void dist_task() {
	// 1) 측정 중이면 결과 확인
	if (is_measuring) {
		uint32_t echo_us = 0;
		hcsr04_status_t status = HCSR04_getResult(&echo_us);

		if (status == HCSR04_IN_PROGRESS) {
			return;		// 아직 에코 대기 중
		}
		is_measuring = false;

		if (status == HCSR04_OK) {
			// 거리 = 왕복시간 x 음속(343 m/s) / 2  ->  mm = us * 343 / 2000
			uint32_t mm = echo_us * 343 / 2000;
			if (mm <= DIST_MAX_MM) {
				last_mm = filter_push((uint16_t)mm);	// 중앙값 필터 적용
				has_value = true;
				fail_count = 0;
				return;
			}
		}
		// 에코 없음(타임아웃) 또는 범위 밖
		// 한두 번 실패는 무시하고 마지막 값을 유지, 연속 DIST_MAX_FAIL 번이면 무효 처리
		if (++fail_count >= DIST_MAX_FAIL) {
			fail_count = DIST_MAX_FAIL;
			has_value = false;
			filter_reset();
		}
		return;
	}

	// 2) 측정 중이 아니면 주기마다 트리거
	if (HAL_GetTick() - prev_ms < DIST_PERIOD_MS) {
		return;
	}
	prev_ms = HAL_GetTick();

	if (HCSR04_start() == HCSR04_OK) {
		is_measuring = true;
	}
}

// 다음 dist_task() 호출에서 바로 측정하도록 주기 대기를 건너뛴다
void dist_request(void) {
	prev_ms = HAL_GetTick() - DIST_PERIOD_MS;
}

distance_status_t distance_get_mm(uint16_t *distance_mm) {
	if (!has_value) {
		return DISTANCE_NOT_READY;
	}
	*distance_mm = last_mm;
	return DISTANCE_OK;
}

distance_status_t distance_get(uint16_t *distance_cm) {
	uint16_t mm;
	if (distance_get_mm(&mm) != DISTANCE_OK) {
		return DISTANCE_NOT_READY;
	}
	*distance_cm = (mm + 5) / 10;	// 반올림
	return DISTANCE_OK;
}

floor_zone_t distance_getFloorZone(void) {
	uint16_t mm;
	if (distance_get_mm(&mm) != DISTANCE_OK) {
		return ZONE_UNKNOWN;
	}
	if (mm >= ZONE_1F_MIN_MM)    return ZONE_1F;
	if (mm >= ZONE_1F_2F_MIN_MM) return ZONE_1F_2F;
	if (mm >= ZONE_2F_MIN_MM)    return ZONE_2F;
	if (mm >= ZONE_2F_3F_MIN_MM) return ZONE_2F_3F;
	return ZONE_3F;
}

// LCD 표시용 문자열 (3글자 고정)
const char *distance_zoneToString(floor_zone_t zone) {
	switch (zone) {
	case ZONE_1F:    return "1F ";
	case ZONE_1F_2F: return "1-2";
	case ZONE_2F:    return "2F ";
	case ZONE_2F_3F: return "2-3";
	case ZONE_3F:    return "3F ";
	default:         return "---";
	}
}

void dist_ISR() {
	HCSR04_echoISR();
}
