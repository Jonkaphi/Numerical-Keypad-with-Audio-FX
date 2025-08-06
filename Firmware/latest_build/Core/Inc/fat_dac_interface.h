#ifndef __FAT_DAC_INT_H
#define __FAT_DAC_INT_H

#include "main.h"
#define FACTOR 10U
#define TEMP_WAV_BUF_SIZE (1U<<((FACTOR)+1U))//buffer size 2048
#define WAV_BUF_SIZE (1U<<(FACTOR))
#define SECTOR_SIZE 512U

typedef struct 
{
  //Control variable for filling the buffers
  uint8_t curBufIdx;
  uint8_t temp_wav_buff[TEMP_WAV_BUF_SIZE];
  uint16_t wavBuf[2][WAV_BUF_SIZE];
  uint32_t wavDataSize;
  uint32_t requested_data;

  //Flags for control of the AUDIO Streaming
  STATE PLAY_WAV_STATE_FLAG;
  STATE DMA_START;
  //STATE CARD_PRESENT;
  STATE AUDIO_FX_ENB_FLAG;

  //Used fro traing the number of bytes read by the dac
  volatile uint32_t bytes_read;
  volatile uint32_t bytes_to_read;

}fat_dac;

typedef fat_dac* fat_dac_handler;

#endif // __FAT_DAC_INT_H
