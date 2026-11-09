/*
 * uart.h
 *
 *  USART2(ST-LINK VCP) 바이트 송수신 드라이버
 *  - 수신: 1바이트 인터럽트 수신 -> 링버퍼에 저장 (ISR에서는 저장만 한다)
 *  - 송신: 메인 루프에서 blocking 송신 (패킷이 짧아서 1ms 이내)
 */

#ifndef SRC_DRIVER_UART_UART_H_
#define SRC_DRIVER_UART_UART_H_

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

void uart_init(UART_HandleTypeDef *huart);
bool uart_read(uint8_t *byte);						// 받은 바이트가 있으면 true
void uart_write(const uint8_t *data, uint16_t len);

#endif /* SRC_DRIVER_UART_UART_H_ */
