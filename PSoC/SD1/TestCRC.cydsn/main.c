

#include <project.h>


CY_ISR(uart_rx_isr){
    char ch = UART_1_GetChar();
    
    if (ch == 27){
        CRC_1_WriteSeed(0x0000);
    }
    
    CyDelay(1u);
    
    uint16 crc = CRC_1_ReadCRC();
    char str_out[4];
    sprintf(str_out, "%04X", crc);
    UART_2_UartPutString(str_out);
    UART_2_UartPutString("\r\n");
}

int main()
{
    isr_1_StartEx(uart_rx_isr);
    CyGlobalIntEnable;
    UART_1_Start();
    UART_2_Start();
    CRC_1_Start();
    UART_2_UartPutString("\r\n");
    
    uint16 crc = CRC_1_ReadCRC();
    char str_out[4];
    sprintf(str_out, "%04X", crc);
    UART_2_UartPutString(str_out);
    UART_2_UartPutString("\r\n");
    
    for (;;) 
    {
        
        
    }
}


