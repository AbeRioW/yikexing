#include "flash_storage.h"

void Flash_Init(void) {
}

uint8_t Flash_ReadData(FlashData_t *data) {
    FlashData_t *flash_data = (FlashData_t *)FLASH_STORAGE_ADDR;
    
    data->magic = flash_data->magic;
    data->temp_threshold = flash_data->temp_threshold;
    data->humi_threshold = flash_data->humi_threshold;
    
    if (data->magic != FLASH_MAGIC) {
        return 0;
    }
    
    // 简单范围检查
    if (data->temp_threshold > 60 || data->humi_threshold > 100) {
        return 0;
    }
    
    return 1;
}

uint8_t Flash_WriteData(uint8_t temp_threshold, uint8_t humi_threshold) {
    HAL_StatusTypeDef status;
    FlashData_t data;
    uint32_t PAGEError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;
    
    data.magic = FLASH_MAGIC;
    data.temp_threshold = temp_threshold;
    data.humi_threshold = humi_threshold;
    data.reserved[0] = 0;
    data.reserved[1] = 0;
    
    HAL_FLASH_Unlock();
    
    // 擦除Flash页
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = FLASH_STORAGE_ADDR;
    EraseInitStruct.NbPages = 1;
    status = HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return 0;
    }
    
    // 写入数据（8字节）
    uint32_t *src = (uint32_t *)&data;
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_STORAGE_ADDR, src[0]);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return 0;
    }
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_STORAGE_ADDR + 4, src[1]);
    if (status != HAL_OK) {
        HAL_FLASH_Lock();
        return 0;
    }
    
    HAL_FLASH_Lock();
    return 1;
}
