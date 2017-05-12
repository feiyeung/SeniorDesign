/***************************************************************************//**
* \file UART_debug_SPI_UART_PVT.h
* \version 3.20
*
* \brief
*  This private file provides constants and parameter values for the
*  SCB Component in SPI and UART modes.
*  Please do not use this file or its content in your project.
*
* Note:
*
********************************************************************************
* \copyright
* Copyright 2013-2016, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_SCB_SPI_UART_PVT_UART_debug_H)
#define CY_SCB_SPI_UART_PVT_UART_debug_H

#include "UART_debug_SPI_UART.h"


/***************************************
*     Internal Global Vars
***************************************/

#if (UART_debug_INTERNAL_RX_SW_BUFFER_CONST)
    extern volatile uint32  UART_debug_rxBufferHead;
    extern volatile uint32  UART_debug_rxBufferTail;
    
    /**
    * \addtogroup group_globals
    * @{
    */
    
    /** Sets when internal software receive buffer overflow
     *  was occurred.
    */  
    extern volatile uint8   UART_debug_rxBufferOverflow;
    /** @} globals */
#endif /* (UART_debug_INTERNAL_RX_SW_BUFFER_CONST) */

#if (UART_debug_INTERNAL_TX_SW_BUFFER_CONST)
    extern volatile uint32  UART_debug_txBufferHead;
    extern volatile uint32  UART_debug_txBufferTail;
#endif /* (UART_debug_INTERNAL_TX_SW_BUFFER_CONST) */

#if (UART_debug_INTERNAL_RX_SW_BUFFER)
    extern volatile uint8 UART_debug_rxBufferInternal[UART_debug_INTERNAL_RX_BUFFER_SIZE];
#endif /* (UART_debug_INTERNAL_RX_SW_BUFFER) */

#if (UART_debug_INTERNAL_TX_SW_BUFFER)
    extern volatile uint8 UART_debug_txBufferInternal[UART_debug_TX_BUFFER_SIZE];
#endif /* (UART_debug_INTERNAL_TX_SW_BUFFER) */


/***************************************
*     Private Function Prototypes
***************************************/

void UART_debug_SpiPostEnable(void);
void UART_debug_SpiStop(void);

#if (UART_debug_SCB_MODE_SPI_CONST_CFG)
    void UART_debug_SpiInit(void);
#endif /* (UART_debug_SCB_MODE_SPI_CONST_CFG) */

#if (UART_debug_SPI_WAKE_ENABLE_CONST)
    void UART_debug_SpiSaveConfig(void);
    void UART_debug_SpiRestoreConfig(void);
#endif /* (UART_debug_SPI_WAKE_ENABLE_CONST) */

void UART_debug_UartPostEnable(void);
void UART_debug_UartStop(void);

#if (UART_debug_SCB_MODE_UART_CONST_CFG)
    void UART_debug_UartInit(void);
#endif /* (UART_debug_SCB_MODE_UART_CONST_CFG) */

#if (UART_debug_UART_WAKE_ENABLE_CONST)
    void UART_debug_UartSaveConfig(void);
    void UART_debug_UartRestoreConfig(void);
#endif /* (UART_debug_UART_WAKE_ENABLE_CONST) */


/***************************************
*         UART API Constants
***************************************/

/* UART RX and TX position to be used in UART_debug_SetPins() */
#define UART_debug_UART_RX_PIN_ENABLE    (UART_debug_UART_RX)
#define UART_debug_UART_TX_PIN_ENABLE    (UART_debug_UART_TX)

/* UART RTS and CTS position to be used in  UART_debug_SetPins() */
#define UART_debug_UART_RTS_PIN_ENABLE    (0x10u)
#define UART_debug_UART_CTS_PIN_ENABLE    (0x20u)


/***************************************
* The following code is DEPRECATED and
* must not be used.
***************************************/

/* Interrupt processing */
#define UART_debug_SpiUartEnableIntRx(intSourceMask)  UART_debug_SetRxInterruptMode(intSourceMask)
#define UART_debug_SpiUartEnableIntTx(intSourceMask)  UART_debug_SetTxInterruptMode(intSourceMask)
uint32  UART_debug_SpiUartDisableIntRx(void);
uint32  UART_debug_SpiUartDisableIntTx(void);


#endif /* (CY_SCB_SPI_UART_PVT_UART_debug_H) */


/* [] END OF FILE */
