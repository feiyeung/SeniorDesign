#include "project.h"

#define LED_ON 0
#define LED_OFF 1
#define UART_RX_HEADER 0xA0
#define UART_RX_FOOTER 0xFF
#define NUM_LOCATIONS 64

enum UART_RX_TYPE{
    SET_SSVEP  = 0x01,
    SET_P300   = 0x02,
    P300_START = 0x04,
};

enum OPERATING_MODE{
    STOP  = 0,
    SSVEP = 1,
    P300  = 2,
};

// milliseconds since it begins
uint32 mills = 0;

uint8 operating_mode = 1;

uint32 SSVEP_next_toggle[NUM_LOCATIONS];
uint8  SSVEP_intervals[NUM_LOCATIONS];
uint8  SSVEP_states[NUM_LOCATIONS];
uint8  SSVEP_intervals_next[NUM_LOCATIONS];

uint8  UART_rx_buffer[64];
uint8  UART_rx_buffer_ind = 0;
uint8  UART_rx_buffer_process_ind = 0;
uint8  UART_rx_buffer_process_ind_reset_flag = 0;
uint8  UART_rx_last_byte = 0;
uint8  UART_rx_type = 0;

uint16 sweep_time[100];
uint8 sweep_i = 0;
uint32 tic, toc;
uint16 mills_time[100];
uint8 mills_time_i = 0;


//CRC-8 - based on the CRC8 formulas by Dallas/Maxim
//code released under the therms of the GNU GPL 3.0 license
uint8 CRC8(uint8 *data, uint8 len) {
  uint8 crc = 0x00;
  uint8 offset;
  for (offset=0; offset<len; offset++){
  // while (len--) {
    uint8 extract = *(data + offset);
    uint8 tempI;
    for (tempI = 8; tempI; tempI--) {
      uint8 sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}

void write_pin(uint8 pin_num, uint8 state){
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
}

void uart_buffer_add_char(uint8 ch){
    UART_rx_buffer[ UART_rx_buffer_ind ] = ch;
    UART_rx_buffer_ind++;
    
    // check overflow
    if (UART_rx_buffer_ind >= 64){
        UART_1_UartPutString("rx buffer full\n\r");
        UART_rx_buffer_ind = 63;
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

// return the unique location identifier
// 0 <= output <= 63
// 0 <= row <= 7
// 0 <= col <= 7
uint8 find_location_number(uint8 row, uint8 col){
    return (row*8 + col);
}

void uart_rx_buffer_process_set_ssvep(uint8 buffer_ind_max){
    if ( (8 < buffer_ind_max) && (buffer_ind_max < 37) ){
        uint8 i;
        for (i = 9; i<37; i+=9){ // sweep ind for freq<x> field
            if ( i == buffer_ind_max ){
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
    else if (buffer_ind_max == 37){
        if (UART_rx_buffer[buffer_ind_max] == UART_RX_FOOTER){
            uint8 crc8sum = CRC8( &UART_rx_buffer, buffer_ind_max );
            UART_1_UartPutChar(crc8sum);
            // apply this new pattern
            operating_mode = STOP;
            uint8 i;
            for (i=0; i<NUM_LOCATIONS; i++){
                SSVEP_intervals[i] = SSVEP_intervals_next[i];
                SSVEP_states[i] = LED_OFF;
                SSVEP_next_toggle[i] = 0;
                write_pin(i, LED_OFF);
            }
            operating_mode = SSVEP;
        }
    }
            
}

void ssvep_buffer_reset(){
    uint8 i; 
    for (i=0; i<NUM_LOCATIONS; i++){
        SSVEP_intervals_next[i] = 0;
    }
    // UART_rx_buffer_process_ind = 0;
}

void uart_rx_buffer_process(uint8 buffer_ind_max){
    if (buffer_ind_max == 0){
        UART_rx_type = UART_rx_buffer[ buffer_ind_max ];
        if (UART_rx_type == SET_SSVEP){
            ssvep_buffer_reset();
        }
    }
    else {
        if (UART_rx_type == SET_SSVEP){
            uart_rx_buffer_process_set_ssvep(buffer_ind_max);
        }
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

void init_data(){
    uint8 i;
    for (i=0; i<NUM_LOCATIONS; i++){
        SSVEP_intervals[i] = 0;
        SSVEP_states[i] = LED_OFF;
        SSVEP_next_toggle[i] = 0;
        write_pin(i, LED_OFF);
        SSVEP_intervals_next[i] = 0;
    }
}

CY_ISR(timer_interrupt)
{
    mills++;
    
    if (1000 < mills && mills < 2000){
        if (mills_time_i < 100){
            Timer_2_Stop();
            mills_time[mills_time_i] = Timer_2_ReadCounter() * 10 / 12;
            Timer_2_WriteCounter(0);
            Timer_2_Enable();
            mills_time_i++;
        }
    }
    
    Timer_ms_ClearInterrupt(Timer_ms_INTR_MASK_TC);
}

CY_ISR(uart_rx_interrupt){    
    // get an incomming byte
    uint8 ch = UART_1_UartGetChar();
    uint8 escape_used = 0;
    
    if (ch == UART_RX_HEADER) {
        if (UART_rx_last_byte == UART_RX_HEADER) {
            uart_buffer_add_char(ch);
            escape_used = 1;
        }
    }
    else{
        if (UART_rx_last_byte == UART_RX_HEADER){
            uart_buffer_clear();
        }
        uart_buffer_add_char(ch);
        
        // process buffer up to this index
        // uart_rx_buffer_process(UART_rx_buffer_ind - 1);
    }
    
    if (escape_used == 0) {
        UART_rx_last_byte = ch;
    }
    else{
        UART_rx_last_byte = 0;
    }
    
    // UART_debug_UartPutChar(ch); // echo for debugging purpose
    
    // clear interrupt
    UART_1_ClearRxInterruptSource( UART_1_GetRxInterruptSourceMasked() );
}


int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    isr_ms_StartEx(timer_interrupt);
    isr_uart_rx_StartEx(uart_rx_interrupt);

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    init_data();
    SSVEP_intervals[0] = 11;
    SSVEP_intervals[1] = 15;
    SSVEP_intervals[2] = 12;
    SSVEP_intervals[3] = 21;
    SSVEP_intervals[4] = 23;
    SSVEP_intervals[5] = 21;
    SSVEP_intervals[6] = 27;
    SSVEP_intervals[7] = 12;
    SSVEP_intervals[8] = 11;
    SSVEP_intervals[9] = 15;
    SSVEP_intervals[10] = 17;
    SSVEP_intervals[11] = 19;
    SSVEP_intervals[12] = 16;
    SSVEP_intervals[13] = 12;
    SSVEP_intervals[14] = 10;
    SSVEP_intervals[15] = 25;
    
    /* Start the components */
    Timer_ms_Start();
    UART_1_Start();
    UART_debug_Start();
    Timer_2_Start();
    
    uint8 i;
    for(;;)
    {
        if (operating_mode == SSVEP){
            
            if (mills > 5000){
                tic = mills; // for evaluation
                Timer_2_WriteCounter(0);
                Timer_2_Enable();
            }
            
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
            
            if (mills > 5000){ // for evaluation
                if (sweep_i < 100){
                    // toc = mills - tic;
                    Timer_2_Stop();
                    sweep_time[sweep_i] = Timer_2_ReadCounter() * 10 / 12;
                    sweep_i++;
                }
                else if (sweep_i == 100){
                    Timer_2_Stop();
                    sweep_i++;
                }
            }
        }
        
        if (UART_rx_buffer_process_ind_reset_flag > 0){
            UART_rx_buffer_process_ind = 0;
            UART_rx_buffer_process_ind_reset_flag = 0;
        }
        if ( (UART_rx_buffer_ind-1) >= UART_rx_buffer_process_ind){
            uart_rx_buffer_process(UART_rx_buffer_process_ind);
            char hex_echo[2];
            sprintf(hex_echo, "%02X", UART_rx_buffer[UART_rx_buffer_process_ind]);
            UART_debug_UartPutString(hex_echo);
            UART_rx_buffer_process_ind++;
            
        }
        
    }
}


/* [] END OF FILE */
