/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    pc_cotrl_int.c
  * @brief   This file provides code for the interupt handling function used 
  *           of the volume encoder and push buttons
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "pc_cotrl_int.h"


/**
  * @brief  Encoder interrupt handler
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line for both encoder pins,
  * @param  keyboard pointer to keypad_controls struct, only access the volatile enc_count
  * @retval None
  */
void ENC_EDGE_IT_Handler(uint16_t GPIO_Pin, keypad_controls_handler keyboard)
{
  //glitchin was mitigated by the reseting every 300 ms, its much more consistent, but it results in the slower fine tuning inputs to be ignored/reset
  //you can still glitch it by mooving it up and down, but it resets every 300 ms.
  
  //TODO: find a better srat for fixing the glithcing.
  
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != 0x00u)
  { 
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);

    static uint16_t prev_pin;
    static uint32_t tick_base;

    uint32_t tick_current = (uint32_t)HAL_GetTick();

    keyboard->enc_fall_edges++;

    keyboard->enc_count = keyboard->enc_count+ (!(keyboard->enc_fall_edges%2) && (GPIO_Pin == ENC_A_Pin && prev_pin == ENC_B_Pin)&&((tick_current-tick_base)<encoder_timeout_ms))*1 + 
    (!(keyboard->enc_fall_edges%2) && (GPIO_Pin == ENC_B_Pin && prev_pin == ENC_A_Pin)&&((tick_current-tick_base)<encoder_timeout_ms))*-1;
    
    keyboard->enc_count= ((keyboard->enc_count< 100) && (keyboard->enc_count> -100)) * (keyboard->enc_count) + 
    (keyboard->enc_count>= 100)*100 + (keyboard->enc_count<= -100)*-100;

    keyboard->enc_fall_edges = ((keyboard->enc_fall_edges>=2)||((tick_current-tick_base)>encoder_timeout_ms))*0 + 
    ((keyboard->enc_fall_edges<2)&&((tick_current-tick_base)<encoder_timeout_ms))*keyboard->enc_fall_edges;
    
    prev_pin = ((keyboard->enc_fall_edges>=2)||((tick_current-tick_base)>encoder_timeout_ms))*0+
    ((keyboard->enc_fall_edges<2)&&((tick_current-tick_base)<encoder_timeout_ms))*GPIO_Pin;
    
    tick_base = tick_current;

  }
}
/**
  * @brief  interrupt handler for all the pushbutton controls 
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line for both encoder pins,
  * @param  keyboard pointer to keypad_controls struct, only access the volatile controls
  * @retval None
  */
void PUSH_BTN_IT_Handler(uint16_t GPIO_Pin, keypad_controls_handler keyboard){
  
  
  //this could be done better if we use timers again rather the cpu clock
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != 0x00u)
  { 
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);

    keyboard->button_press_tick_check=(uint32_t)HAL_GetTick();
    keyboard->populate_media_slow_controls_flag=1;

    //  0x09, 0xE2,        //   Usage (Mute)
    // 0x09, 0xCD,        //   Usage (Play/Pause)
    // 0x09, 0xB5,        //   Usage (Scan Next Track)
    // 0x09, 0xB6,        //   Usage (Scan Previous Track)
    //it seems that the toogle for the mute changes state for each pulse of the operations for the usage so to mute we do 1 thn 0 and unmute need to send another 1 then 0
    keyboard->controls |= (GPIO_Pin == MUTE_Pin )*(1<<0);
    keyboard->controls |=(GPIO_Pin == PLAY_PAUSE_Pin && (((keyboard->controls)&0x04)==0x04))*(1<<3) ;
    keyboard->controls |=(GPIO_Pin == PLAY_PAUSE_Pin && (((keyboard->controls)&0x02)==0x02))*(1<<2);
    keyboard->controls |=(GPIO_Pin == PLAY_PAUSE_Pin )*(1<<1);

    //Mute 1, Play 1 or 0, Pause 2 or 3, Next track 4 or 5, Previous 6 or 7
    
  }
}
