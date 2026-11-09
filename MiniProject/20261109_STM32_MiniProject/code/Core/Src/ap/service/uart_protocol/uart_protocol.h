/*
 * uart_protocol.h
 *
 *  PC(GUI) <-> STM32 바이너리 패킷 프로토콜
 *
 *  Frame : [STX 0xAA][LEN][CMD][DATA (LEN-1)][CHK]
 *          LEN = CMD 1바이트 + DATA 바이트 수 (1 ~ 15)
 *          CHK = (LEN + CMD + DATA 합) 의 하위 8비트 (STX 제외)
 *  UART  : USART2, 115200 8N1
 *
 *  체크섬 오류 / 알 수 없는 CMD -> 응답하지 않음 (PC는 타임아웃으로 처리)
 */

#ifndef SRC_AP_SERVICE_UART_PROTOCOL_UART_PROTOCOL_H_
#define SRC_AP_SERVICE_UART_PROTOCOL_UART_PROTOCOL_H_

#include "stm32f4xx_hal.h"

#define PKT_STX				0xAA

// ---- Request (PC -> STM32) ----
#define CMD_STATUS_REQ		0x10	// DATA 없음
#define CMD_CALL_FLOOR		0x20	// DATA 1 : 층 (1 ~ 3)
#define CMD_EMERGENCY		0x21	// DATA 없음 : NORMAL -> EMERGENCY
#define CMD_ESCAPE			0x22	// DATA 없음 : EMERGENCY 탈출 (1층 복귀) = 1번 버튼과 동일
#define CMD_MANUAL_MOVE		0x23	// DATA 1 : 0 정지, 1 상승, 2 하강 (EMERGENCY 수동 조작 중)
									//  버튼을 누르고 있는 동안 PC가 100ms 마다 반복 전송,
									//  300ms 동안 안 오면 STM32가 스스로 정지 (연결 끊김 대비)

// ---- Response (STM32 -> PC) : 요청 CMD | 0x80 ----
#define RSP_STATUS			0x90	// DATA 12 (아래 표)
#define RSP_CALL_FLOOR		0xA0	// DATA 1 : 결과
#define RSP_EMERGENCY		0xA1	// DATA 1 : 결과
#define RSP_ESCAPE			0xA2	// DATA 1 : 결과
#define RSP_MANUAL_MOVE		0xA3	// DATA 1 : 결과

// 결과 코드
#define RESULT_OK			0x00
#define RESULT_REJECTED		0x01	// 지금 상태에서는 할 수 없음
#define RESULT_BAD_PARAM	0x02	// DATA 길이나 값이 잘못됨

// CMD_MANUAL_MOVE DATA
#define MOVE_STOP			0x00
#define MOVE_UP				0x01
#define MOVE_DOWN			0x02

/*
 *  RSP_STATUS DATA (12 bytes)
 *   [0]  mode            0 NORMAL, 1 EMERGENCY
 *   [1]  emergency state 0 MANUAL, 1 ESCAPE (EMERGENCY일 때만 의미 있음)
 *   [2]  elevator state  0 IDLE, 1 RISING, 2 FALLING, 3 ARRIVE
 *   [3]  current floor   1 ~ 3
 *   [4]  target floor    1 ~ 3
 *   [5]  floor zone      0 UNKNOWN, 1 1F, 2 1F~2F, 3 2F, 4 2F~3F, 5 3F (초음파 판단)
 *   [6]  distance MSB    mm 단위, 0xFFFF = 측정값 없음
 *   [7]  distance LSB
 *   [8]  temp int        [9]  temp dec
 *   [10] humid int       [11] humid dec
 */

void uartProto_init(void);
void uartProto_task(void);

#endif /* SRC_AP_SERVICE_UART_PROTOCOL_UART_PROTOCOL_H_ */
