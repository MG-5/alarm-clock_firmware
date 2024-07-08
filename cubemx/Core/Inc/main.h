/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RTC_INT_Pin GPIO_PIN_13
#define RTC_INT_GPIO_Port GPIOC
#define ButtonLeft_Pin GPIO_PIN_14
#define ButtonLeft_GPIO_Port GPIOC
#define ButtonRight_Pin GPIO_PIN_15
#define ButtonRight_GPIO_Port GPIOC
#define enableESPBuckConverter_Pin GPIO_PIN_0
#define enableESPBuckConverter_GPIO_Port GPIOH
#define VibrationCushion_Pin GPIO_PIN_1
#define VibrationCushion_GPIO_Port GPIOH
#define LDR_ADC_Pin GPIO_PIN_0
#define LDR_ADC_GPIO_Port GPIOA
#define Alarm2_LED_Pin GPIO_PIN_1
#define Alarm2_LED_GPIO_Port GPIOA
#define enableWarmWhite_Pin GPIO_PIN_2
#define enableWarmWhite_GPIO_Port GPIOA
#define enableColdWhite_Pin GPIO_PIN_3
#define enableColdWhite_GPIO_Port GPIOA
#define PS_24V_ADC_Pin GPIO_PIN_4
#define PS_24V_ADC_GPIO_Port GPIOA
#define Alarm1_LED_Pin GPIO_PIN_5
#define Alarm1_LED_GPIO_Port GPIOA
#define BatteryADC_Pin GPIO_PIN_6
#define BatteryADC_GPIO_Port GPIOA
#define ButtonSnooze_Pin GPIO_PIN_7
#define ButtonSnooze_GPIO_Port GPIOA
#define ButtonBrightnessPlus_Pin GPIO_PIN_0
#define ButtonBrightnessPlus_GPIO_Port GPIOB
#define ButtonBrightnessMinus_Pin GPIO_PIN_1
#define ButtonBrightnessMinus_GPIO_Port GPIOB
#define ButtonCCTPlus_Pin GPIO_PIN_2
#define ButtonCCTPlus_GPIO_Port GPIOB
#define Status_LED_Red_Pin GPIO_PIN_10
#define Status_LED_Red_GPIO_Port GPIOB
#define Status_LED_Green_Pin GPIO_PIN_11
#define Status_LED_Green_GPIO_Port GPIOB
#define ButtonCCTMinus_Pin GPIO_PIN_12
#define ButtonCCTMinus_GPIO_Port GPIOB
#define enableHeatwire_Pin GPIO_PIN_13
#define enableHeatwire_GPIO_Port GPIOB
#define ShiftRegisterClock_Pin GPIO_PIN_14
#define ShiftRegisterClock_GPIO_Port GPIOB
#define Strobe_Pin GPIO_PIN_15
#define Strobe_GPIO_Port GPIOB
#define ShiftRegisterData_Pin GPIO_PIN_8
#define ShiftRegisterData_GPIO_Port GPIOA
#define enableGrid5_Pin GPIO_PIN_9
#define enableGrid5_GPIO_Port GPIOA
#define enableGrid4_Pin GPIO_PIN_10
#define enableGrid4_GPIO_Port GPIOA
#define enableGrid3_Pin GPIO_PIN_11
#define enableGrid3_GPIO_Port GPIOA
#define enableGrid2_Pin GPIO_PIN_12
#define enableGrid2_GPIO_Port GPIOA
#define enableGrid1_Pin GPIO_PIN_15
#define enableGrid1_GPIO_Port GPIOA
#define enableGrid0_Pin GPIO_PIN_4
#define enableGrid0_GPIO_Port GPIOB
#define enable35V_Pin GPIO_PIN_5
#define enable35V_GPIO_Port GPIOB
#define enableBatteryVoltageDivider_Pin GPIO_PIN_3
#define enableBatteryVoltageDivider_GPIO_Port GPIOH

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
