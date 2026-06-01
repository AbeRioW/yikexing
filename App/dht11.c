#include "dht11.h"

static void DHT11_Delay_us(uint32_t us)
{
    uint32_t i;
    for(i = 0; i < us * 8; i++)
    {
        __NOP();
    }
}

static void DHT11_GPIO_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

static void DHT11_GPIO_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

static void DHT11_Rst(void)
{
    DHT11_GPIO_Output();
    DHT11_DQ_OUT_0();
    HAL_Delay(20);
    DHT11_DQ_OUT_1();
    DHT11_Delay_us(30);
}

static uint8_t DHT11_Check(void)
{
    uint8_t retry = 0;
    DHT11_GPIO_Input();
    while(DHT11_DQ_IN() && retry < 100)
    {
        retry++;
        DHT11_Delay_us(1);
    }
    if(retry >= 100) return 1;
    else retry = 0;
    while(!DHT11_DQ_IN() && retry < 100)
    {
        retry++;
        DHT11_Delay_us(1);
    }
    if(retry >= 100) return 1;
    return 0;
}

static uint8_t DHT11_Read_Bit(void)
{
    uint8_t retry = 0;
    while(DHT11_DQ_IN() && retry < 100)
    {
        retry++;
        DHT11_Delay_us(1);
    }
    retry = 0;
    while(!DHT11_DQ_IN() && retry < 100)
    {
        retry++;
        DHT11_Delay_us(1);
    }
    DHT11_Delay_us(40);
    if(DHT11_DQ_IN()) return 1;
    else return 0;
}

static uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, dat = 0;
    for(i = 0; i < 8; i++)
    {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}

void DHT11_Init(void)
{
    DHT11_GPIO_Output();
    DHT11_DQ_OUT_1();
}

uint8_t DHT11_Read_Data(DHT11_Data_TypeDef *data)
{
    uint8_t buf[5];
    uint8_t i;
    
    DHT11_Rst();
    if(DHT11_Check() == 0)
    {
        for(i = 0; i < 5; i++)
        {
            buf[i] = DHT11_Read_Byte();
        }
        if((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            data->humi_int = buf[0];
            data->humi_dec = buf[1];
            data->temp_int = buf[2];
            data->temp_dec = buf[3];
            data->check_sum = buf[4];
            
            DHT11_GPIO_Output();
            DHT11_DQ_OUT_1();
            
            return 0;
        }
    }
    
    DHT11_GPIO_Output();
    DHT11_DQ_OUT_1();
    
    return 1;
}
