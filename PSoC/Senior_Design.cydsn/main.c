#include <stdio.h> 
#include <stdlib.h>
#include "project.h"

#define LED_ON              1
#define LED_OFF             0
#define UART_RX_HEADER      0xA0
#define UART_RX_FOOTER      0xFF
// #define UART_TX_OK          200
// #define UART_TX_BAD         0xF0
// #define UART_TX_TIMEOUT     0xF0
// #define UART_TX_FULL        0xFF
#define NUM_LOCATIONS       64
#define UART_RX_BUFFER_SIZE 64
#define P300_SIZE           32
#define P300_DURA_SCAL      2
#define P300_SEPE_SCAL      10
#define P300_START_CRC      218
#define DEBUG_SIZE          70

enum UART_RX_TYPE{
    SSVEP_CLEAR_SET     = 0x01,
    SSVEP_SET_ADDITIVE  = 0x02,
    P300_CLEAR_SET      = 0x05,
    // P300_SET_ADDITIVE   = 0x08,
    P300_START          = 0x16,
    AUX_LED_CLEAR_SET   = 0x33,
};

enum UART_TX_CODE{
    UART_TX_OK          = 0xAA,  // received message is good
    UART_TX_BAD         = 0x55, // received message is bad
    UART_TX_TIMEOUT     = 0x55, // time out for receiving a message
    UART_TX_FULL        = 0xFF, // incoming buffer is full
    UART_TX_P300_DONE   = 0x0D, // p300 iterations done
};

enum OPERATING_MODE{
    STOP  = 0,
    SSVEP = 1,
    P300_READY  = 2,
    P300 = 3,
};

// milliseconds since it begins
uint32 mills = 0;

uint8 operating_mode = 1;

uint32 SSVEP_next_toggle[NUM_LOCATIONS];
uint8  SSVEP_intervals[NUM_LOCATIONS];
uint8  SSVEP_states[NUM_LOCATIONS];
uint8  SSVEP_intervals_next[NUM_LOCATIONS];

uint8  P300_sequence[P300_SIZE]; // 2 locations per byte
uint8  P300_seq_size = 0;
uint8  P300_duration = 50; // unit: 2ms
uint8  P300_seperation = 50; // unit: 10ms
uint8  P300_repetition = 5; 
uint8  P300_iteration = 0;
uint8  P300_state = 0;
uint32 P300_transition = 0; // transition time
uint8  P300_pin_num;

uint8  UART_rx_buffer[UART_RX_BUFFER_SIZE];
uint8  UART_rx_buffer_ind = 0;
uint8  UART_rx_buffer_process_ind = 0;
uint8  UART_rx_buffer_process_ind_reset_flag = 0;
uint8  UART_rx_last_byte_1 = 0;
uint8  UART_rx_last_byte_2 = 0;
uint8  UART_rx_type = 0;
uint8  UART_rx_ch = 0;
uint32 UART_rx_last_time = 0;
uint8  UART_rx_ongoing = 0;

uint8  DEBUG_skip_crc = 0;
uint8  DEBUG_capture_flag = 0;
uint16 DEBUG_capture[DEBUG_SIZE];
uint16 this_time, last_time;
uint8  DEBUG_i = 0;
uint8  DEBUG_no_repeat = 1;

// mapping from pin number to location
uint8  pin_assign_table[64] = { 
    6,      10,     8,      7,      0xFF,   0xFF,   0xFF,   0xFF,
    13,     15,     11,     9,      0xFF,   0xFF,   0xFF,   0xFF,
    1,      5,      12,     4,      0xFF,   0xFF,   0xFF,   0xFF,
    0,      14,     3,      2,      0xFF,   0xFF,   0xFF,   0xFF,
    0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
    0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
    0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
    0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,   0xFF,
};

uint8 CRC8(uint8 *data, uint8 len){
    uint8 generator = 0x07;
    uint8 crc = 0x00; /* start with 0 so first byte can be 'xored' in */
    uint8 offset, byte, i;
    for (offset=0; offset<(len); offset++)
    {
        byte = *(data + offset);
        crc ^= byte; /* XOR-in the next input byte */
        for (i = 0; i < 8; i++)
        {
            if ((crc & 0x80) != 0)
            {
                crc = (uint8)((crc << 1) ^ generator);
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

int check_crc(uint8 msg_len){
    // uint8 msg_good = 0;
    uint8 crc8sum = CRC8( &UART_rx_buffer[0], msg_len );
    char msg[2];
    sprintf(msg, "%02X", crc8sum);
    UART_debug_UartPutString("\r\nCRC8=0x");
    UART_debug_UartPutString(msg);
    UART_debug_UartPutString("\r\n");
    
    // switch for CRC check skip
    if (DEBUG_skip_crc == 0){
        // check CRC
        // if (crc8sum == UART_rx_buffer[msg_len]) msg_good = 1;
        return (crc8sum == UART_rx_buffer[msg_len]);
    }
    else{
        return 1;
        // msg_good = 1;
    }
}

uint8 find_location_number(uint8 row, uint8 col){
    // return the unique location identifier
    // 0 <= output <= 63
    // 0 <= row <= 7
    // 0 <= col <= 7
    return (row*8 + col);
}

void write_pin(uint8 pin_num, uint8 state){
    if (pin_num < NUM_LOCATIONS){ // do look up here
        pin_num = pin_assign_table[pin_num];
    }
    
         if ( pin_num ==  0 ){  Pin_1_Write(state); }
    else if ( pin_num ==  1 ){  Pin_2_Write(state); }
    else if ( pin_num ==  2 ){  Pin_3_Write(state); }
    else if ( pin_num ==  3 ){  Pin_4_Write(state); }
    else if ( pin_num ==  4 ){  Pin_5_Write(state); }
    else if ( pin_num ==  5 ){  Pin_6_Write(state); }
    else if ( pin_num ==  6 ){  Pin_7_Write(state); }
    else if ( pin_num ==  7 ){  Pin_8_Write(state); }
    else if ( pin_num ==  8 ){  Pin_9_Write(state); }
    else if ( pin_num ==  9 ){ Pin_10_Write(state); }
    else if ( pin_num == 10 ){ Pin_11_Write(state); }
    else if ( pin_num == 11 ){ Pin_12_Write(state); }
    else if ( pin_num == 12 ){ Pin_13_Write(state); }
    else if ( pin_num == 13 ){ Pin_14_Write(state); }
    else if ( pin_num == 14 ){ Pin_15_Write(state); }
    else if ( pin_num == 15 ){ Pin_16_Write(state); }   
    else{
        Pin_Dummy_Write(state);
    }
}

void uart_buffer_add_char(uint8 ch){
    UART_rx_buffer[ UART_rx_buffer_ind ] = ch;
    UART_rx_buffer_ind++;
    
    // check overflow
    if (UART_rx_buffer_ind >= UART_RX_BUFFER_SIZE){
        UART_1_UartPutChar(UART_TX_FULL);
        UART_rx_buffer_ind = (UART_RX_BUFFER_SIZE-1);
    }
}

void uart_buffer_clear(){
    uint8 i;
    for (i=0; i<64; i++){
        UART_rx_buffer[i] = 0;
    }
    UART_rx_buffer_ind = 0;
    UART_rx_buffer_process_ind_reset_flag = 1;
}

void debug_tic_toc(){
    if (DEBUG_capture_flag == 1){
        if (DEBUG_i < DEBUG_SIZE){
            Timer_2_Stop();
            DEBUG_capture[DEBUG_i] = Timer_2_ReadCounter();
            DEBUG_i++;
            Timer_2_WriteCounter(0);
            Timer_2_Start();
        }
        else{
            Timer_2_Stop();
            Timer_2_WriteCounter(0);
            Timer_2_Start();
            DEBUG_i = 0;
            DEBUG_capture_flag = 0;
        }
    }
}

void ssvep_buffer_reset(){
    uint8 i; 
    for (i=0; i<NUM_LOCATIONS; i++){
        SSVEP_intervals_next[i] = 0;
    }
}

void ssvep_flip(uint8 pin_num){
    if (SSVEP_states[pin_num] == LED_ON){
        SSVEP_states[pin_num] = LED_OFF;
    }
    else{
        SSVEP_states[pin_num] = LED_ON;
    }
    write_pin(pin_num, SSVEP_states[pin_num]);
}

void uart_rx_buffer_process_set_p300(uint8 slot){
    uint8 val = UART_rx_buffer[slot];
    /*
    char msg[3]; 
    sprintf(msg, "%02X ", val);
    UART_debug_UartPutString(msg);
    */
    if (slot == 1){
        P300_duration = val;
    }
    else if (slot == 2){
        P300_seperation = val;
    }
    else if (slot == 3){
        P300_repetition = val;
    }
    else if (slot == 4){
        P300_seq_size = val;
    }
    else if ( (4 < slot) && (slot < 37) ){
        P300_sequence[ (slot-5) ] = val;
    }
    else if ( slot == 38 ){
        UART_rx_ongoing = 0;
        if (check_crc(slot)){
            operating_mode = P300_READY;
            P300_state = 0;
            P300_transition = mills;
            P300_iteration = 0;
            P300_pin_num = P300_sequence[0]; // find the first pin to turn on
            uint8 i;
            for (i=0; i<NUM_LOCATIONS; i++){
                write_pin(i, LED_OFF);
            }
            UART_1_UartPutChar(UART_TX_OK); // for good message
        }
        else{
            UART_1_UartPutChar(UART_TX_BAD); // for bad message
        }
        if (DEBUG_capture_flag == 3){
            Timer_2_Stop();
            DEBUG_capture[DEBUG_i] = Timer_2_ReadCounter();
            Timer_2_WriteCounter(0);
            Timer_2_Start();
        }
    }
    
}

void uart_rx_buffer_process_start_p300(uint8 slot){
    if (slot == 2){
        UART_rx_ongoing = 0;
        if ( UART_rx_buffer[2] == P300_START_CRC ){
            if ( UART_rx_buffer[1] == 255 ){
                UART_1_UartPutChar(UART_TX_OK); // for good message
                
                if (DEBUG_capture_flag == 2){
                    DEBUG_capture_flag = 1;
                    Timer_2_WriteCounter(0);
                    Timer_2_Start();
                }
                
                P300_transition = mills + 1;
                operating_mode = P300;
                return;
            }
        }
        UART_1_UartPutChar(UART_TX_BAD); // for bad message
        check_crc(slot); // compute and report its CRC
    }
}

void uart_rx_buffer_process_set_ssvep(uint8 buffer_ind_max){
    if ( (8 < buffer_ind_max) && (buffer_ind_max < 37) ){
        uint8 i;
        for (i = 9; i<37; i+=9){ // sweep ind for freq<x> field
            if ( i == buffer_ind_max && UART_rx_buffer[buffer_ind_max]>0 ){
                uint8 interval, row, col; 
                // toggle interval for this mask
                interval = UART_rx_buffer[buffer_ind_max];
                for (row = 0; row < 8; row++){ // sweep each byte (row)
                    uint8 this_byte = UART_rx_buffer[(buffer_ind_max-8+row)];
                    for (col = 0; col < 8; col++){ // sweep each bit (column)
                        uint8 mask = (128 >> col); // mask for selecting a bit
                        if ( (this_byte & mask) > 0 ){ // if this bit is 1
                            uint8 id = find_location_number(row, col);
                            SSVEP_intervals_next[id] = interval;
                        }
                    }
                }
                break;
            }
        }
    }
    else if (buffer_ind_max == 38){
        if (DEBUG_capture_flag == 2){
            DEBUG_capture_flag = 1;
            Timer_2_WriteCounter(0);
            Timer_2_Start();
        }
        if (check_crc(buffer_ind_max)){ // good message, CRC checks out
            debug_tic_toc(); // index = 0
            uint8 i;
            for (i=0; i<NUM_LOCATIONS; i++){
                SSVEP_intervals[i] = SSVEP_intervals_next[i];
                SSVEP_states[i] = LED_OFF;
                SSVEP_next_toggle[i] = mills+1;
                write_pin(i, LED_OFF);
            }
            UART_1_UartPutChar(UART_TX_OK); // for good message
            debug_tic_toc(); // index = 1
            operating_mode = SSVEP;
        }
        else{
            UART_1_UartPutChar(UART_TX_BAD); // for bad message
        }
        UART_rx_ongoing = 0;
        
        if (DEBUG_capture_flag == 3){
            Timer_2_Stop();
            DEBUG_capture[DEBUG_i] = Timer_2_ReadCounter();
            Timer_2_WriteCounter(0);
            Timer_2_Start();
        }
    }
            
}

void uart_rx_buffer_process(uint8 buffer_ind_max){
    
    char msg[3]; 
    sprintf(msg, "%02X ", UART_rx_buffer[buffer_ind_max]);
    UART_debug_UartPutString(msg);
    
    if (buffer_ind_max > 0){ // cached more than type/mode byte
        if (UART_rx_buffer[0] == SSVEP_CLEAR_SET){
            uart_rx_buffer_process_set_ssvep(buffer_ind_max);
        }
        else if (UART_rx_buffer[0] == SSVEP_SET_ADDITIVE){
            uart_rx_buffer_process_set_ssvep(buffer_ind_max);
        }
        else if (UART_rx_buffer[0] == P300_CLEAR_SET){
            uart_rx_buffer_process_set_p300(buffer_ind_max);
        }
        else if (UART_rx_buffer[0] == P300_START){
            /*
            UART_rx_ongoing = 0;
            if (UART_rx_buffer[buffer_ind_max] == 0xB1){
                UART_1_UartPutChar(UART_TX_OK); // for good message
                operating_mode = P300;
            }
            else{
                UART_1_UartPutChar(UART_TX_BAD); // for bad message
                check_crc(buffer_ind_max); // compute and report its CRC
            }
            */
            uart_rx_buffer_process_start_p300(buffer_ind_max);
        }
    }
    else{
        if (UART_rx_buffer[0] == SSVEP_CLEAR_SET) ssvep_buffer_reset();
    }
    
}

void uart_rx_buffer_shift(){
    
    UART_rx_last_byte_2 = UART_rx_last_byte_1;
    
    if ( UART_rx_ch != UART_RX_HEADER && 
        UART_rx_last_byte_1 == UART_RX_HEADER &&
        UART_rx_last_byte_2 == UART_RX_HEADER ){
            UART_rx_last_byte_1 = 1; // avoid reseting with the next byte
        }
    else{
        UART_rx_last_byte_1 = UART_rx_ch;
    }
    
}

CY_ISR(timer_interrupt){
    mills++;
    Timer_ms_ClearInterrupt(Timer_ms_INTR_MASK_TC);
}

CY_ISR(uart_rx_interrupt){    
    UART_rx_ch = UART_1_UartGetChar();
    
    if (UART_rx_ch == UART_RX_HEADER) {
        if (UART_rx_last_byte_1 == UART_RX_HEADER) {
            uart_buffer_add_char(UART_rx_ch);
        }
    }
    else{
        if (UART_rx_last_byte_1 == UART_RX_HEADER && UART_rx_last_byte_2 != UART_RX_HEADER){
            if (DEBUG_capture_flag == 4){
                Timer_2_Stop();
                Timer_2_WriteCounter(0);
                Timer_2_Start();
                DEBUG_capture_flag = 3;
            }
            uart_buffer_clear();
        }
        uart_buffer_add_char(UART_rx_ch);
    }
    
    UART_rx_last_time = mills; // record time for last reception
    UART_rx_ongoing = 1;
    
    uart_rx_buffer_shift();
    
    // clear interrupt
    UART_1_ClearRxInterruptSource( UART_1_GetRxInterruptSourceMasked() );
}

CY_ISR(sw_interrupt){

    // print debug buffer
    uint8 i; 
    LED_R_Write(0);
    char msg[12];
    for (i=0; i<DEBUG_SIZE; i++){
        sprintf(msg, "%02i: %05i\r\n", i, DEBUG_capture[i]);
        UART_debug_UartPutString(msg);
    }
    LED_R_Write(1);
    
    // switch for set CRC check skip (whitespace)
    if (UART_debug_SpiUartGetRxBufferSize() >= 4){
        char uart_debug_rx[4];
        for (i=1; i<4; i++){
            uart_debug_rx[i] = (char)UART_debug_UartGetChar();
        }
        if (strcmp( uart_debug_rx , "crce") == 0){
            DEBUG_skip_crc = 0;
            UART_debug_UartPutString("CRC check enabled\r\n");
        }
        else if (strcmp( uart_debug_rx , "crcd") == 0){
            DEBUG_skip_crc = 1;
            UART_debug_UartPutString("CRC check disabled\r\n");
        }
        else if (strcmp( uart_debug_rx , "nore") == 0){
            DEBUG_no_repeat = 1;
            UART_debug_UartPutString("no repeat for debug capture\r\n");
        }
        else if (strcmp( uart_debug_rx , "repe") == 0){
            DEBUG_no_repeat = 0;
            UART_debug_UartPutString("repeat for debug capture\r\n");
        }
        
        /*
        if ((uint8)UART_debug_UartGetByte() == 32){
            if (DEBUG_skip_crc == 1){
                DEBUG_skip_crc = 0;
                UART_debug_UartPutString("CRC check enabled\r\n");
            }
            else{
                DEBUG_skip_crc = 1;
                UART_debug_UartPutString("CRC check disabled\r\n");
            }
        }
        UART_debug_SpiUartClearRxBuffer();
        */
    }
    
    UART_debug_SpiUartClearRxBuffer();
    
    for (i=0; i<DEBUG_SIZE; i++){
        DEBUG_capture[i]=0;
    }
    
    
    DEBUG_i = 0;
    DEBUG_capture_flag = 4;
    Pin_SW_ClearInterrupt();
}

void main_debug(){
    if (DEBUG_i < DEBUG_SIZE){
        Timer_2_Stop();
        this_time =  Timer_2_ReadCounter();
        if ((DEBUG_no_repeat == 1) &&
           ((this_time - last_time) > 0)){
            DEBUG_capture[DEBUG_i] = this_time;
            DEBUG_i++;
            last_time = this_time;
        }
        else{
            DEBUG_capture[DEBUG_i] = this_time;
            DEBUG_i++;
        }
        Timer_2_WriteCounter(0);
        Timer_2_Start();
    }
    else {
        Timer_2_Stop();
        Timer_2_WriteCounter(0);
        DEBUG_capture_flag = 0;
        DEBUG_i = 0;
        last_time = 0;
    }
}

void init(){
    CyGlobalIntEnable; /* Enable global interrupts. */
    isr_ms_StartEx(timer_interrupt);
    isr_uart_rx_StartEx(uart_rx_interrupt);
    isr_sw_StartEx(sw_interrupt);
    
    /* Start the components */
    Timer_ms_Start();
    UART_1_Start();
    UART_debug_Start();
    Timer_2_Start();
    
    // turn off onboard LED
    LED_R_Write(1);
    LED_G_Write(1);
    LED_B_Write(1);
    
    uint8 i;
    for (i=0; i<NUM_LOCATIONS; i++){
        SSVEP_intervals[i] = 0;
        SSVEP_states[i] = LED_OFF;
        SSVEP_next_toggle[i] = 0;
        write_pin(i, LED_OFF);
        SSVEP_intervals_next[i] = 0;
    }
}

void main_ssvep(){
    uint8 i;
    // sweep each pin
    for (i=0; i < NUM_LOCATIONS; i++){
        
        // if enabled
        if (SSVEP_intervals[i] > 0){
            
            // cache this time instance
            uint32 this_instance = mills;
            
            // if toggle time reached
            if (this_instance > SSVEP_next_toggle[i]){
                
                // set time for next toggle
                SSVEP_next_toggle[i] = SSVEP_next_toggle[i] + SSVEP_intervals[i];
                
                ssvep_flip(i); // toggle
            }
        }
    }
}

void main_p300(){
    if (mills >= P300_transition){ // time for a transition
        if (P300_iteration < P300_repetition){ // required repetitions not reached yet
            if ( (P300_state/2) < P300_seq_size ){
                main_debug();
                if ((P300_state % 2) == 0){ // OFF -> ON transition
                    write_pin(P300_pin_num, LED_ON);
                    P300_transition += (P300_duration * P300_DURA_SCAL);
                }
                else{ // ON -> OFF transition
                    write_pin(P300_pin_num, LED_OFF);
                    P300_pin_num = P300_sequence[(P300_state/2)+1]; // find pin for the next one
                    P300_transition += (P300_seperation * P300_SEPE_SCAL);
                }
                P300_state++;
            }
            
            if ( (P300_state/2) >= P300_seq_size) { // reset for the next round
                P300_state = 0;
                P300_iteration++;
                P300_pin_num = P300_sequence[0];
            }
        }
        
        if( P300_iteration >= P300_repetition ){ // done with all repetitions
            operating_mode = STOP;
        }
    }
}

void main_uart_rx(){
    if (UART_rx_buffer_process_ind_reset_flag > 0){
        UART_rx_buffer_process_ind = 0;
        UART_rx_buffer_process_ind_reset_flag = 0;
    }
    
    // process uart rx buffer based on slot id
    if ( (UART_rx_buffer_ind-1) >= UART_rx_buffer_process_ind){
        uart_rx_buffer_process(UART_rx_buffer_process_ind);
        UART_rx_buffer_process_ind++;
    }
    
    // check for timed out reception
    if (( mills - UART_rx_last_time ) > 1500){
        UART_rx_ongoing = 0;
        UART_1_UartPutChar(UART_TX_TIMEOUT);
        UART_debug_UartPutChar("UART Rx Timed Out\r\n");
    }
}



int main(void)
{
    init();
    
    while(1)
    {
        if (DEBUG_capture_flag == 1){
            main_debug();
        }
        
        if (operating_mode == SSVEP){
            main_ssvep();
        }
        
        if (operating_mode == P300){
            main_p300();
        }
        
        if ( UART_rx_ongoing == 1 ){
            main_uart_rx();
        }
    }
}


