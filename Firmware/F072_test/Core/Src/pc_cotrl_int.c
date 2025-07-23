#include "stm32f0xx_hal.h"
#include "main.h"
#include "pc_cotrl_int.h"

void HAL_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin, volatile int8_t * count_pntr)
{
  /* EXTI line interrupt detected */
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != 0x00u)
  { 
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
    HAL_GPIO_EXTI_ENC_Callback(GPIO_Pin, count_pntr);
  }
}

/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
__weak void HAL_GPIO_EXTI_ENC_Callback(uint16_t GPIO_Pin, volatile int8_t * count_pntr)
{
  /* Prevent unused argument(s) compilation warning */
  static uint8_t cnt_fall;
  cnt_fall = (cnt_fall != 1) + (!(cnt_fall != 1)*(cnt_fall + 1));
 
  *count_pntr = *count_pntr + (cnt_fall == 2 && GPIO_Pin == ENC_B_Pin)*1 + (cnt_fall == 2 && GPIO_Pin == ENC_A_Pin)*-1;

  *count_pntr = (!(*count_pntr >= 127) && !(*count_pntr <= -126)) * (*count_pntr) + (*count_pntr >= 127)*127 + (*count_pntr <= -126)*-126;

  /* NOTE: This function should not be modified, when the callback is needed,
            the HAL_GPIO_EXTI_Callback could be implemented in the user file
   */ 
}

void HAL_GPIO_EXTI_PUSH_Callback(uint16_t GPIO_Pin, volatile uint8_t * btn_stat ){
  static uint32_t tick_base;
  uint32_t tick_current = (uint32_t)HAL_GetTick;
  

  if(__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != 0x00u)
  { 
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
    *btn_stat = *btn_stat - 2*((*btn_stat > 1) && ((tick_current - tick_base)>= 300));
    *btn_stat = (GPIO_Pin == MUTE_Pin)*1 + (GPIO_Pin == PLAY_PAUSE_Pin)*2 + (GPIO_Pin == PLAY_PAUSE_Pin && *btn_stat >= 2)*2;
    tick_base = tick_current*(*btn_stat >= 2) + tick_base*(!(*btn_stat >= 2));
    
    
  }
}
