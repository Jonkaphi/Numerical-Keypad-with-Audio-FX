/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    key_scan.h
  * @brief   This file contains the function prototypes and struct definition
  *           for the scannig of the key pad controls, including the key matrix 
  *           and the volume and play/pause controls
  *          
  * @author  Yoan Philipov
  ******************************************************************************
  * @attention
  *
  * 
  ******************************************************************************
  */
/* USER CODE END Header */
#ifndef KEY_SCAN_H
#define KEY_SCAN_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "key_codes_HID.h"
#include "main.h"


#define CLM_1__3_PORT GPIOA 
#define CLM_4__7_PORT GPIOB 
#define ROW_1__3_PORT GPIOA
#define ROW_4__5_PORT GPIOB
#define key_code_arry_size 35U
#define KEY_SCAN_HID_REPORT_KEYS_SIZE 11U
#define KEY_SCAN_HID_REPORT_MEDIA_SIZE 3U
#define colum_size 7U
#define row_size 5U

#define play_button_timeout_ms 500UL

//these codes are not used, this is a left over from an old method to implement the media controls
// USB media codes
// #define USB_HID_SCAN_NEXT 0x01
// #define USB_HID_SCAN_PREV 0x02
// #define USB_HID_STOP      0x04
// #define USB_HID_EJECT     0x08
// #define USB_HID_PAUSE     0x10
// #define USB_HID_MUTE      0x20
// #define USB_HID_VOL_UP    0x40
// #define USB_HID_VOL_DEC   0x80


typedef struct 
{   
    //array holding the raw data for each key press
    uint8_t keyboard_matrix[colum_size];
    uint8_t curr_keycode_arry[colum_size][row_size];
    uint8_t prev_keycode_arry[colum_size][row_size];

    //flags to indicate a contros have been pressed
    uint8_t packet_update_flag;
    uint8_t sys_fast_controls_update_flag;
    uint8_t sys_slow_controls_update_flag;

    //Arrays for the HID reports
    uint8_t keys_HID_data_report[KEY_SCAN_HID_REPORT_KEYS_SIZE];
    int8_t media_fast_controls_HID_report[KEY_SCAN_HID_REPORT_MEDIA_SIZE];
    int8_t media_slow_controls_HID_report[KEY_SCAN_HID_REPORT_MEDIA_SIZE];//Report ID, MMKey.
    uint8_t highest_element;

    
    CONTROL MUTE_FX;
    
    //Variables for the interrupt driven controls (volume controls and play/pause button)
    volatile uint8_t controls;
    volatile int8_t enc_count;
    volatile int8_t enc_fall_edges;
    volatile uint32_t button_press_tick_check;
    volatile uint8_t populate_media_slow_controls_flag;

}keypad_controls;

typedef keypad_controls* keypad_controls_handler;

//array is orientated from bottom left to top right, also 1-3 is on the top rather than the bottom

static CONTROL scan_sweep_key_matrix (keypad_controls_handler keyboard_inst);
/** 
  *  @brief Performs the pooling of the colum and row pins on the keyboard to see whcih keywas pressed 
  * @param keyboard_inst pointer to the keypad_controls struct, modifies the curr and prev arrays
    * @retval play_wav - indicates if a button is only pressed down as to control when to play the wav
*/
CONTROL check_keypad_controls(keypad_controls_handler keyboard_inst);

/** 
  *  @brief Populates the HID report with the keycodes which will be send to th PC
  * @param keyboard_inst pointer to the keypad_controls struct, modifies the curr and prev arrays
    * @retval none
*/
void populate_HID_keycode_report(keypad_controls_handler keyboard_inst);//uint8_t * curr_keycode_arry, uint8_t * prev_keycode_arry, uint8_t * HID_data_packet, uint8_t * new_packet_stat,  uint8_t array_size);

/** 
  *  @brief Populates the HID report with the keycodes for playback controls (mute, value) that are not timming dependent 
  * @param keyboard_inst pointer to the keypad_controls struct, modifies the curr and prev arrays
    * @retval none
*/
void populat_HID_fast_media_controls_report(keypad_controls_handler keyboard_inst);//[volatile uint8_t * bttn_controls, volatile int8_t * encoder_val, int8_t * HID_data_packet, uint8_t * new_packet_state){

/** 
  *  @brief Populates the HID report with the keycodes for playback controls (forward track, backward strack) that are timming dependent 
  * @param keyboard_inst pointer to the keypad_controls struct, modifies the curr and prev arrays
    * @retval none
*/
void populat_HID_slow_media_controls_report(keypad_controls_handler keyboard_inst);//[volatile uint8_t * bttn_controls, volatile int8_t * encoder_val, int8_t * HID_data_packet, uint8_t * new_packet_state){
 
#ifdef __cplusplus
}
#endif

#endif // KEY_SCAN_H