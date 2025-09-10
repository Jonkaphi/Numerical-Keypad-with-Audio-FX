/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    key_scan.c
  * @brief   This file provides code for the scanning and formating
  *            of the HID reports for the key matrix , audio and play/pause button controls
  ******************************************************************************
  */
/* USER CODE END Header */
#include "key_scan.h"
#include "stm32f0xx_hal.h"
#include "main.h"
#include "stm32f0xx_hal_def.h"

const uint8_t keycode_array[7][5] = {{KEY_LEFT, KEY_NONE, KEY_DELETE, KEY_INSERT, KEY_SYSRQ},
{KEY_DOWN, KEY_UP, KEY_END, KEY_HOME, KEY_SCROLLLOCK},
{KEY_RIGHT, KEY_NONE, KEY_PAGEDOWN, KEY_PAGEUP, KEY_PAUSE},
{KEY_KP0, KEY_KP7, KEY_KP4, KEY_KP1, KEY_NUMLOCK},
{KEY_NONE, KEY_KP8, KEY_KP5, KEY_KP2, KEY_KPSLASH},
{KEY_KPDOT, KEY_KP9, KEY_KP6, KEY_KP3, KEY_KPASTERISK},
{KEY_NONE, KEY_KPENTER, KEY_NONE, KEY_KPPLUS, KEY_KPMINUS}};


static CONTROL scan_sweep_key_matrix (keypad_controls_handler keyboard_inst){
    uint8_t temp = 0;
    CONTROL play_wav=0;
    for(uint8_t clm_offset = 0 ; clm_offset < 7;clm_offset++){
      
        HAL_GPIO_WritePin((GPIO_TypeDef*)(((clm_offset < 3)*((uint32_t)CLM_1__3_PORT)) + ((!(clm_offset < 3))*((uint32_t)CLM_4__7_PORT))), (clm_offset<3)*(CLM_1_Pin<<clm_offset) + (!(clm_offset<3))*(CLM_4_Pin<<(clm_offset-3)), GPIO_PIN_SET);//each next pin is times two mutiple of the previous

        keyboard_inst->keyboard_matrix[clm_offset] = (uint8_t)(((((uint16_t)(ROW_1__3_PORT->IDR) & (uint16_t)0x700)>>6) | (((uint16_t)(ROW_4__5_PORT->IDR) & (uint16_t)0xC000)>>14))&0xFF);//pins are arranged 12345, where 1 is the MSB
           
        CLM_1__3_PORT->BRR = CLM_1_Pin|CLM_2_Pin|CLM_3_Pin;//resets all the CLM output pins
        CLM_4__7_PORT->BRR = CLM_4_Pin|CLM_5_Pin|CLM_6_Pin|CLM_7_Pin;

        for(uint8_t row_offset = 0; row_offset < 5; row_offset++){ 

            temp = keyboard_inst->keyboard_matrix[clm_offset]; 
            keyboard_inst->curr_keycode_arry[clm_offset][row_offset] = (temp>>row_offset & 1)*keycode_array[clm_offset][row_offset];
            if((keyboard_inst->curr_keycode_arry[clm_offset][row_offset] > 0) && (keyboard_inst->prev_keycode_arry[clm_offset][row_offset] == 0)){
                play_wav=1;
            }
            if(keyboard_inst->highest_element == 0){
                keyboard_inst->highest_element = (keyboard_inst->curr_keycode_arry[clm_offset][row_offset] != 0)*(&keyboard_inst->curr_keycode_arry[clm_offset][row_offset]-&keyboard_inst->curr_keycode_arry[0][0]);
            }else{
                keyboard_inst->highest_element = keyboard_inst->highest_element + (keyboard_inst->curr_keycode_arry[clm_offset][row_offset] != 0)*((&keyboard_inst->curr_keycode_arry[clm_offset][row_offset]-&keyboard_inst->curr_keycode_arry[0][0])-keyboard_inst->highest_element);  
            }
        }

    }
    return play_wav;

}

/** 
  *  @brief Performs the pooling of the colum and row pins on the keyboard to see whcih keywas pressed 
  * @param keyboard_inst pointer to the keypad_controls struct, modifies the curr and prev arrays
    * @retval play_wav - indicates if a button is only pressed down as to control when to play the wav
*/
CONTROL check_keypad_controls(keypad_controls_handler keyboard_inst){ 
    
    
    static TIME last_toggle;
    CONTROL play_wav=0; 


    keyboard_inst->highest_element = 0;

    play_wav=scan_sweep_key_matrix(keyboard_inst);

    if(((keyboard_inst->curr_keycode_arry[6][4] > 0) && (keyboard_inst->curr_keycode_arry[5][0] > 0))&&(((TIME)HAL_GetTick()-last_toggle)>500)){
        keyboard_inst->MUTE_FX=(keyboard_inst->MUTE_FX==0)?1:0;
        last_toggle=(TIME)HAL_GetTick();
        play_wav=0;
    }
    return play_wav;
    
}



/** 
  *  @brief Populates the HID report which will be send to th PC
  * @param keyboard_inst pointer to the keypad_controls struct, modifies the curr and prev arrays
    * @retval none
*/
void populate_HID_keycode_report(keypad_controls_handler keyboard_inst){//(uint8_t * curr_keycode_arry, uint8_t * prev_keycode_arry, uint8_t * HID_data_packet, uint8_t * new_packet_stat,uint8_t array_size){
    
    //difference here is not only a controll variables it is also a array offset very bad
    
    uint8_t difference = 0;
    static uint8_t prev_array_size;
    uint8_t HID_offset=0;
    //1-D pointer to 2-D array
    uint8_t *ptr_curr_arry = &keyboard_inst->curr_keycode_arry[0][0];
    uint8_t *ptr_prev_arry = &keyboard_inst->prev_keycode_arry[0][0];
   

    //this is very closly tied to the HID_REPORT_SIZE, this lines is a bit redundant since the passed size is static, thus prev_array_size is always equal to passed size
    prev_array_size = ((keyboard_inst->highest_element >= prev_array_size)&&(keyboard_inst->packet_update_flag == 0))*keyboard_inst->highest_element+ (!((keyboard_inst->highest_element >= prev_array_size)&&(keyboard_inst->packet_update_flag == 0)))*prev_array_size;
    
    if(keyboard_inst->highest_element >(colum_size*row_size)){
        //breakpoint due this being an error condition
        prev_array_size=colum_size*row_size;
        
    }

    keyboard_inst->packet_update_flag = 0;
    keyboard_inst->keys_HID_data_report[0] = 1;//padd the modifier and reserved slot with 0
    keyboard_inst->keys_HID_data_report[1] = 0;
    keyboard_inst->keys_HID_data_report[2] = 0;

    for(uint8_t offset = 0; offset < prev_array_size + 1 ; offset++){
        
        //since the hid report needs to be only updated to changes to the keys, 
        //e.g. give the keycode for the "5" key when pressed and continiously done so, we reove the keycode when the key is no longer pressed
        //we use highest element variable and we do multiples of the colums and rows as to use them as 2d arrays, thus access them in a more well defined behaviour

        //linarly accesing the 2d array since its easier with the way we are checking for differences in the pressed buttons
        
        //look for a difference in the pressed buttons, if a change is seen we update the HID report with that change
        //if a new button is pressed we add the keycode, if it is depresed the keycode is removed from the report

        difference = (*(ptr_curr_arry+offset)) != (*(ptr_prev_arry+offset));
        keyboard_inst->packet_update_flag += (difference == 1 && keyboard_inst->packet_update_flag == 0);

        //need to make sure when accesing thge HID report offset is limted to 8 elements
        keyboard_inst->keys_HID_data_report[HID_offset+3] = difference * (*(ptr_curr_arry+offset));
        (*(ptr_prev_arry+offset)) = (*(ptr_curr_arry+offset));
        HID_offset +=difference;
        
        //Terminates population function if more tha 8 keys have been pressed, anyhting more will not be reported by the device
        if(HID_offset>7){
            break;
        }
    }
    
 }

 /** 
  *  @brief Populates the HID report with the keycodes for playback controls (volume, play,pause, mute) will be send to th PC
  * @param keyboard_inst pointer to the keypad_controls struct, modifies the curr and prev arrays
    * @retval none
*/
void populat_HID_fast_media_controls_report(keypad_controls_handler keyboard_inst){//[volatile uint8_t * bttn_controls, volatile int8_t * encoder_val, int8_t * HID_data_packet, uint8_t * new_packet_state){
    
    

    keyboard_inst->sys_fast_controls_update_flag = 0;
    uint8_t difference = (0 != (keyboard_inst->controls&0x01)) || (keyboard_inst->enc_count != 0);
    

    

    //report id, following the collection page on line 320 in usbd_hid.c file in middleware folder
    keyboard_inst->media_fast_controls_HID_report[0] = 2;
   
    keyboard_inst->sys_fast_controls_update_flag += difference;
    


    keyboard_inst->media_fast_controls_HID_report[1] = (keyboard_inst->controls&0x01);

    keyboard_inst->media_fast_controls_HID_report[2] = keyboard_inst->enc_count;

    
    keyboard_inst->controls = (keyboard_inst->controls&~(0x01));
    keyboard_inst->enc_count = 0;

   
}

void populat_HID_slow_media_controls_report(keypad_controls_handler keyboard_inst){//[volatile uint8_t * bttn_controls, volatile int8_t * encoder_val, int8_t * HID_data_packet, uint8_t * new_packet_state){
    
        
    keyboard_inst->sys_slow_controls_update_flag = 0;
    
    uint8_t slow_difference = ((keyboard_inst->controls&(~(0x01))) != 0);
   
    //report id, following the collection page on line 320 in usbd_hid.c file in middleware folder
    keyboard_inst->media_slow_controls_HID_report[0] = 2;
   
    
    keyboard_inst->sys_slow_controls_update_flag+=slow_difference;


    //should mask the bit based on position if bit 4 is one mask the rest
    //sort though the bits and find the MSB
    keyboard_inst->media_slow_controls_HID_report[1]=0;
    keyboard_inst->media_slow_controls_HID_report[1]+= (((keyboard_inst->controls&(0x08))==0x08)&&(keyboard_inst->media_slow_controls_HID_report[1]==0))*0x08;
    keyboard_inst->media_slow_controls_HID_report[1]+= (((keyboard_inst->controls&(0x04))==0x04)&&(keyboard_inst->media_slow_controls_HID_report[1]==0))*0x04;
    keyboard_inst->media_slow_controls_HID_report[1]+= (((keyboard_inst->controls&(0x02))==0x02)&&(keyboard_inst->media_slow_controls_HID_report[1]==0))*0x02;
    


    keyboard_inst->media_slow_controls_HID_report[2]=0;
    //mask the forward and backward functionality
    keyboard_inst->controls = (keyboard_inst->controls&(0x01));

    //clear flag to stop the population fucntion
    keyboard_inst->populate_media_slow_controls_flag=0;

    
}
//