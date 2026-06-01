#ifndef __DHT11_H
#define __DHT11_H

#include "stdint.h"
#include "main.h"
#include "stm32f1xx_hal.h"

#define DHT11_DQ_OUT_0()  HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET)
#define DHT11_DQ_OUT_1()  HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET)
#define DHT11_DQ_IN()     HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin)

typedef struct
{
    uint8_t humi_int;
    uint8_t humi_dec;
    uint8_t temp_int;
    uint8_t temp_dec;
    uint8_t check_sum;
} DHT11_Data_TypeDef;

void DHT11_Init(void);
uint8_t DHT11_Read_Data(DHT11_Data_TypeDef *data);

#endif
