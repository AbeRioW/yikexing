#ifndef __FLASH_STORAGE_H__
#define __FLASH_STORAGE_H__

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define FLASH_STORAGE_ADDR 0x0800F800  // STM32F103C8T6最后一页（64KB Flash）
#define FLASH_MAGIC 0x12345678

typedef struct {
    uint32_t magic;
    uint8_t temp_threshold;
    uint8_t humi_threshold;
    uint8_t reserved[2];  // 对齐
} FlashData_t;

void Flash_Init(void);
uint8_t Flash_ReadData(FlashData_t *data);
uint8_t Flash_WriteData(uint8_t temp_threshold, uint8_t humi_threshold);

#endif
