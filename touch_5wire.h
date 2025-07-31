/*
 * touch_5wire.h
 *
 *  Created on: May 25, 2025
 *      Author: ngovi
 */

#ifndef TOUCH_5WIRE_H_
#define TOUCH_5WIRE_H_

#include "stm32f4xx_hal.h"

typedef struct{
	GPIO_TypeDef* GPIOx;
	uint16_t Pin;
}GPIO_WIRE_t;

typedef struct{
	ADC_HandleTypeDef *hadc;
	uint32_t Channel;
}ADC_WIRE_t;

//5-wire touch screen: SS, LR, LL, UR, UL
typedef struct{
	ADC_WIRE_t Sen_ADC;
	GPIO_WIRE_t Sen_GPIO;
	GPIO_WIRE_t LR;
	GPIO_WIRE_t LL;
	GPIO_WIRE_t UR;
	GPIO_WIRE_t UL;

	uint32_t arr_value_x[10];
	uint32_t total_x;
	uint8_t current_index_x;
	uint32_t arr_value_y[10];
	uint32_t total_y;
	uint8_t current_index_y;

}TOUCH_5WIRE_t;


void touch_5wire_init(
		TOUCH_5WIRE_t* Ts,
		ADC_HandleTypeDef *hadc,uint32_t Channel,
		GPIO_TypeDef* GPIO_Sen, uint16_t Pin_Sen,
		GPIO_TypeDef* GPIO_LR, uint16_t Pin_LR,
		GPIO_TypeDef* GPIO_LL, uint16_t Pin_LL,
		GPIO_TypeDef* GPIO_UR, uint16_t Pin_UR,
		GPIO_TypeDef* GPIO_UL, uint16_t Pin_UL);

int32_t map(uint32_t input_value, uint32_t min_input, uint32_t max_input, int32_t min_output, int32_t max_output);

uint32_t average_filter(uint32_t* arr_filter, uint32_t raw_value, uint32_t* total, uint8_t* current_index);

void Write_Pin(GPIO_WIRE_t wire, GPIO_PinState PinState);

GPIO_PinState  Read_Pin(GPIO_WIRE_t wire);

void config_touch_detect(TOUCH_5WIRE_t*  Ts);

void config_read_x(TOUCH_5WIRE_t* Ts);

void config_read_y(TOUCH_5WIRE_t* Ts);

void set_to_gpio(TOUCH_5WIRE_t* Ts);

void set_to_adc(TOUCH_5WIRE_t* Ts);

uint8_t touch_detect(TOUCH_5WIRE_t* Ts);

uint32_t get_raw_value_x(TOUCH_5WIRE_t* Ts);

uint32_t get_raw_value_y(TOUCH_5WIRE_t* Ts);

int32_t get_position_x(TOUCH_5WIRE_t* Ts, uint16_t min_adc_x, uint16_t max_adc_x, uint16_t length);

int32_t get_position_y(TOUCH_5WIRE_t* Ts, uint16_t min_adc_y, uint16_t max_adc_y, uint16_t width);

#endif /* TOUCH_5WIRE_H_ */
