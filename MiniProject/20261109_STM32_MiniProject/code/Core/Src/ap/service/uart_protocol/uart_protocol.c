/*
 * uart_protocol.c
 *
 *  uartProto_task()를 메인 루프에서 계속 부른다.
 *  받은 바이트를 상태머신으로 조립 -> 체크섬 확인 -> 명령 실행 -> 응답 송신
 *  (모두 메인 루프에서 실행되므로 엘리베이터 상태와 충돌하지 않는다)
 */
#include "uart_protocol.h"
#include "../../../driver/uart/uart.h"
#include "../elevator/elevator.h"
#include "../distance/distance.h"
#include "../temp_humid/temp_humid.h"

#define PKT_MAX_LEN			16		// LEN 최대값 + 1 (DATA 최대 14바이트)
#define PKT_BYTE_TIMEOUT_MS	50		// 패킷 중간에 이 시간 이상 끊기면 버린다

extern UART_HandleTypeDef huart2;

typedef enum {
	RX_WAIT_STX,
	RX_WAIT_LEN,
	RX_WAIT_CMD,
	RX_WAIT_DATA,
	RX_WAIT_CHK
} rx_state_t;

static rx_state_t rx_state = RX_WAIT_STX;
static uint8_t  rx_len;
static uint8_t  rx_cmd;
static uint8_t  rx_data[PKT_MAX_LEN];
static uint8_t  rx_idx;
static uint8_t  rx_sum;
static uint32_t rx_last_ms;

static void proto_send(uint8_t cmd, const uint8_t *data, uint8_t data_len)
{
	uint8_t pkt[PKT_MAX_LEN + 3];
	uint8_t len = data_len + 1;
	uint8_t sum = len + cmd;

	pkt[0] = PKT_STX;
	pkt[1] = len;
	pkt[2] = cmd;
	for (uint8_t i = 0; i < data_len; i++) {
		pkt[3 + i] = data[i];
		sum += data[i];
	}
	pkt[3 + data_len] = sum;
	uart_write(pkt, data_len + 4);
}

static void proto_sendResult(uint8_t rsp_cmd, uint8_t result)
{
	proto_send(rsp_cmd, &result, 1);
}

static void proto_sendStatus(void)
{
	elevator_status_t st;
	dht11_data_t th;
	uint16_t mm;
	uint8_t d[12];

	Elevator_getStatus(&st);
	TH_getTempHumid(&th);
	if (distance_get_mm(&mm) != DISTANCE_OK) {
		mm = 0xFFFF;
	}

	d[0]  = st.mode;
	d[1]  = st.emergency_state;
	d[2]  = st.state;
	d[3]  = st.current_floor + 1;
	d[4]  = st.target_floor + 1;
	d[5]  = (uint8_t)distance_getFloorZone();
	d[6]  = (uint8_t)(mm >> 8);
	d[7]  = (uint8_t)(mm & 0xFF);
	d[8]  = th.t_int;
	d[9]  = th.t_dec;
	d[10] = th.h_int;
	d[11] = th.h_dec;
	proto_send(RSP_STATUS, d, sizeof(d));
}

// 완성된 패킷 처리 (data_len = LEN - 1)
static void proto_handle(uint8_t cmd, const uint8_t *data, uint8_t data_len)
{
	switch (cmd) {
	case CMD_STATUS_REQ:
		proto_sendStatus();
		break;

	case CMD_CALL_FLOOR:
		if (data_len != 1 || data[0] < 1 || data[0] > 3) {
			proto_sendResult(RSP_CALL_FLOOR, RESULT_BAD_PARAM);
		} else {
			bool ok = Elevator_requestFloor(data[0] - 1);	// 1~3층 -> FLOOR_1~FLOOR_3
			proto_sendResult(RSP_CALL_FLOOR, ok ? RESULT_OK : RESULT_REJECTED);
		}
		break;

	case CMD_EMERGENCY:
		if (data_len != 0) {
			proto_sendResult(RSP_EMERGENCY, RESULT_BAD_PARAM);
		} else {
			proto_sendResult(RSP_EMERGENCY, Elevator_requestEmergency() ? RESULT_OK : RESULT_REJECTED);
		}
		break;

	case CMD_ESCAPE:
		if (data_len != 0) {
			proto_sendResult(RSP_ESCAPE, RESULT_BAD_PARAM);
		} else {
			proto_sendResult(RSP_ESCAPE, Elevator_requestEscape() ? RESULT_OK : RESULT_REJECTED);
		}
		break;

	case CMD_MANUAL_MOVE:
		if (data_len != 1 || data[0] > MOVE_DOWN) {
			proto_sendResult(RSP_MANUAL_MOVE, RESULT_BAD_PARAM);
		} else {
			bool ok = Elevator_requestManualMove((elevator_move_t)data[0]);
			proto_sendResult(RSP_MANUAL_MOVE, ok ? RESULT_OK : RESULT_REJECTED);
		}
		break;

	default:
		break;		// 알 수 없는 CMD -> 무응답
	}
}

void uartProto_init(void)
{
	rx_state = RX_WAIT_STX;
	uart_init(&huart2);
}

void uartProto_task(void)
{
	uint8_t b;

	// 패킷 중간에 오래 끊기면 버리고 처음부터
	if (rx_state != RX_WAIT_STX && (HAL_GetTick() - rx_last_ms) > PKT_BYTE_TIMEOUT_MS) {
		rx_state = RX_WAIT_STX;
	}

	while (uart_read(&b)) {
		rx_last_ms = HAL_GetTick();

		switch (rx_state) {
		case RX_WAIT_STX:
			if (b == PKT_STX) {
				rx_state = RX_WAIT_LEN;
			}
			break;
		case RX_WAIT_LEN:
			if (b >= 1 && b < PKT_MAX_LEN) {
				rx_len = b;
				rx_sum = b;
				rx_state = RX_WAIT_CMD;
			} else {
				rx_state = RX_WAIT_STX;		// 잘못된 길이
			}
			break;
		case RX_WAIT_CMD:
			rx_cmd = b;
			rx_sum += b;
			rx_idx = 0;
			rx_state = (rx_len > 1) ? RX_WAIT_DATA : RX_WAIT_CHK;
			break;
		case RX_WAIT_DATA:
			rx_data[rx_idx++] = b;
			rx_sum += b;
			if (rx_idx >= rx_len - 1) {
				rx_state = RX_WAIT_CHK;
			}
			break;
		case RX_WAIT_CHK:
			if (b == rx_sum) {
				proto_handle(rx_cmd, rx_data, rx_len - 1);
			}
			// 체크섬 오류 -> 무응답
			rx_state = RX_WAIT_STX;
			break;
		default:
			rx_state = RX_WAIT_STX;
			break;
		}
	}
}
