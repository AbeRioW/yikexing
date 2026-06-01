/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "dht11.h"
#include "flash_storage.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
typedef enum {
    STATE_NORMAL,
    STATE_SET_TEMP,
    STATE_SET_HUMI,
    STATE_SAVE
} SystemState_t;

SystemState_t system_state = STATE_NORMAL;
uint8_t temp_threshold = 20;
uint8_t humi_threshold = 60;
volatile uint8_t key1_pressed = 0;
volatile uint8_t key2_pressed = 0;
volatile uint8_t key3_pressed = 0;
volatile uint32_t key1_last_time = 0;
volatile uint32_t key2_last_time = 0;
volatile uint32_t key3_last_time = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(500);
  
  OLED_Init();
  OLED_Clear();
  OLED_ShowString(0, 0, (uint8_t*)"Init...", 8, 1);
  OLED_Refresh();
  
  HAL_Delay(500);
  
  DHT11_Init();
  
  // 读取Flash
  OLED_ShowString(0, 16, (uint8_t*)"Read Flash", 8, 1);
  OLED_Refresh();
  Flash_Init();
  
  FlashData_t flash_data;
  if (Flash_ReadData(&flash_data)) {
      temp_threshold = flash_data.temp_threshold;
      humi_threshold = flash_data.humi_threshold;
      OLED_ShowString(0, 16, (uint8_t*)"Load OK", 8, 1);
  } else {
      OLED_ShowString(0, 16, (uint8_t*)"Default", 8, 1);
      // 首次使用写入默认值
      OLED_ShowString(0, 32, (uint8_t*)"Write...", 8, 1);
      OLED_Refresh();
      Flash_WriteData(temp_threshold, humi_threshold);
  }
  OLED_Refresh();
  HAL_Delay(1000);
  
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  
  OLED_Clear();
  OLED_ShowString(0, 0, (uint8_t*)"Temp:", 8, 1);
  OLED_ShowString(0, 16, (uint8_t*)"FAN:", 8, 1);
  OLED_Refresh();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    char str[32];
    
    if (key1_pressed) {
        key1_pressed = 0;
        if (system_state == STATE_NORMAL) {
            system_state = STATE_SET_TEMP;
        } else if (system_state == STATE_SET_TEMP) {
            system_state = STATE_SET_HUMI;
        } else if (system_state == STATE_SET_HUMI) {
            system_state = STATE_SAVE;
        } else if (system_state == STATE_SAVE) {
            system_state = STATE_NORMAL;
        }
    }
    
    if (key2_pressed) {
        key2_pressed = 0;
        if (system_state == STATE_SET_TEMP) {
            if (temp_threshold < 60) temp_threshold++;
        } else if (system_state == STATE_SET_HUMI) {
            if (humi_threshold < 100) humi_threshold++;
        }
    }
    
    if (key3_pressed) {
        key3_pressed = 0;
        if (system_state == STATE_SET_TEMP) {
            if (temp_threshold > 0) temp_threshold--;
        } else if (system_state == STATE_SET_HUMI) {
            if (humi_threshold > 0) humi_threshold--;
        }
    }
    
    if (system_state == STATE_NORMAL) {
        char fan_str[20];
        DHT11_Data_TypeDef dht_data;
        uint32_t pwm_period = 65535;
        
        if(DHT11_Read_Data(&dht_data) == 0)
        {
            sprintf(str, "%d.%dC H:%d.%d%%", dht_data.temp_int, dht_data.temp_dec, dht_data.humi_int, dht_data.humi_dec);
            
            int8_t temp = (int8_t)dht_data.temp_int;
            if(temp < temp_threshold)
            {
                uint32_t temp_diff = temp_threshold - temp;
                if(temp_diff > temp_threshold) temp_diff = temp_threshold;
                uint32_t pwm_value = (temp_diff * pwm_period) / temp_threshold;
                __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_value);
            }
            else
            {
                __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
            }
            
            uint32_t humi_diff = 0;
            if(dht_data.humi_int > humi_threshold)
            {
                uint8_t max_diff = 100 - humi_threshold;
                humi_diff = dht_data.humi_int - humi_threshold;
                if(humi_diff > max_diff) humi_diff = max_diff;
                uint32_t pwm_value = (humi_diff * pwm_period) / max_diff;
                __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pwm_value);
                uint32_t fan_speed = (humi_diff * 100) / max_diff;
                sprintf(fan_str, "ON %d%%", fan_speed);
            }
            else
            {
                __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
                sprintf(fan_str, "OFF      ");
            }
            
            OLED_ShowString(40, 0, (uint8_t*)str, 8, 1);
            OLED_ShowString(40, 16, (uint8_t*)fan_str, 8, 1);
            OLED_Refresh();
        }
        else
        {
            OLED_ShowString(40, 0, (uint8_t*)"Error", 8, 1);
            OLED_ShowString(40, 16, (uint8_t*)"     ", 8, 1);
            OLED_Refresh();
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
        }
        
        HAL_Delay(2000);
    } else if (system_state == STATE_SET_TEMP) {
        OLED_Clear();
        OLED_ShowString(0, 0, (uint8_t*)"Set Temp:", 8, 1);
        sprintf(str, "%d", temp_threshold);
        OLED_ShowString(40, 16, (uint8_t*)str, 8, 1);
        OLED_Refresh();
        HAL_Delay(100);
    } else if (system_state == STATE_SET_HUMI) {
        OLED_Clear();
        OLED_ShowString(0, 0, (uint8_t*)"Set Humi:", 8, 1);
        sprintf(str, "%d", humi_threshold);
        OLED_ShowString(40, 16, (uint8_t*)str, 8, 1);
        OLED_Refresh();
        HAL_Delay(100);
    } else if (system_state == STATE_SAVE) {
        OLED_Clear();
        OLED_ShowString(0, 0, (uint8_t*)"Saving...", 8, 1);
        OLED_Refresh();
        if (Flash_WriteData(temp_threshold, humi_threshold)) {
            OLED_ShowString(0, 16, (uint8_t*)"OK!", 8, 1);
        } else {
            OLED_ShowString(0, 16, (uint8_t*)"Fail!", 8, 1);
        }
        OLED_Refresh();
        HAL_Delay(1000);
        system_state = STATE_NORMAL;
        OLED_Clear();
        OLED_ShowString(0, 0, (uint8_t*)"Temp:", 8, 1);
        OLED_ShowString(0, 16, (uint8_t*)"FAN:", 8, 1);
        OLED_Refresh();
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    uint32_t now = HAL_GetTick();
    
    if (GPIO_Pin == KEY1_Pin) {
        if (now - key1_last_time > 200) {
            key1_pressed = 1;
            key1_last_time = now;
        }
    } else if (GPIO_Pin == KEY2_Pin) {
        if (now - key2_last_time > 200) {
            key2_pressed = 1;
            key2_last_time = now;
        }
    } else if (GPIO_Pin == KEY3_Pin) {
        if (now - key3_last_time > 200) {
            key3_pressed = 1;
            key3_last_time = now;
        }
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
