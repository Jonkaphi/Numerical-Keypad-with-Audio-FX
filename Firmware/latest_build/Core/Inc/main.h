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
#include "stm32f0xx_hal.h"
#include "ErrorCodes.h"

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
//
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(Error index);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_STAT_Pin GPIO_PIN_13
#define LED_STAT_GPIO_Port GPIOC
#define CLM_1_Pin GPIO_PIN_0
#define CLM_1_GPIO_Port GPIOA
#define CLM_2_Pin GPIO_PIN_1
#define CLM_2_GPIO_Port GPIOA
#define CLM_3_Pin GPIO_PIN_2
#define CLM_3_GPIO_Port GPIOA
#define AMP_SHUTDWN_Pin GPIO_PIN_3
#define AMP_SHUTDWN_GPIO_Port GPIOA
#define SPI_CS_Pin GPIO_PIN_0
#define SPI_CS_GPIO_Port GPIOB
#define CARD_detect_Pin GPIO_PIN_1
#define CARD_detect_GPIO_Port GPIOB
#define CARD_SHUTDOWN_Pin GPIO_PIN_2
#define CARD_SHUTDOWN_GPIO_Port GPIOB
#define CLM_4_Pin GPIO_PIN_10
#define CLM_4_GPIO_Port GPIOB
#define CLM_5_Pin GPIO_PIN_11
#define CLM_5_GPIO_Port GPIOB
#define CLM_6_Pin GPIO_PIN_12
#define CLM_6_GPIO_Port GPIOB
#define CLM_7_Pin GPIO_PIN_13
#define CLM_7_GPIO_Port GPIOB
#define ROW_5_Pin GPIO_PIN_14
#define ROW_5_GPIO_Port GPIOB
#define ROW_4_Pin GPIO_PIN_15
#define ROW_4_GPIO_Port GPIOB
#define ROW_3_Pin GPIO_PIN_8
#define ROW_3_GPIO_Port GPIOA
#define ROW_2_Pin GPIO_PIN_9
#define ROW_2_GPIO_Port GPIOA
#define ROW_1_Pin GPIO_PIN_10
#define ROW_1_GPIO_Port GPIOA
#define PLAY_PAUSE_Pin GPIO_PIN_3
#define PLAY_PAUSE_GPIO_Port GPIOB
#define MUTE_Pin GPIO_PIN_4
#define MUTE_GPIO_Port GPIOB
#define ENC_B_Pin GPIO_PIN_5
#define ENC_B_GPIO_Port GPIOB
#define ENC_A_Pin GPIO_PIN_6
#define ENC_A_GPIO_Port GPIOB

#define DAC_FX_OUT DAC_CHANNEL_1

#define SYS_CLK_MHZ 48000U
#define DEFAULT_PRESCALER 119U

/* USER CODE BEGIN Private defines */
#define POWER_ON_SDCARD HAL_GPIO_WritePin(GPIOB, CARD_SHUTDOWN_Pin, GPIO_PIN_SET);
#define POWER_OFF_SDCARD HAL_GPIO_WritePin(GPIOB, CARD_SHUTDOWN_Pin, GPIO_PIN_RESET);

#define POWER_ON_AUDIO_AMP HAL_GPIO_WritePin(GPIOA, AMP_SHUTDWN_Pin, GPIO_PIN_SET);
#define POWER_OFF_AUDIO_AMP HAL_GPIO_WritePin(GPIOA, AMP_SHUTDWN_Pin, GPIO_PIN_RESET);

#define GET_TIMER_PERIOD(X) (uint16_t)(((uint32_t)((SYS_CLK_MHZ/(DEFAULT_PRESCALER+1))*1000))/X);

//keyword for FSM variables
typedef uint8_t STATE;
//Keyword for control variables in if statements
typedef uint8_t CONTROL;
//keyword for variable holding timmings
typedef uint32_t TIME;
//keyword for file name strings
typedef uint8_t FILE_NAME;

//FSM States
#define KEYBOARD_SCAN 1U
#define SEND_HID_REPORTS 2U
/* USER CODE END Private defines */


//Settings: Configure the features of the build
//NOTE:Given the small size of flash memory on the F072C8T6
//SD card Logging has been sepperated out
#define __LOGGING_ENABLED 0
#define __DEBUG 0
#define __AUDIO_FX_ENABLED 1

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
