/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "key_scan.h"
#include "usb_device.h"
#include "usbd_hid.h"
#include "pc_cotrl_int.h"

//#include "dac.h"
#include "fatfs.h"
#include "spi.h"

#include "sd_functions.h"
#include "stdio.h"
#include "sd_benchmark.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
extern USBD_HandleTypeDef hUsbDeviceFS;

uint8_t bufr[80];
UINT br;
IO_controls keyboard_inst;
IO_controls_handler keyboard=&keyboard_inst;



//IO_controls keyboard;//initilise contents to 0

 //Modifyer, reserved,Keycode.....

/* USER CODE END PV */
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
  uint32_t prev_tick=0;
  //uint8_t temp=0;
  

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
   RCC->APB2ENR |= RCC_APB2ENR_DBGMCUEN; //enables suspension of the APB2 bus dduring debug
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  MX_LED_TIM_Init();
  //MX_SPI1_Init();

  //MX_DAC_Init();
  //MX_FATFS_Init();

  keyboard->controls = 0;
  keyboard->enc_count = 0;// there is problems with iveflows and underflows
  keyboard->packet_update_flag = 0;
  keyboard->highest_element = 0;
  keyboard->sys_fast_controls_update_flag = 0;
  keyboard->sys_slow_controls_update_flag = 0;
  keyboard->enc_fall_edges = 0;


  HAL_TIM_Base_Start_IT(&led_tim);
  HAL_Delay(500);//Delay is required since the code executes much faster than the enumeration period.
  
  prev_tick=(uint32_t)HAL_GetTick();
  
  
 
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    // sd_mount();
    // sd_list_files();
    // sd_unmount();
    /* USER CODE BEGIN 3 */
    
    //currelty pooling is a bit slow since the sequtial exection of the scanning functions,
    //should make the report fucntion be called by the timmer ever 1 ms.
    scan_keyboard(keyboard);

    populate_HID_keycode_report(keyboard);

    populat_HID_fast_media_controls_report(keyboard);

    if(((uint32_t)HAL_GetTick()-prev_tick)>=1000){
      prev_tick=(uint32_t)HAL_GetTick();
      populat_HID_slow_media_controls_report(keyboard);
    }

    if(keyboard->packet_update_flag){
	    USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t *)keyboard->keys_HID_data_report, KEY_SCAN_HID_REPORT_KEYS_SIZE );
      keyboard->packet_update_flag=0;
    }


    //could make two sepperate report be send one for the mute and encoder, and a sepperate for the play pause forwars, backward
    //edit: yes that helped, but it sill is ver much needed to make the sending of the reports in a DMA format.
    if((keyboard->sys_fast_controls_update_flag)){

      //temp=keyboard->media_fast_controls_HID_report[1];
      //keyboard->media_fast_controls_HID_report[1]= (keyboard->media_fast_controls_HID_report[1] & 0x01);
      USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t *)keyboard->media_fast_controls_HID_report,KEY_SCAN_HID_REPORT_MEDIA_SIZE);//direction of volume incremnetation radomly swicthes and button control dontw work reliably
      //keyboard->keys_HID_data_report[1]=temp;
      HAL_Delay(10);

      // keyboard->media_fast_controls_HID_report[1]&=(0x00);
      // keyboard->media_fast_controls_HID_report[2]=0;
      // USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t *)keyboard->media_fast_controls_HID_report,KEY_SCAN_HID_REPORT_MEDIA_SIZE);
      // keyboard->media_fast_controls_HID_report[1]=temp;
      keyboard->sys_fast_controls_update_flag=0; 
    }




    if((keyboard->sys_slow_controls_update_flag)){
      //TODO:
      //it seems that the media_controls_HID_report[1] part of the byte that deals with the play, forward, etc, is stuck in the enables state which results in continous forwarding
      //need to find a ways to reset the play, pause forward, backward  bits and send a report instantly.
      
      // temp=keyboard->media_slow_controls_HID_report[1];
      
      
      //possible that then enc_count has not been sent yet
      //modify element as to reset the lowest bit for forward or reverese
      //keyboard->media_controls_HID_report[1] &= ((keyboard->media_controls_HID_report[1] & ((0x0C)))>0x04)*(~(0x04))+((keyboard->media_controls_HID_report[1] & ((0x0C)))<=0x04)*(~(0x08));

      USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t *)keyboard->media_slow_controls_HID_report,KEY_SCAN_HID_REPORT_MEDIA_SIZE);
      HAL_Delay(50);
      keyboard->media_slow_controls_HID_report[1]= (keyboard->media_slow_controls_HID_report[1] & ((0x00)));
      USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t *)keyboard->media_slow_controls_HID_report,KEY_SCAN_HID_REPORT_MEDIA_SIZE);
      //keyboard->media_controls_HID_report[1]  = (keyboard->media_controls_HID_report[1] & (0x03));
      //USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t *)keyboard->media_controls_HID_report,KEY_SCAN_HID_REPORT_MEDIA_SIZE);//direction of volume incremnetation radomly swicthes and button control dontw work reliably
      
      //keyboard->keys_HID_data_report[1]=temp;
      keyboard->sys_slow_controls_update_flag=0;
    }
  /* USER CODE END 3 */
}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
  HAL_TIM_Base_Stop_IT(&led_tim);
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
