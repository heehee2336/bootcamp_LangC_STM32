/*
 * joyStick.c
 *
 *  Created on: 2026. 11. 3.
 *      Author: kccistc
 */
#include "joyStick.h"
#include "stm32f4xx_hal.h"

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

#define ADC_CH_COUNT 	2

volatile uint16_t value[ADC_CH_COUNT] = {0};

static uint16_t pout[ADC_CH_COUNT];
static uint16_t vout[ADC_CH_COUNT];

void js_init(void)
{
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)value, ADC_CH_COUNT);
}

void js_getValue(cordinate_XY *cord_value)
{
	pout[0] = (uint16_t)(value[0] * 100 / 4096);

	pout[1] = 100 - (uint16_t)(value[1] * 100 / 4096);

	cord_value -> X_cord = pout[0];
	cord_value -> Y_cord = pout[1];
}
