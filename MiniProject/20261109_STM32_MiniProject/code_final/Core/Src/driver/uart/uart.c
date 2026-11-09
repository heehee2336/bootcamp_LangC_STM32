/*
 * uart.c
 */
#include "uart.h"

#define UART_RX_BUF_SIZE	64		// 2의 거듭제곱
#define UART_TX_TIMEOUT_MS	10

static UART_HandleTypeDef *uart_huart;
static uint8_t rx_byte;									// HAL이 1바이트씩 받아오는 자리
static volatile uint8_t rx_buf[UART_RX_BUF_SIZE];		// 링버퍼
static volatile uint16_t rx_head = 0;					// ISR이 쓰는 위치
static volatile uint16_t rx_tail = 0;					// 메인 루프가 읽는 위치

void uart_init(UART_HandleTypeDef *huart)
{
	uart_huart = huart;
	rx_head = 0;
	rx_tail = 0;
	HAL_UART_Receive_IT(uart_huart, &rx_byte, 1);
}

bool uart_read(uint8_t *byte)
{
	if (rx_head == rx_tail) {
		return false;
	}
	*byte = rx_buf[rx_tail];
	rx_tail = (rx_tail + 1) & (UART_RX_BUF_SIZE - 1);
	return true;
}

void uart_write(const uint8_t *data, uint16_t len)
{
	HAL_UART_Transmit(uart_huart, (uint8_t *)data, len, UART_TX_TIMEOUT_MS);
}

// 1바이트 수신 완료 -> 링버퍼에 넣고 다시 수신 대기
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart != uart_huart) {
		return;
	}
	uint16_t next = (rx_head + 1) & (UART_RX_BUF_SIZE - 1);
	if (next != rx_tail) {			// 버퍼가 가득 차면 새 바이트는 버린다
		rx_buf[rx_head] = rx_byte;
		rx_head = next;
	}
	HAL_UART_Receive_IT(uart_huart, &rx_byte, 1);
}

// 오버런/노이즈 등 에러가 나면 HAL이 수신을 멈추므로 다시 시작해 준다
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart != uart_huart) {
		return;
	}
	HAL_UART_Receive_IT(uart_huart, &rx_byte, 1);
}
