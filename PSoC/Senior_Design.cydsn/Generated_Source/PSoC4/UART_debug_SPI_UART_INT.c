/***************************************************************************//**
* \file UART_debug_SPI_UART_INT.c
* \version 3.20
*
* \brief
*  This file provides the source code to the Interrupt Service Routine for
*  the SCB Component in SPI and UART modes.
*
* Note:
*
********************************************************************************
* \copyright
* Copyright 2013-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "UART_debug_PVT.h"
#include "UART_debug_SPI_UART_PVT.h"
#include "cyapicallbacks.h"

#if (UART_debug_SCB_IRQ_INTERNAL)
/*******************************************************************************
* Function Name: UART_debug_SPI_UART_ISR
****************************************************************************//**
*
*  Handles the Interrupt Service Routine for the SCB SPI or UART modes.
*
*******************************************************************************/
CY_ISR(UART_debug_SPI_UART_ISR)
{
#if (UART_debug_INTERNAL_RX_SW_BUFFER_CONST)
    uint32 locHead;
#endif /* (UART_debug_INTERNAL_RX_SW_BUFFER_CONST) */

#if (UART_debug_INTERNAL_TX_SW_BUFFER_CONST)
    uint32 locTail;
#endif /* (UART_debug_INTERNAL_TX_SW_BUFFER_CONST) */

#ifdef UART_debug_SPI_UART_ISR_ENTRY_CALLBACK
    UART_debug_SPI_UART_ISR_EntryCallback();
#endif /* UART_debug_SPI_UART_ISR_ENTRY_CALLBACK */

    if (NULL != UART_debug_customIntrHandler)
    {
        UART_debug_customIntrHandler();
    }

    #if(UART_debug_CHECK_SPI_WAKE_ENABLE)
    {
        /* Clear SPI wakeup source */
        UART_debug_ClearSpiExtClkInterruptSource(UART_debug_INTR_SPI_EC_WAKE_UP);
    }
    #endif

    #if (UART_debug_CHECK_RX_SW_BUFFER)
    {
        if (UART_debug_CHECK_INTR_RX_MASKED(UART_debug_INTR_RX_NOT_EMPTY))
        {
            do
            {
                /* Move local head index */
                locHead = (UART_debug_rxBufferHead + 1u);

                /* Adjust local head index */
                if (UART_debug_INTERNAL_RX_BUFFER_SIZE == locHead)
                {
                    locHead = 0u;
                }

                if (locHead == UART_debug_rxBufferTail)
                {
                    #if (UART_debug_CHECK_UART_RTS_CONTROL_FLOW)
                    {
                        /* There is no space in the software buffer - disable the
                        * RX Not Empty interrupt source. The data elements are
                        * still being received into the RX FIFO until the RTS signal
                        * stops the transmitter. After the data element is read from the
                        * buffer, the RX Not Empty interrupt source is enabled to
                        * move the next data element in the software buffer.
                        */
                        UART_debug_INTR_RX_MASK_REG &= ~UART_debug_INTR_RX_NOT_EMPTY;
                        break;
                    }
                    #else
                    {
                        /* Overflow: through away received data element */
                        (void) UART_debug_RX_FIFO_RD_REG;
                        UART_debug_rxBufferOverflow = (uint8) UART_debug_INTR_RX_OVERFLOW;
                    }
                    #endif
                }
                else
                {
                    /* Store received data */
                    UART_debug_PutWordInRxBuffer(locHead, UART_debug_RX_FIFO_RD_REG);

                    /* Move head index */
                    UART_debug_rxBufferHead = locHead;
                }
            }
            while(0u != UART_debug_GET_RX_FIFO_ENTRIES);

            UART_debug_ClearRxInterruptSource(UART_debug_INTR_RX_NOT_EMPTY);
        }
    }
    #endif


    #if (UART_debug_CHECK_TX_SW_BUFFER)
    {
        if (UART_debug_CHECK_INTR_TX_MASKED(UART_debug_INTR_TX_NOT_FULL))
        {
            do
            {
                /* Check for room in TX software buffer */
                if (UART_debug_txBufferHead != UART_debug_txBufferTail)
                {
                    /* Move local tail index */
                    locTail = (UART_debug_txBufferTail + 1u);

                    /* Adjust local tail index */
                    if (UART_debug_TX_BUFFER_SIZE == locTail)
                    {
                        locTail = 0u;
                    }

                    /* Put data into TX FIFO */
                    UART_debug_TX_FIFO_WR_REG = UART_debug_GetWordFromTxBuffer(locTail);

                    /* Move tail index */
                    UART_debug_txBufferTail = locTail;
                }
                else
                {
                    /* TX software buffer is empty: complete transfer */
                    UART_debug_DISABLE_INTR_TX(UART_debug_INTR_TX_NOT_FULL);
                    break;
                }
            }
            while (UART_debug_SPI_UART_FIFO_SIZE != UART_debug_GET_TX_FIFO_ENTRIES);

            UART_debug_ClearTxInterruptSource(UART_debug_INTR_TX_NOT_FULL);
        }
    }
    #endif

#ifdef UART_debug_SPI_UART_ISR_EXIT_CALLBACK
    UART_debug_SPI_UART_ISR_ExitCallback();
#endif /* UART_debug_SPI_UART_ISR_EXIT_CALLBACK */

}

#endif /* (UART_debug_SCB_IRQ_INTERNAL) */


/* [] END OF FILE */
