/***************************************************************************//**
* \file .h
* \version 3.20
*
* \brief
*  This private file provides constants and parameter values for the
*  SCB Component.
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

#if !defined(CY_SCB_PVT_UART_debug_H)
#define CY_SCB_PVT_UART_debug_H

#include "UART_debug.h"


/***************************************
*     Private Function Prototypes
***************************************/

/* APIs to service INTR_I2C_EC register */
#define UART_debug_SetI2CExtClkInterruptMode(interruptMask) UART_debug_WRITE_INTR_I2C_EC_MASK(interruptMask)
#define UART_debug_ClearI2CExtClkInterruptSource(interruptMask) UART_debug_CLEAR_INTR_I2C_EC(interruptMask)
#define UART_debug_GetI2CExtClkInterruptSource()                (UART_debug_INTR_I2C_EC_REG)
#define UART_debug_GetI2CExtClkInterruptMode()                  (UART_debug_INTR_I2C_EC_MASK_REG)
#define UART_debug_GetI2CExtClkInterruptSourceMasked()          (UART_debug_INTR_I2C_EC_MASKED_REG)

#if (!UART_debug_CY_SCBIP_V1)
    /* APIs to service INTR_SPI_EC register */
    #define UART_debug_SetSpiExtClkInterruptMode(interruptMask) \
                                                                UART_debug_WRITE_INTR_SPI_EC_MASK(interruptMask)
    #define UART_debug_ClearSpiExtClkInterruptSource(interruptMask) \
                                                                UART_debug_CLEAR_INTR_SPI_EC(interruptMask)
    #define UART_debug_GetExtSpiClkInterruptSource()                 (UART_debug_INTR_SPI_EC_REG)
    #define UART_debug_GetExtSpiClkInterruptMode()                   (UART_debug_INTR_SPI_EC_MASK_REG)
    #define UART_debug_GetExtSpiClkInterruptSourceMasked()           (UART_debug_INTR_SPI_EC_MASKED_REG)
#endif /* (!UART_debug_CY_SCBIP_V1) */

#if(UART_debug_SCB_MODE_UNCONFIG_CONST_CFG)
    extern void UART_debug_SetPins(uint32 mode, uint32 subMode, uint32 uartEnableMask);
#endif /* (UART_debug_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*     Vars with External Linkage
***************************************/

#if (UART_debug_SCB_IRQ_INTERNAL)
#if !defined (CY_REMOVE_UART_debug_CUSTOM_INTR_HANDLER)
    extern cyisraddress UART_debug_customIntrHandler;
#endif /* !defined (CY_REMOVE_UART_debug_CUSTOM_INTR_HANDLER) */
#endif /* (UART_debug_SCB_IRQ_INTERNAL) */

extern UART_debug_BACKUP_STRUCT UART_debug_backup;

#if(UART_debug_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Common configuration variables */
    extern uint8 UART_debug_scbMode;
    extern uint8 UART_debug_scbEnableWake;
    extern uint8 UART_debug_scbEnableIntr;

    /* I2C configuration variables */
    extern uint8 UART_debug_mode;
    extern uint8 UART_debug_acceptAddr;

    /* SPI/UART configuration variables */
    extern volatile uint8 * UART_debug_rxBuffer;
    extern uint8   UART_debug_rxDataBits;
    extern uint32  UART_debug_rxBufferSize;

    extern volatile uint8 * UART_debug_txBuffer;
    extern uint8   UART_debug_txDataBits;
    extern uint32  UART_debug_txBufferSize;

    /* EZI2C configuration variables */
    extern uint8 UART_debug_numberOfAddr;
    extern uint8 UART_debug_subAddrSize;
#endif /* (UART_debug_SCB_MODE_UNCONFIG_CONST_CFG) */

#if (! (UART_debug_SCB_MODE_I2C_CONST_CFG || \
        UART_debug_SCB_MODE_EZI2C_CONST_CFG))
    extern uint16 UART_debug_IntrTxMask;
#endif /* (! (UART_debug_SCB_MODE_I2C_CONST_CFG || \
              UART_debug_SCB_MODE_EZI2C_CONST_CFG)) */


/***************************************
*        Conditional Macro
****************************************/

#if(UART_debug_SCB_MODE_UNCONFIG_CONST_CFG)
    /* Defines run time operation mode */
    #define UART_debug_SCB_MODE_I2C_RUNTM_CFG     (UART_debug_SCB_MODE_I2C      == UART_debug_scbMode)
    #define UART_debug_SCB_MODE_SPI_RUNTM_CFG     (UART_debug_SCB_MODE_SPI      == UART_debug_scbMode)
    #define UART_debug_SCB_MODE_UART_RUNTM_CFG    (UART_debug_SCB_MODE_UART     == UART_debug_scbMode)
    #define UART_debug_SCB_MODE_EZI2C_RUNTM_CFG   (UART_debug_SCB_MODE_EZI2C    == UART_debug_scbMode)
    #define UART_debug_SCB_MODE_UNCONFIG_RUNTM_CFG \
                                                        (UART_debug_SCB_MODE_UNCONFIG == UART_debug_scbMode)

    /* Defines wakeup enable */
    #define UART_debug_SCB_WAKE_ENABLE_CHECK       (0u != UART_debug_scbEnableWake)
#endif /* (UART_debug_SCB_MODE_UNCONFIG_CONST_CFG) */

/* Defines maximum number of SCB pins */
#if (!UART_debug_CY_SCBIP_V1)
    #define UART_debug_SCB_PINS_NUMBER    (7u)
#else
    #define UART_debug_SCB_PINS_NUMBER    (2u)
#endif /* (!UART_debug_CY_SCBIP_V1) */

#endif /* (CY_SCB_PVT_UART_debug_H) */


/* [] END OF FILE */
