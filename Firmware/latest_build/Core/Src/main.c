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
  * @attention
  * Application code for the keypad, setting for the enabling and disabling features
  * seen in the compile time if statements can be seen in "main.h" on line 133
  * 
  * Assuming the audio fx are enbaled, normal operation is indicated by the flashing green LED with a period of 1s.
  * 
  * If there are any errors related to the reading files off the SD card or tied hardware peripherals such as the DAC, 
  * SPI and DMA the system will disbale the FX feature and attempt to operate as normal.
  * 
  * This type of error condition is treated as non-critical, so you should be able to still use the keypad and audio controls,
  * but now the LED will flash with a period of 100ms indicating to the user of the error with the audio fx.
  * 
  * Critial erros realted to the initilisation of USB, TIM7 and RCC peripherals will result in a system reset
  *
  * This error handling in seen in function "Error_Handler(Error index)" on line 483 of this file
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
#include "fat_dac_interface.h"
#include "dac.h"
#include "dma.h"
#include "fatfs.h"
#include "spi.h"
#include "FATFS_SD.h"



extern USBD_HandleTypeDef hUsbDeviceFS;

//object and pointer to keypad object
keypad_controls keyboard_inst={0};
keypad_controls_handler keyboard=&keyboard_inst;

#if __LOGGING_ENABLED || __AUDIO_FX_ENABLED
//FS obeject
FATFS fileSystem={0};
//FS error result
FRESULT FR_stat=0;

#if __AUDIO_FX_ENABLED
//object and poiter for streaming of the audio data
fat_dac wav_data_inst = {0};
fat_dac_handler wav_data = &wav_data_inst;

//file obeject for the audio file
FIL audioFile={0};
uint32_t readBytes = 0;

//Name of the audio fx file to be played
//The 8k Hz at 16 bit depth requires less read resulting in a more responsive button presses
const FILE_NAME audiofile_path[10] = "fx_8k.wav";
#endif

#if __LOGGING_ENABLED
//file object for the log file
FIL logFile={0};
uint32_t writtenBytes = 0;
//Name of the log file 
const FILE_NAME logfile_path[10] = "log.txt";
#endif
#endif

//contorl variables for the check, 
//if peripheral hardware failed to init
CONTROL HARDWARE_FAIL=0;

//control var. for check,
//if the SD card is in the slot
CONTROL CARD_PRESENT=0;

//Containt curreently executed state in the FSM
STATE CURR_STATE=KEYBOARD_SCAN;

void SystemClock_Config(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{    
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  #if __DEBUG
  //enables suspension of the APB2 bus during debug
  //!ONLY TO BE USED DURING DEBUGING
  RCC->APB2ENR |= RCC_APB2ENR_DBGMCUEN; 
  #endif
    
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  POWER_OFF_AUDIO_AMP;

  MX_USB_DEVICE_Init();
  MX_LED_TIM_Init();
  

  //IMPORTANT ALWAYS SETUP DMA BEFORE THE DAC
  #if __AUDIO_FX_ENABLED || __LOGGING_ENABLED
  MX_SPI1_Init();
  MX_FATFS_Init();

  #if __AUDIO_FX_ENABLED
  MX_DMA_Init();
  MX_DAC_Init();

  
 
  //Variables for controls of the streaming of the audio
  uint16_t dataOffset = 0;
  uint16_t timer_sample_rate_period=0;
  uint16_t temp_size_check=0;
  uint32_t file_sample_rate=0;
  uint32_t audio_data_size=0;
  #endif
  #endif
  
  HAL_TIM_Base_Start_IT(&led_tim);

  HAL_Delay(500);//Delay is required since the code executes much faster than the enumeration period.
  
  #if __AUDIO_FX_ENABLED || __LOGGING_ENABLED
  //checks if card is in the reader
  if (((HAL_GPIO_ReadPin(CARD_detect_GPIO_Port, CARD_detect_Pin))==GPIO_PIN_SET )&& !HARDWARE_FAIL){
    CARD_PRESENT=1;

    POWER_ON_SDCARD; 
 
    FR_stat = f_mount(&fileSystem, "", 1);
    if(FR_stat!=FR_OK){
      Error_Handler(CANT_MOUNT_CARD);
    }

    
    #if __AUDIO_FX_ENABLED
    FR_stat = f_open(&audioFile, (char*)audiofile_path, FA_READ);
    if(FR_stat!=FR_OK){
      Error_Handler(CANT_OPEN_AUDIO_FILE);
    }
    #endif

    #if __LOGGING_ENABLED
    //Todo: add logging system eveytime there is a error when trying to read the wav file
    // FR_stat = f_open(&logFile, (char*)logfile_path, FA_OPEN_EXISTING|FA_WRITE|FA_READ);
    if(FR_stat!=FR_OK){
      if(FR_stat==FR_NO_FILE){
        FR_stat = f_open(&logFile, (char*)logfile_path, FA_CREATE_ALWAYS|FA_WRITE|FA_READ);
      }else{
        Error_Handler(CANT_OPEN_LOG_FILE);
      }
    }

    //set to start of file
    FR_stat = f_lseek(&logFile, 0);
    if(FR_stat!=FR_OK){
      Error_Handler(CANT_SET_RW_POINTER);
    }
    #endif
    

    //Format of the .wav file header
    //https://blog.fileformat.com/audio/understanding-the-wav-file-header-structure-format-and-how-to-repair/
    //Offset (Bytes)	Field	Size (Bytes)	Description
    // 0	Chunk ID	      4	Should be “RIFF” to indicate the file format.
    // 4	Chunk Size	    4	Size of the file minus 8 bytes for the RIFF and size field.
    // 8	Format	        4	Should be “WAVE”.
    // 12	Subchunk1 ID	  4	“fmt " (includes a trailing space).
    // 16	Subchunk1 Size	4	Size of the format chunk (usually 16 for PCM).
    // 20	Audio Format	  2	Format code (1 for PCM/uncompressed).
    // 22	Number of Channels	2	Mono = 1, Stereo = 2, etc.
    // 24	Sample Rate	    4	Sampling frequency (e.g., 44100 Hz).
    // 28	Byte Rate	      4	SampleRate × NumChannels × BitsPerSample / 8.
    // 32	Block Align	    2	NumChannels × BitsPerSample / 8.
    // 34	Bits per Sample	2	Bit depth (e.g., 16, 24, or 32 bits).
    // 36	Subchunk2 ID	  4	“data” — indicates the beginning of audio data.
    // 40	Subchunk2 Size	4	Number of bytes in the data section.
    
    #if __AUDIO_FX_ENABLED
    //grabs the header of the file given the sector size is 512, min data request is equal to the sector
    FR_stat = f_read(&audioFile, wav_data->temp_wav_buff, SECTOR_SIZE, (UINT*)&readBytes);
    if(FR_stat!=FR_OK){
      CARD_PRESENT=0;
      Error_Handler(CANT_READ_AUDIO_FILE);
    }
    if(wav_data->temp_wav_buff[34] != 16U){
      //wrong bit depth
      CARD_PRESENT=0;
      #if __LOGGING_ENABLED
      //f_puts("Err:FFD16\r\n",&logFile);
      f_puts("Wrong bit depth not 16 bit!\r\n",&logFile);
      #endif
      Error_Handler(NOT_16BIT_DEPTH);
      
    }

    file_sample_rate=(((uint32_t)wav_data->temp_wav_buff[27])<<24)|(((uint32_t)wav_data->temp_wav_buff[26])<<16)|
    (((uint32_t)wav_data->temp_wav_buff[25])<<8)|((uint32_t)(wav_data->temp_wav_buff[24]));

    //only able to play wav files are sample rates of power of two between 8 to 16 kHz
    if (file_sample_rate%2!=0&&(file_sample_rate>16000U||file_sample_rate<8000U)){
      CARD_PRESENT=0;
      
      #if __LOGGING_ENABLED
      //f_puts("Err:FFB16\r\n",&logFile);
      f_puts("Wrong bit rate not 16kHz!\r\n",&logFile);
      #endif
      Error_Handler(WRONG_BIT_RATE);
    }

    if(CARD_PRESENT){
      timer_sample_rate_period=GET_TIMER_PERIOD(file_sample_rate);
      //configures the timer period to the sample rate of the .wav file
      MX_WAV_SAMPLE_RATE_TIM_Init(DEFAULT_PRESCALER,timer_sample_rate_period);
      
      for (uint16_t i = 0; i < (WAV_BUF_SIZE - 3); i++)
      {    
        if ((wav_data->temp_wav_buff[i] == 'd') && (wav_data->temp_wav_buff[i + 1] == 'a') &&
            (wav_data->temp_wav_buff[i + 2] == 't') && (wav_data->temp_wav_buff[i + 3] == 'a'))
        { 
          //sests offset value to the start of actuall audio data 
          dataOffset = i + 8;
          break;
        }
      }

      audio_data_size = (((uint32_t)wav_data->temp_wav_buff[43])<<24)|(((uint32_t)wav_data->temp_wav_buff[42])<<16)|
      (((uint32_t)wav_data->temp_wav_buff[41])<<8)|((uint32_t)(wav_data->temp_wav_buff[40]));

      wav_data->wavDataSize=audio_data_size;
      wav_data->bytes_to_read=wav_data->wavDataSize;
     
      POWER_ON_AUDIO_AMP;
       
    } 
    #endif
  }
  #endif
  
  while (1)
  {
    /* USER CODE END WHILE */
   switch(CURR_STATE){
  
    case KEYBOARD_SCAN:
      //currelty pooling is a bit slow since the sequtial exection of the scanning functions,
      //should make the report fucntion be called by the timmer ever 1 ms.
      #if __AUDIO_FX_ENABLED
      wav_data->PLAY_WAV_STATE_FLAG=(wav_data->PLAY_WAV_STATE_FLAG==0&&wav_data->DMA_START==0)*check_keypad_controls(keyboard)+!((wav_data->PLAY_WAV_STATE_FLAG==0&&wav_data->DMA_START==0))*(wav_data->PLAY_WAV_STATE_FLAG);
      #else
      check_keypad_controls(keyboard);
      #endif

      #if __AUDIO_FX_ENABLED
      if(CARD_PRESENT && (!HARDWARE_FAIL)){
        if(keyboard->MUTE_FX){
          
          POWER_OFF_AUDIO_AMP;
          wav_data->AUDIO_FX_ENB_FLAG=0;
          wav_data->PLAY_WAV_STATE_FLAG=0;
        }else{
          
          POWER_ON_AUDIO_AMP;
          wav_data->AUDIO_FX_ENB_FLAG=1;
        }
      }
      #endif

      populate_HID_keycode_report(keyboard);

      populat_HID_fast_media_controls_report(keyboard);

      if((((uint32_t)HAL_GetTick()-keyboard->button_press_tick_check)>=play_button_timeout_ms)&&(keyboard->populate_media_slow_controls_flag==1)){
        populat_HID_slow_media_controls_report(keyboard);
      }

      if(keyboard->sys_fast_controls_update_flag||keyboard->sys_slow_controls_update_flag||keyboard->packet_update_flag){
        CURR_STATE = SEND_HID_REPORTS;
        break;
      }

      #if __AUDIO_FX_ENABLED
      if((((wav_data->bytes_read >= WAV_BUF_SIZE)&& (wav_data->bytes_read>0 ))||(wav_data->bytes_to_read==0&&!wav_data->PLAY_WAV_STATE_FLAG))
      && (CARD_PRESENT && wav_data->AUDIO_FX_ENB_FLAG))
      { 
    
        wav_data->bytes_read=(WAV_BUF_SIZE<wav_data->bytes_read)?0:wav_data->bytes_read;

        if(!wav_data->wavDataSize&&!wav_data->PLAY_WAV_STATE_FLAG){
        
          //set file Read pointer to the start of the audio data
          FR_stat = f_lseek(&audioFile, dataOffset);
          if(FR_stat!=FR_OK){

            #if __LOGGING_ENABLED
            //f_puts("Err:FFSEK\r\n",&logFile);
            f_puts("Could not RW pointer of audio file",&logFile);
            #endif
            CARD_PRESENT=0;
            Error_Handler(CANT_SET_RW_POINTER);
          }

          wav_data->wavDataSize=audio_data_size;
          wav_data->bytes_to_read=audio_data_size;
          wav_data->requested_data=TEMP_WAV_BUF_SIZE;
          
          for(wav_data->curBufIdx=0; wav_data->curBufIdx<2; wav_data->curBufIdx++){

            wav_data->requested_data=TEMP_WAV_BUF_SIZE;
            for(uint8_t i=9;i<12;i++){
                temp_size_check=(1<<i);
                if(wav_data->wavDataSize<temp_size_check){
                  wav_data->requested_data=temp_size_check;
                  break;
                }
              }
            
            for(uint16_t i=0; i<TEMP_WAV_BUF_SIZE;i+=2){
              wav_data->wavBuf[wav_data->curBufIdx][i/2] = ((uint16_t)((((uint16_t)wav_data->temp_wav_buff[i + 1]) << 8) | ((uint16_t)wav_data->temp_wav_buff[i])) + 32767);
              wav_data->wavBuf[wav_data->curBufIdx][i/2] = (wav_data->wavBuf[wav_data->curBufIdx][i/2]*8)/(16*6);
              
            }
          }

          wav_data->bytes_read=0;
          break;
        }

        wav_data->requested_data=TEMP_WAV_BUF_SIZE;

        for(uint8_t i=9;i<12;i++){
          temp_size_check=(1<<i);
          if(wav_data->wavDataSize<temp_size_check){
            wav_data->requested_data=temp_size_check;
            break;
          }
        }

        FR_stat = f_read(&audioFile, wav_data->temp_wav_buff, wav_data->requested_data, (UINT*)&readBytes);
        if(FR_stat!=FR_OK){
          #if __LOGGING_ENABLED
          //f_puts("Err:FFNR\r\n",&logFile);
          f_puts("Could not read file!\r\n",&logFile);
          #endif
          CARD_PRESENT=0;
          Error_Handler(CANT_READ_AUDIO_FILE);
        }
        
        if(wav_data->wavDataSize<SECTOR_SIZE){
          wav_data->wavDataSize=0;
        }else{
          wav_data->wavDataSize-=readBytes;
        }
        
        wav_data->curBufIdx=(wav_data->curBufIdx==0)?1U:0U;
        for(uint16_t i=0; i<readBytes;i+=2)
        {
          wav_data->wavBuf[wav_data->curBufIdx][i/2] = ((uint16_t)((((uint16_t)wav_data->temp_wav_buff[i + 1]) << 8) | ((uint16_t)wav_data->temp_wav_buff[i])) + 32767);
          wav_data->wavBuf[wav_data->curBufIdx][i/2] /= 16;
        }
      }
      #endif  
      
      break;

    case SEND_HID_REPORTS:
      
      //could make two sepperate report be send one for the mute and encoder, and a sepperate for the play pause forwars, backward
      //edit: yes that helped, but it sill is ver much needed to make the sending of the reports in a DMA format.
      if((keyboard->sys_fast_controls_update_flag)){
        USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t *)keyboard->media_fast_controls_HID_report,KEY_SCAN_HID_REPORT_MEDIA_SIZE);//direction of volume incremnetation radomly swicthes and button control dontw work reliably
        keyboard->sys_fast_controls_update_flag=0; 
      }

      if((keyboard->sys_slow_controls_update_flag)){
        USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t *)keyboard->media_slow_controls_HID_report,KEY_SCAN_HID_REPORT_MEDIA_SIZE);
        HAL_Delay(50);
        keyboard->media_slow_controls_HID_report[1]= (keyboard->media_slow_controls_HID_report[1] & ((0x00)));
        USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t *)keyboard->media_slow_controls_HID_report,KEY_SCAN_HID_REPORT_MEDIA_SIZE);
        keyboard->sys_slow_controls_update_flag=0;
      }

      if(keyboard->packet_update_flag){
        USBD_HID_SendReport(&hUsbDeviceFS,(uint8_t *)keyboard->keys_HID_data_report, KEY_SCAN_HID_REPORT_KEYS_SIZE );
        keyboard->packet_update_flag=0;

        #if __AUDIO_FX_ENABLED
        if(CARD_PRESENT && wav_data->AUDIO_FX_ENB_FLAG){
          if(wav_data->PLAY_WAV_STATE_FLAG && (wav_data->bytes_to_read==audio_data_size) && wav_data->DMA_START==0){
            HAL_DAC_Start_DMA(&hdac, DAC_FX_OUT, (uint32_t *)wav_data->wavBuf[0],WAV_BUF_SIZE*2, DAC_ALIGN_12B_R );
            HAL_TIM_Base_Start_IT(&DAC_sample_rate_tim);
          
            wav_data->DMA_START=1;
          }
        }
        #endif
      }

    CURR_STATE=KEYBOARD_SCAN;  
    break;

    default:
    #if __LOGGING_ENABLED
    //f_puts("Err:SF00\r\n",&logFile);
    f_puts("Unrecognised state!\r\n",&logFile);
    #endif

    Error_Handler(UKNOWN_STATE);
    break;
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
    Error_Handler(RCC_OSC_FAIL);
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
    Error_Handler(RCC_CLK_FAIL);
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler(RCC_CLK_PERIPH_FAIL);
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

//TODO:
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(Error index)
{
  
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  if(index!=NO_ERROR){
    __disable_irq();

    CONTROL RESET_FLAG=0;

     #if __AUDIO_FX_ENABLED
     //if the card cant be mounte we disable the audio fx,
     //and led blinks faster and try to opperate the system with them disabled.
      if((index>=CANT_MOUNT_CARD &&
        index<=CANT_READ_AUDIO_FILE)){

        wav_data_inst.AUDIO_FX_ENB_FLAG=0;
        wav_data_inst.CARD_PRESENT=0;

        if(led_tim.Instance!=0){
          HAL_TIM_Base_Stop_IT(&led_tim);
          //change to a faster blink period 
          led_tim.Init.Prescaler = 767;
          led_tim.Init.Period = 6249;
          
          //if status led cannot be re-initilize with the new period we reset
          if (HAL_TIM_Base_Init(&led_tim) != HAL_OK)
          {
            RESET_FLAG=1;
          }
          HAL_TIM_Base_Start_IT(&led_tim);
        }

        if(hdac.Instance!=0){
          HAL_DAC_Stop_DMA(&hdac, DAC_FX_OUT);
        }

        if(DAC_sample_rate_tim.Instance!=0){
          HAL_TIM_Base_Stop_IT(&DAC_sample_rate_tim);
        }
      
      }
    

    //if the dac and spi hardware cant init
    if(index>=DAC_INIT_FAIL && 
      index<=SPI_INIT_FAIL){
        HARDWARE_FAIL=1;
    }

    #else
    #if __LOGGING_ENABLED
    if(index==SPI_INIT_FAIL){
        
    }
    #endif
    #endif
    
    //if the clk tree or led status cant initilise
    if(index>=RCC_OSC_FAIL && 
      index<=LED_TIM_CONFIG_FAIL){
        RESET_FLAG=1;
    }

    if(RESET_FLAG){
      NVIC_SystemReset();
    }
    __enable_irq();
  
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
