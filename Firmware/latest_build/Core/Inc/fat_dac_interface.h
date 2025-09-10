/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    fat_dac_interface.h
  * @brief   This file contains the private defines and struct def. used for storing the .wav audio data 
  *           and controls of the audio fx streaming. 
  *    
  * @author  Yoan Philipov
  ******************************************************************************
  * @attention
  *
  * 
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef __FAT_DAC_INT_H
#define __FAT_DAC_INT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "main.h"
//Settings: Selects the buffer size for the wav data
//NOTE: the bigger the buffer the less reads needed,
#define FACTOR 10U

//size for the byte array from the wav file
#define TEMP_WAV_BUF_SIZE (1U<<((FACTOR)+1U))//buffer size 2048
//size for the 16 bit array used to feed data to the DAC
#define WAV_BUF_SIZE (1U<<(FACTOR))
#define SECTOR_SIZE 512U

typedef struct 
{
  //Control variable for filling the buffers
  
  uint8_t temp_wav_buff[TEMP_WAV_BUF_SIZE];
  uint16_t wavBuf[2][WAV_BUF_SIZE];
  uint32_t wavDataSize;
  uint32_t requested_data;
  uint8_t curBufIdx;

  //Flags for control of the AUDIO Streaming
  CONTROL PLAY_WAV_STATE_FLAG;
  CONTROL DMA_START;
  CONTROL AUDIO_FX_ENB_FLAG;
  CONTROL CARD_PRESENT;
  

  //Used for controling the number of bytes read by the dac
  volatile uint32_t bytes_read;
  volatile uint32_t bytes_to_read;

}fat_dac;

typedef fat_dac* fat_dac_handler;

#ifdef __cplusplus
}
#endif

#endif // __FAT_DAC_INT_H
