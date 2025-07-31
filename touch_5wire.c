/*
 * touch_5wire.c
 *
 *  Created on: May 25, 2025
 *      Author: ngovi
 */

#include "touch_5wire.h"

void touch_5wire_init(
		TOUCH_5WIRE_t* Ts,
		ADC_HandleTypeDef *hadc,uint32_t Channel,
		GPIO_TypeDef* GPIO_Sen, uint16_t Pin_Sen,
		GPIO_TypeDef* GPIO_LR, uint16_t Pin_LR,
		GPIO_TypeDef* GPIO_LL, uint16_t Pin_LL,
		GPIO_TypeDef* GPIO_UR, uint16_t Pin_UR,
		GPIO_TypeDef* GPIO_UL, uint16_t Pin_UL){

	Ts->Sen_ADC.hadc = hadc;
	Ts->Sen_ADC.Channel = Channel;

	Ts->Sen_GPIO.GPIOx = GPIO_Sen;
	Ts->Sen_GPIO.Pin = Pin_Sen;

	Ts->LR.GPIOx = GPIO_LR; Ts->LR.Pin = Pin_LR;
	Ts->LL.GPIOx = GPIO_LL; Ts->LL.Pin = Pin_LL;
	Ts->UR.GPIOx = GPIO_UR; Ts->UR.Pin = Pin_UR;
	Ts->UL.GPIOx = GPIO_UL; Ts->UL.Pin = Pin_UL;

	Ts->total_x = 0;
	Ts->total_y = 0;
	Ts->current_index_x = 0;
	Ts->current_index_y = 0;

	for(int i = 0; i < 10; i++){
		Ts->arr_value_x[i] = 0;
		Ts->arr_value_y[i] = 0;
	}

	//Configure to GPIO
	set_to_gpio(Ts);
}

int32_t map(uint32_t input_value, uint32_t min_input, uint32_t max_input, int32_t min_output, int32_t max_output){
	return (input_value - min_input)*(max_output - min_output)/(max_input - min_input) + min_output;
}

uint32_t average_filter(uint32_t* arr_filter, uint32_t raw_value, uint32_t* total, uint8_t* current_index){
	*total -= arr_filter[*current_index];
	arr_filter[*current_index] = raw_value;
	*total += raw_value;
	(*current_index)++;
	if(*current_index >= 10){
		*current_index = 0;
	}
	return (uint32_t)(*total/10);
}

void Write_Pin(GPIO_WIRE_t wire, GPIO_PinState PinState){
	HAL_GPIO_WritePin(wire.GPIOx, wire.Pin, PinState);
}

GPIO_PinState  Read_Pin(GPIO_WIRE_t wire){
	return HAL_GPIO_ReadPin(wire.GPIOx, wire.Pin);
}

void config_touch_detect(TOUCH_5WIRE_t*  Ts){
	Write_Pin(Ts->LR, GPIO_PIN_RESET);
	Write_Pin(Ts->LL, GPIO_PIN_RESET);
	Write_Pin(Ts->UL, GPIO_PIN_RESET);
	Write_Pin(Ts->UR, GPIO_PIN_RESET);
}

void config_read_x(TOUCH_5WIRE_t* Ts){
	Write_Pin(Ts->LR, GPIO_PIN_SET);
	Write_Pin(Ts->LL, GPIO_PIN_RESET);
	Write_Pin(Ts->UL, GPIO_PIN_RESET);
	Write_Pin(Ts->UR, GPIO_PIN_SET);
}

void config_read_y(TOUCH_5WIRE_t* Ts){
	Write_Pin(Ts->LR, GPIO_PIN_RESET);
	Write_Pin(Ts->LL, GPIO_PIN_RESET);
	Write_Pin(Ts->UL, GPIO_PIN_SET);
	Write_Pin(Ts->UR, GPIO_PIN_SET);
}

void set_to_gpio(TOUCH_5WIRE_t* Ts){
	//Stop ADC
	HAL_ADC_Stop(Ts->Sen_ADC.hadc);
	HAL_ADC_DeInit(Ts->Sen_ADC.hadc);
	//Configure to GPIO
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = Ts->Sen_GPIO.Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(Ts->Sen_GPIO.GPIOx, &GPIO_InitStruct);
}

void set_to_adc(TOUCH_5WIRE_t* Ts){
	ADC_ChannelConfTypeDef sConfig = {0};

	Ts->Sen_ADC.hadc->Instance = ADC1;
	Ts->Sen_ADC.hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
	Ts->Sen_ADC.hadc->Init.Resolution = ADC_RESOLUTION_12B;
	Ts->Sen_ADC.hadc->Init.ScanConvMode = DISABLE;
	Ts->Sen_ADC.hadc->Init.ContinuousConvMode = DISABLE;
	Ts->Sen_ADC.hadc->Init.DiscontinuousConvMode = DISABLE;
	Ts->Sen_ADC.hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	Ts->Sen_ADC.hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
	Ts->Sen_ADC.hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
	Ts->Sen_ADC.hadc->Init.NbrOfConversion = 1;
	Ts->Sen_ADC.hadc->Init.DMAContinuousRequests = DISABLE;
	Ts->Sen_ADC.hadc->Init.EOCSelection = ADC_EOC_SINGLE_CONV;

	HAL_ADC_Init(Ts->Sen_ADC.hadc);

	sConfig.Channel = Ts->Sen_ADC.Channel;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	HAL_ADC_ConfigChannel(Ts->Sen_ADC.hadc, &sConfig);

}

uint8_t touch_detect(TOUCH_5WIRE_t* Ts){
	config_touch_detect(Ts);
	set_to_gpio(Ts);
	HAL_Delay(10);
	if(Read_Pin(Ts->Sen_GPIO) == GPIO_PIN_RESET){
		return 1;
	}
	return 0;
}

uint32_t get_raw_value_x(TOUCH_5WIRE_t* Ts){
	config_read_x(Ts);
	HAL_ADC_Start(Ts->Sen_ADC.hadc);
	HAL_ADC_PollForConversion(Ts->Sen_ADC.hadc, 10);
	return HAL_ADC_GetValue(Ts->Sen_ADC.hadc);
}

int32_t get_position_x(TOUCH_5WIRE_t* Ts, uint16_t min_adc_x, uint16_t max_adc_x, uint16_t length){
	uint32_t raw_value_x = get_raw_value_x(Ts);
	uint32_t value_filtered_x = average_filter(Ts->arr_value_x, raw_value_x, &Ts->total_x, &Ts->current_index_x);
	return map(value_filtered_x, min_adc_x, max_adc_x, -length/2, length/2);
}

uint32_t get_raw_value_y(TOUCH_5WIRE_t* Ts){
	config_read_y(Ts);
	HAL_ADC_Start(Ts->Sen_ADC.hadc);
	HAL_ADC_PollForConversion(Ts->Sen_ADC.hadc, 10);
	return HAL_ADC_GetValue(Ts->Sen_ADC.hadc);
}

int32_t get_position_y(TOUCH_5WIRE_t* Ts, uint16_t min_adc_y, uint16_t max_adc_y, uint16_t width){
	uint32_t raw_value_y = get_raw_value_y(Ts);
	uint32_t value_filtered_y =  average_filter(Ts->arr_value_y, raw_value_y, &Ts->total_y, &Ts->current_index_y);
	return map(value_filtered_y, min_adc_y, max_adc_y, -width/2, width/2);
}


