/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ErrorCodes.h
  * @brief   This file contains all the error codes for the error 
  *          handling in the main application code
  * @author  Yoan Philipov
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
typedef enum
{
    NO_ERROR,

    //MAIN APP CODES
    CANT_MOUNT_CARD,
    NOT_16BIT_DEPTH,
    WRONG_BIT_RATE,
    CANT_SET_RW_POINTER,
    CANT_OPEN_AUDIO_FILE,
    CANT_READ_AUDIO_FILE,
    CANT_OPEN_LOG_FILE,
    
    UKNOWN_STATE,

    //RCC PERIPH CODES
    RCC_OSC_FAIL,
    RCC_CLK_FAIL,
    RCC_CLK_PERIPH_FAIL,

    //USB PERIPH CODES
    USB_HAL_INIT_FAIL,
    USB_HAL_START_FAIL,
    USB_LL_INIT_FAIL,
    USB_LL_START_FAIL,
    USB_LL_RESEST_CALLBACK_FAIL,

    //TIM FOR LED CODES
    LED_TIM_INIT_FAIL,
    LED_TIM_CONFIG_FAIL,

    //DAC PERIPH CODES
    DAC_INIT_FAIL,
    DAC_CONFIG_FAIL,
    DAC_DMA_INIT_FAIL,

    //TIM FOR DAC CODES
    DAC_TIM_INIT_FAIL,
    DAC_TIM_CONFIG_FAIL,
    
    SPI_INIT_FAIL
}Error;