#ifndef PC_COTRL_INT_H
#define PC_COTRL_INT_H

#include "key_scan.h"
#include "main.h"

#define encoder_timeout_ms 400UL

/**
  * @brief  Encoder interrupt handler
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line for both encoder pins,
  * @param  keyboard pointer to IO_controls struct, only access the volatile enc_count
  * @retval None
  */
void ENC_EDGE_IT_Handler(uint16_t GPIO_Pin, IO_controls_handler keyboard);

/**
  * @brief  interrupt handler for all the pushbutton controls (PLAY, PAUSE, FORWARD, BACKWARDS) 
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line for both encoder pins,
  * @param  keyboard pointer to IO_controls struct, only access the volatile controls
  * @retval None
  */
void PUSH_BTN_IT_Handler(uint16_t GPIO_Pin, IO_controls_handler keyboard);


#endif 