
#include "stm32f0xx_hal_def.h"

void              HAL_GPIO_EXTI_ENC_Callback(uint16_t GPIO_Pin, volatile int8_t * count_pntr);
void              HAL_GPIO_EXTI_PUSH_Callback(uint16_t GPIO_Pin, volatile uint8_t * btn_stat);