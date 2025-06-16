#include "key_scan.h"

const uint8_t keycode_array[7][5] = {{KEY_LEFT, KEY_NONE, KEY_DELETE, KEY_INSERT, KEY_SYSRQ},
{KEY_DOWN, KEY_UP, KEY_END, KEY_HOME, KEY_SCROLLLOCK},
{KEY_RIGHT, KEY_NONE, KEY_PAGEDOWN, KEY_PAGEUP, KEY_PAUSE},
{KEY_KP0, KEY_KP7, KEY_KP4, KEY_KP1, KEY_NUMLOCK},
{KEY_NONE, KEY_KP8, KEY_KP5, KEY_KP2, KEY_KPSLASH},
{KEY_KPDOT, KEY_KP9, KEY_KP6, KEY_KP3, KEY_KPASTERISK},
{KEY_NONE, KEY_KPENTER, KEY_NONE, KEY_KPPLUS, KEY_KPMINUS}};

void scan_keyboard(IO_controls * keyboard_inst){ 
    uint8_t temp = 0; 
    keyboard_inst->highest_element = 0;  
    for(uint8_t clm_offset = 0 ; clm_offset < 7;clm_offset++){
      
    HAL_GPIO_WritePin((GPIO_TypeDef*)(((clm_offset < 3)*((uint32_t)CLM_1__3_PORT)) + ((!(clm_offset < 3))*((uint32_t)CLM_4__7_PORT))), (clm_offset<3)*(CLM_1_Pin<<clm_offset) + (!(clm_offset<3))*(CLM_4_Pin<<(clm_offset-3)), GPIO_PIN_SET);//each next pin is times two mutiple of the previous

    keyboard_inst->keyboard_matrix[clm_offset] = (uint8_t)(((((uint16_t)(ROW_1__3_PORT->IDR) & (uint16_t)0x700)>>6) | (((uint16_t)(ROW_4__5_PORT->IDR) & (uint16_t)0xC000)>>14))&0xFF);//pins are arranged 12345, where 1 is the MSB
           
    CLM_1__3_PORT->BRR = CLM_1_Pin|CLM_2_Pin|CLM_3_Pin;//resets all the CLM output pins
    CLM_4__7_PORT->BRR = CLM_4_Pin|CLM_5_Pin|CLM_6_Pin|CLM_7_Pin;

    for(uint8_t row_offset = 0; row_offset < 5; row_offset++){ 
    temp = keyboard_inst->keyboard_matrix[clm_offset]; 
    keyboard_inst->curr_keycode_arry[clm_offset][row_offset] = (temp>>row_offset & 1)*keycode_array[clm_offset][row_offset];
    if(keyboard_inst->highest_element == 0){
    keyboard_inst->highest_element = (keyboard_inst->curr_keycode_arry[clm_offset][row_offset] != 0)*(&keyboard_inst->curr_keycode_arry[clm_offset][row_offset]-&keyboard_inst->curr_keycode_arry[0][0]);
    }else{
    keyboard_inst->highest_element = keyboard_inst->highest_element + (keyboard_inst->curr_keycode_arry[clm_offset][row_offset] != 0)*((&keyboard_inst->curr_keycode_arry[clm_offset][row_offset]-&keyboard_inst->curr_keycode_arry[0][0])-keyboard_inst->highest_element);  
    }
    }

    }
    
}

void populate_HIDreport(uint8_t * curr_keycode_arry, uint8_t * prev_keycode_arry, uint8_t * HID_data_packet, uint8_t * new_packet_stat,uint8_t array_size){
    
    uint8_t difference = 0;
    static uint8_t prev_array_size;
    prev_array_size = ((array_size >= prev_array_size)&&(*new_packet_stat == 0))*array_size + (!((array_size >= prev_array_size)&&(*new_packet_stat == 0)))*prev_array_size;

    *new_packet_stat = 0;
    *HID_data_packet = 0;//padd the modifier and reserved slot with 0
    HID_data_packet++;
    *HID_data_packet = 0;
    HID_data_packet++;

    for(uint8_t offset = 0; offset < prev_array_size + 1; offset++){

    difference = (*curr_keycode_arry != *prev_keycode_arry);
    *new_packet_stat = *new_packet_stat + (difference == 1 && *new_packet_stat == 0);
    *HID_data_packet = difference * (*curr_keycode_arry);
    HID_data_packet = HID_data_packet + difference;
    *prev_keycode_arry = *curr_keycode_arry;
    curr_keycode_arry++;
    prev_keycode_arry++;
    
    }
 }


//