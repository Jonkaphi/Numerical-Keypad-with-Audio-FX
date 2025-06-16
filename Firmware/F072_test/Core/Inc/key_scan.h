#include "main.h"
#include "key_codes_HID.h"

#define CLM_1__3_PORT GPIOA 
#define CLM_4__7_PORT GPIOB 
#define ROW_1__3_PORT GPIOA
#define ROW_4__5_PORT GPIOB
#define key_code_arry_size 35U


typedef struct 
{   
    uint8_t keyboard_matrix[7];
    uint8_t curr_keycode_arry[7][5];
    uint8_t prev_keycode_arry[7][5];
    uint8_t packet_update_flag;
    uint8_t keys_HID_data_report[8];
    uint8_t highest_element;
    volatile uint8_t controls;
    volatile int8_t enc_count;
}IO_controls;

//array is orientated from bottom left to top right, also 1-3 is on the top rather than the bottom



void scan_keyboard(IO_controls * keyboard_inst);
void populate_HIDreport(uint8_t * curr_keycode_arry, uint8_t * prev_keycode_arry, uint8_t * HID_data_packet, uint8_t * new_packet_stat,  uint8_t array_size);

