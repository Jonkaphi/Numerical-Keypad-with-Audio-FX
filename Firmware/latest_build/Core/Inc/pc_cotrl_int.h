/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    pc_cotrl_int.h
  * @brief   Contains the function prototypes for the interrupt handlers for the
  *           volume and play/pause contols
  *          
  * @author  Yoan Philipov
  ******************************************************************************
  * @attention
  *
  * 
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef PC_COTRL_INT_H
#define PC_COTRL_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "key_scan.h"
#include "main.h"

#define encoder_timeout_ms 400UL

/**
  * @brief  Encoder interrupt handler
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line for both encoder pins,
  * @param  keyboard pointer to keypad_controls struct, only access the volatile enc_count
  * @retval None
  */
void ENC_EDGE_IT_Handler(uint16_t GPIO_Pin, keypad_controls_handler keyboard);

/**
  * @brief  interrupt handler for all the pushbutton controls (PLAY, PAUSE, FORWARD, BACKWARDS) 
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line for both encoder pins,
  * @param  keyboard pointer to keypad_controls struct, only access the volatile controls
  * @retval None
  */
void PUSH_BTN_IT_Handler(uint16_t GPIO_Pin, keypad_controls_handler keyboard);

#ifdef __cplusplus
}
#endif

#endif //PC_COTRL_INT_H