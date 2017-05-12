/*******************************************************************************
* File Name: CRC_1.c
* Version 2.40
*
* Description:
*  This file provides the source code to the API for the CRC component
*
* Note:
*  None
*
********************************************************************************
* Copyright 2008-2013, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "CRC_1.h"

uint8 CRC_1_initVar = 0u;

CRC_1_BACKUP_STRUCT CRC_1_backup =
{
    0x00u, /* enableState; */

    /* Save D0 and D1 registers are none-retention for Panther ES1 */
    #if (CY_UDB_V0)

        #if(CRC_1_CRC_SIZE <= 32u)
            CRC_1_DEFAULT_POLYNOM, /* polynomial; */

        #else
            CRC_1_DEFAULT_POLYNOM_UPPER, /* polynomialUpper; */
            CRC_1_DEFAULT_POLYNOM_LOWER,  /* polynomialLower; */

        #endif  /* End (CRC_1_CRC_SIZE <= 32u) */

    #endif  /* End (CY_UDB_V0) */
};


/*******************************************************************************
* Function Name: CRC_1_Init
********************************************************************************
*
* Summary:
*  Initializes seed and polynomial registers with initial values.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void CRC_1_Init(void) 
{
    /* Writes seed value and ponynom value provided for customizer */
    #if (CRC_1_CRC_SIZE <= 32u)
        CRC_1_WritePolynomial(CRC_1_DEFAULT_POLYNOM);
        CRC_1_WriteSeed(CRC_1_DEFAULT_SEED);
        CRC_1_ResetSeedInit(CRC_1_DEFAULT_SEED);

    #else
        CRC_1_WritePolynomialUpper(CRC_1_DEFAULT_POLYNOM_UPPER);
        CRC_1_WritePolynomialLower(CRC_1_DEFAULT_POLYNOM_LOWER);
        CRC_1_WriteSeedUpper(CRC_1_DEFAULT_SEED_UPPER);
        CRC_1_WriteSeedLower(CRC_1_DEFAULT_SEED_LOWER);
        CRC_1_ResetSeedInitUpper(CRC_1_DEFAULT_SEED_UPPER);
        CRC_1_ResetSeedInitLower(CRC_1_DEFAULT_SEED_LOWER);

    #endif  /* End (CRC_1_CRC_SIZE <= 32u) */
}


/*******************************************************************************
* Function Name: CRC_1_Enable
********************************************************************************
*
* Summary:
*  Starts CRC computation on rising edge of input clock.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void CRC_1_Enable(void) 
{
    CRC_1_CONTROL_REG |= CRC_1_CTRL_ENABLE;
}


/*******************************************************************************
* Function Name: CRC_1_Start
********************************************************************************
*
* Summary:
*  Initializes seed and polynomial registers with initial values. Computation
*  of CRC starts on rising edge of input clock.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  CRC_1_initVar: global variable is used to indicate initial
*  configuration of this component.  The variable is initialized to zero and set
*  to 1 the first time CRC_1_Start() is called. This allows
*  enable/disable component without re-initialization in all subsequent calls
*  to the CRC_1_Start() routine.
*
*******************************************************************************/
void CRC_1_Start(void) 
{
    /* Writes seed value and ponynom value provided from customizer */
    if (CRC_1_initVar == 0u)
    {
        CRC_1_Init();
        CRC_1_initVar = 1u;
    }

    CRC_1_Enable();
}


/*******************************************************************************
* Function Name: CRC_1_Stop
********************************************************************************
*
* Summary:
*  Stops CRC computation.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Reentrant:
*  No
*
*******************************************************************************/
void CRC_1_Stop(void) 
{
    CRC_1_CONTROL_REG &= ((uint8)~CRC_1_CTRL_ENABLE);
}


#if (CRC_1_CRC_SIZE <= 32u) /* 8-32 bits CRC */
    /*******************************************************************************
    * FUNCTION NAME: CRC_1_ResetSeedInit
    ********************************************************************************
    *
    * Summary:
    *  Increments the CRC by one when API single step mode is used.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  void
    *
    *******************************************************************************/
    void CRC_1_ResetSeedInit(uint16 seed)
                                        
    {
         uint8 enableInterrupts;

        /* Mask the Seed to cut unused bits */
        #if((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u))
            seed &= CRC_1_MASK;
        #endif /* ((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u)) */

        /* Change AuxControl reg, need to be safety */
        enableInterrupts = CyEnterCriticalSection();

        #if (CRC_1_TIME_MULTIPLEXING_ENABLE)
            /* Set FIFOs to single register */
            CRC_1_AUX_CONTROL_A_REG |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;

            #if(CRC_1_CRC_SIZE > 16u)       /* 17-32 bits CRC */
                CRC_1_AUX_CONTROL_B_REG |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;
            #endif  /* End (CRC_1_CRC_SIZE <= 8u) */

            /* AuxControl reg settings are done */
            CyExitCriticalSection(enableInterrupts);

            /* Write Seed COPY */
            #if (CRC_1_CRC_SIZE <= 16u)          /* 16 bits CRC */
                CRC_1_SEED_COPY_A__A1_REG = HI8(seed);
                CRC_1_SEED_COPY_A__A0_REG = LO8(seed);

            #elif (CRC_1_CRC_SIZE <= 24u)        /* 24 bits CRC */
                CRC_1_SEED_COPY_B__A1_REG = LO8(HI16(seed));
                CRC_1_SEED_COPY_B__A0_REG = HI8(seed);
                CRC_1_SEED_COPY_A__A0_REG = LO8(seed);

            #else                                           /* 32 bits CRC */
                CRC_1_SEED_COPY_B__A1_REG = HI8(HI16(seed));
                CRC_1_SEED_COPY_A__A1_REG = LO8(HI16(seed));
                CRC_1_SEED_COPY_B__A0_REG = HI8(seed);
                CRC_1_SEED_COPY_A__A0_REG = LO8(seed);
            #endif  /* End (CRC_1_CRC_SIZE <= 32u) */

        #else
            /* Set FIFOs to single register */
            #if (CRC_1_CRC_SIZE <= 8u)         /* 8 bits CRC */
                CRC_1_AUX_CONTROL_A_REG |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;

            #elif (CRC_1_CRC_SIZE <= 16u)      /* 16 bits CRC */
                CRC_1_AUX_CONTROL_A_REG  |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;
                CRC_1_AUX_CONTROL_B_REG  |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;

            #elif (CRC_1_CRC_SIZE <= 24u)      /* 24 bits CRC */
                CRC_1_AUX_CONTROL_A_REG  |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;
                CRC_1_AUX_CONTROL_B_REG  |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;
                CRC_1_AUX_CONTROL_C_REG  |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;

            #elif (CRC_1_CRC_SIZE <= 32u)      /* 32 bits CRC */
                CRC_1_AUX_CONTROL_A_REG  |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;
                CRC_1_AUX_CONTROL_B_REG  |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;
                CRC_1_AUX_CONTROL_C_REG  |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;
                CRC_1_AUX_CONTROL_D_REG  |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;

            #endif  /* End (CRC_1_CRC_SIZE <= 8u) */

            /* AuxControl reg setting are done */
            CyExitCriticalSection(enableInterrupts);

            /* Write Seed COPY */
            CY_SET_REG16(CRC_1_SEED_COPY_PTR, seed);
        #endif  /* End (CRC_1_TIME_MULTIPLEXING_ENABLE) */
    }

#else

    /*******************************************************************************
    * FUNCTION NAME: CRC_1_ResetSeedInitUpper
    ********************************************************************************
    *
    * Summary:
    *  Increments the CRC by one when API single step mode is used.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  void
    *
    *******************************************************************************/
    void CRC_1_ResetSeedInitUpper(uint32 seed) 
    {
        uint8 enableInterrupts;

        /* Mask the Seed Upper half to cut unused bits in case, when CRC size is not byte-aligned or equal 24 */
        #if((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u))
            seed &= CRC_1_MASK;
        #endif /* ((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u)) */

        /* Change AuxControl reg, need to be safety */
        enableInterrupts = CyEnterCriticalSection();

        /* Set FIFOs to single register */
        CRC_1_AUX_CONTROL_A_REG |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;
        CRC_1_AUX_CONTROL_B_REG |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;
        CRC_1_AUX_CONTROL_C_REG |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;

        #if (CRC_1_CRC_SIZE > 48u)               /* 49-64 bits CRC */
            CRC_1_AUX_CONTROL_D_REG |= CRC_1_AUXCTRL_FIFO_SINGLE_REG;
        #endif  /* End (CRC_1_CRC_SIZE <= 8u) */

        /* AuxControl reg settings are done */
        CyExitCriticalSection(enableInterrupts);

        /* Write Seed Upper COPY */
        #if (CRC_1_CRC_SIZE <= 40u)          /* 40 bits CRC */
            CRC_1_SEED_UPPER_COPY_C__A1_REG = LO8(seed);

        #elif (CRC_1_CRC_SIZE <= 48u)        /* 48 bits CRC */
            CRC_1_SEED_UPPER_COPY_C__A1_REG = HI8(seed);
            CRC_1_SEED_UPPER_COPY_B__A1_REG = LO8(seed);

        #elif (CRC_1_CRC_SIZE <= 56u)        /* 56 bits CRC */
            CRC_1_SEED_UPPER_COPY_D__A1_REG = LO8(HI16(seed));
            CRC_1_SEED_UPPER_COPY_C__A1_REG = HI8(seed);
            CRC_1_SEED_UPPER_COPY_B__A1_REG = HI8(seed);

        #else                                           /* 64 bits CRC */
            CRC_1_SEED_UPPER_COPY_D__A1_REG = HI8(HI16(seed));
            CRC_1_SEED_UPPER_COPY_C__A1_REG = LO8(HI16(seed));
            CRC_1_SEED_UPPER_COPY_B__A1_REG = HI8(seed);
            CRC_1_SEED_UPPER_COPY_A__A1_REG = LO8(seed);

        #endif  /* End (CRC_1_CRC_SIZE <= 32u) */
    }


    /*******************************************************************************
    * FUNCTION NAME: CRC_1_ResetSeedInitLower
    ********************************************************************************
    *
    * Summary:
    *  Increments the CRC by one when API single step mode is used.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  void
    *
    *******************************************************************************/
    void CRC_1_ResetSeedInitLower(uint32 seed) 
    {
        /* Write Seed Lower COPY */
        #if (CRC_1_CRC_SIZE <= 40u)          /* 40 bits CRC */
            CRC_1_SEED_LOWER_COPY_B__A1_REG = HI8(HI16(seed));
            CRC_1_SEED_LOWER_COPY_C__A0_REG = LO8(HI16(seed));
            CRC_1_SEED_LOWER_COPY_B__A0_REG = HI8(seed);
            CRC_1_SEED_LOWER_COPY_A__A0_REG = LO8(seed);

        #elif (CRC_1_CRC_SIZE <= 48u)        /* 48 bits CRC */
            CRC_1_SEED_LOWER_COPY_A__A1_REG = HI8(HI16(seed));
            CRC_1_SEED_LOWER_COPY_C__A0_REG = LO8(HI16(seed));
            CRC_1_SEED_LOWER_COPY_B__A0_REG = HI8(seed);
            CRC_1_SEED_LOWER_COPY_A__A0_REG = LO8(seed);

        #else                                           /* 64 bits CRC */
            CRC_1_SEED_LOWER_COPY_D__A0_REG = HI8(HI16(seed));
            CRC_1_SEED_LOWER_COPY_C__A0_REG = LO8(HI16(seed));
            CRC_1_SEED_LOWER_COPY_B__A0_REG = HI8(seed);
            CRC_1_SEED_LOWER_COPY_A__A0_REG = LO8(seed);

        #endif  /* End (CRC_1_CRC_SIZE <= 32u) */
    }
#endif  /* End (CRC_1_CRC_SIZE <= 32u) */


#if(CRC_1_CRC_SIZE <= 32u) /* 8-32 bits CRC */
    /*******************************************************************************
    * Function Name: CRC_1_ReadCRC
    ********************************************************************************
    *
    * Summary:
    *  Reads CRC value.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  Returns CRC value.
    *
    * Side Effects:
    *  The CRC value is cut according to mask = 2^(Resolution) - 1.
    *  For example if CRC Resolution is 14 bits the mask value is:
    *  mask = 2^(14) - 1 = 0x3FFFu.
    *  The seed value = 0xFFFFu is cut:
    *  seed & mask = 0xFFFFu & 0x3FFFu = 0x3FFFu.
    *
    *******************************************************************************/
    uint16 CRC_1_ReadCRC(void) 
    {
        /* Read CRC */
        #if (CRC_1_TIME_MULTIPLEXING_ENABLE)

            uint16 seed;

            #if (CRC_1_CRC_SIZE <= 16u)          /* 16 bits CRC */
                seed = ((uint16) CRC_1_SEED_A__A1_REG) << 8u;
                seed |= CRC_1_SEED_A__A0_REG;

            #elif (CRC_1_CRC_SIZE <= 24u)        /* 24 bits CRC */
                seed = ((uint32) (CRC_1_SEED_B__A1_REG)) << 16u;
                seed |= ((uint32) (CRC_1_SEED_B__A0_REG)) << 8u;
                seed |= CRC_1_SEED_A__A0_REG;

            #else                                           /* 32 bits CRC */
                seed = ((uint32) CRC_1_SEED_B__A1_REG) << 24u;
                seed |= ((uint32) CRC_1_SEED_A__A1_REG) << 16u;
                seed |= ((uint32) CRC_1_SEED_B__A0_REG) << 8u;
                seed |= CRC_1_SEED_A__A0_REG;

            #endif  /* End (CRC_1_CRC_SIZE <= 8u) */
            
            #if((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u))
                seed &= CRC_1_MASK;
            #endif /* ((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u)) */
            
            return (seed);

        #else
            
            #if((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u))
                return (CY_GET_REG16(CRC_1_SEED_PTR)  & CRC_1_MASK);
            #else
                return (CY_GET_REG16(CRC_1_SEED_PTR));
            #endif /* ((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u)) */
            

        #endif  /* End (CRC_1_TIME_MULTIPLEXING_ENABLE) */
    }


    /*******************************************************************************
    * Function Name: CRC_1_WriteSeed
    ********************************************************************************
    *
    * Summary:
    *  Writes seed value.
    *
    * Parameters:
    *  seed:  Seed value.
    *
    * Return:
    *  void
    *
    * Side Effects:
    *  The seed value is cut according to mask = 2^(Resolution) - 1.
    *  For example if CRC Resolution is 14 bits the mask value is:
    *  mask = 2^(14) - 1 = 0x3FFFu.
    *  The seed value = 0xFFFFu is cut:
    *  seed & mask = 0xFFFFu & 0x3FFFu = 0x3FFFu.
    *
    *******************************************************************************/
    void CRC_1_WriteSeed(uint16 seed) 
    {
        /* Mask the Seed Upper half to cut unused bits in case, when CRC size is not byte-aligned or equal 24 */
        #if((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u))
            seed &= CRC_1_MASK;
        #endif /* ((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u)) */

        /* Writes Seed */
        #if (CRC_1_TIME_MULTIPLEXING_ENABLE)

            #if (CRC_1_CRC_SIZE <= 16u)          /* 16 bits CRC */
                CRC_1_SEED_A__A1_REG = HI8(seed);
                CRC_1_SEED_A__A0_REG = LO8(seed);

            #elif (CRC_1_CRC_SIZE <= 24u)        /* 24 bits CRC */
                CRC_1_SEED_B__A1_REG = LO8(HI16(seed));
                CRC_1_SEED_B__A0_REG = HI8(seed);
                CRC_1_SEED_A__A0_REG = LO8(seed);

            #else                                           /* 32 bits CRC */
                CRC_1_SEED_B__A1_REG = HI8(HI16(seed));
                CRC_1_SEED_A__A1_REG = LO8(HI16(seed));
                CRC_1_SEED_B__A0_REG = HI8(seed);
                CRC_1_SEED_A__A0_REG = LO8(seed);
            #endif  /* End (CRC_1_CRC_SIZE <= 32u) */

            /* Resets triggers */
            CRC_1_EXECUTE_DFF_RESET;

        #else

            CY_SET_REG16(CRC_1_SEED_PTR, seed);

        #endif  /* End (CRC_1_TIME_MULTIPLEXING_ENABLE) */
    }


    /*******************************************************************************
    * Function Name: CRC_1_ReadPolynomial
    ********************************************************************************
    *
    * Summary:
    *  Reads CRC polynomial value.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  Returns CRC polynomial value.
    *
    *******************************************************************************/
    uint16 CRC_1_ReadPolynomial(void)
                                               
    {
        /* Reads polynomial */
        #if (CRC_1_TIME_MULTIPLEXING_ENABLE)

            uint16 polynomial;

            #if (CRC_1_CRC_SIZE <= 16u)          /* 16 bits CRC */
                polynomial = ((uint16) CRC_1_POLYNOM_A__D1_REG) << 8u;
                polynomial |= (CRC_1_POLYNOM_A__D0_REG);

            #elif (CRC_1_CRC_SIZE <= 24u)        /* 24 bits CRC */
                polynomial = ((uint32) CRC_1_POLYNOM_B__D1_REG) << 16u;
                polynomial |= ((uint32) CRC_1_POLYNOM_B__D0_REG) << 8u;
                polynomial |= CRC_1_POLYNOM_A__D0_REG;

            #else                                           /* 32 bits CRC */
                polynomial = ((uint32) CRC_1_POLYNOM_B__D1_REG) << 24u;
                polynomial |= ((uint32) CRC_1_POLYNOM_A__D1_REG) << 16u;
                polynomial |= ((uint32) CRC_1_POLYNOM_B__D0_REG) << 8u;
                polynomial |= CRC_1_POLYNOM_A__D0_REG;

            #endif  /* End (CRC_1_CRC_SIZE <= 32u) */

            return polynomial;

        #else

            return CY_GET_REG16(CRC_1_POLYNOM_PTR);

        #endif  /* End (CRC_1_TIME_MULTIPLEXING_ENABLE) */
    }


    /*******************************************************************************
    * Function Name: CRC_1_WritePolynomial
    ********************************************************************************
    *
    * Summary:
    *  Writes CRC polynomial value.
    *
    * Parameters:
    *  polynomial:  CRC polynomial.
    *
    * Return:
    *  void
    *
    * Side Effects:
    *  The polynomial value is cut according to mask = 2^(Resolution) - 1.
    *  For example if CRC Resolution is 14 bits the mask value is:
    *  mask = 2^(14) - 1 = 0x3FFFu.
    *  The polynomial value = 0xFFFFu is cut:
    *  polynomial & mask = 0xFFFFu & 0x3FFFu = 0x3FFFu.
    *
    *******************************************************************************/
    void CRC_1_WritePolynomial(uint16 polynomial)
                                          
    {
        /* Mask polynomial to cut unused bits in case, when CRC size is not byte-aligned or equal 24 */
        #if((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u))
            polynomial &= CRC_1_MASK;
        #endif /* ((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u)) */

        /* Write polynomial */
        #if (CRC_1_TIME_MULTIPLEXING_ENABLE)

            #if (CRC_1_CRC_SIZE <= 16u)          /* 16 bits CRC */

                CRC_1_POLYNOM_A__D1_REG = HI8(polynomial);
                CRC_1_POLYNOM_A__D0_REG = LO8(polynomial);

            #elif (CRC_1_CRC_SIZE <= 24u)        /* 24 bits CRC */
                CRC_1_POLYNOM_B__D1_REG = LO8(HI16(polynomial));
                CRC_1_POLYNOM_B__D0_REG = HI8(polynomial);
                CRC_1_POLYNOM_A__D0_REG = LO8(polynomial);

            #else                                           /* 32 bits CRC */
                CRC_1_POLYNOM_B__D1_REG = HI8(HI16(polynomial));
                CRC_1_POLYNOM_A__D1_REG = LO8(HI16(polynomial));
                CRC_1_POLYNOM_B__D0_REG = HI8(polynomial);
                CRC_1_POLYNOM_A__D0_REG = LO8(polynomial);

            #endif  /* End (CRC_1_CRC_SIZE <= 32u) */

            /* Resets triggers */
            CRC_1_EXECUTE_DFF_RESET;

        #else

            CY_SET_REG16(CRC_1_POLYNOM_PTR, polynomial);

        #endif  /* End (CRC_1_TIME_MULTIPLEXING_ENABLE) */
    }

#else   /* 33-64 bits CRC */

    /*******************************************************************************
    *  Function Name: CRC_1_ReadCRCUpper
    ********************************************************************************
    *
    * Summary:
    *  Reads upper half of CRC value. Only generated for 33-64-bit CRC.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  Returns upper half of CRC value.
    *
    * Side Effects:
    *  The polynomial value is cut according to mask = 2^(Resolution) - 1.
    *  For example if CRC Resolution is 14 bits the mask value is:
    *  mask = 2^(14) - 1 = 0x3FFFu.
    *  The polynomial value = 0xFFFFu is cut:
    *  polynomial & mask = 0xFFFFu & 0x3FFFu = 0x3FFFu.
    *
    *******************************************************************************/
    uint32 CRC_1_ReadCRCUpper(void) 
    {
        uint32 seed;

        /* Read CRC Upper */
        #if (CRC_1_CRC_SIZE <= 40u)          /* 40 bits CRC */
            seed = CRC_1_SEED_UPPER_C__A1_REG;

        #elif (CRC_1_CRC_SIZE <= 48u)        /* 48 bits CRC */
            seed = ((uint32) CRC_1_SEED_UPPER_C__A1_REG) << 8u;
            seed |= CRC_1_SEED_UPPER_B__A1_REG;

        #elif (CRC_1_CRC_SIZE <= 56u)        /* 56 bits CRC */
            seed = ((uint32) CRC_1_SEED_UPPER_D__A1_REG) << 16u;
            seed |= ((uint32) CRC_1_SEED_UPPER_C__A1_REG) << 8u;
            seed |= CRC_1_SEED_UPPER_B__A1_REG;

        #else                                           /* 64 bits CRC */
            seed = ((uint32) CRC_1_SEED_UPPER_D__A1_REG) << 24u;
            seed |= ((uint32) CRC_1_SEED_UPPER_C__A1_REG) << 16u;
            seed |= ((uint32) CRC_1_SEED_UPPER_B__A1_REG) << 8u;
            seed |= CRC_1_SEED_UPPER_A__A1_REG;

        #endif  /* End (CRC_1_CRC_SIZE <= 32u) */
        
        #if((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u))
            seed &= CRC_1_MASK;
        #endif /* ((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u)) */
                
        return (seed);
    }


    /*******************************************************************************
    *  Function Name: CRC_1_ReadCRCLower
    ********************************************************************************
    *
    * Summary:
    *  Reads lower half of CRC value. Only generated for 33-64-bit CRC.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  Returns lower half of CRC value.
    *
    *******************************************************************************/
    uint32 CRC_1_ReadCRCLower(void) 
    {
        uint32 seed;

        /* Read CRC Lower */
        #if (CRC_1_CRC_SIZE <= 40u)          /* 40 bits CRC */
            seed = ((uint32) CRC_1_SEED_LOWER_B__A1_REG) << 24u;
            seed |= ((uint32) CRC_1_SEED_LOWER_C__A0_REG) << 16u;
            seed |= ((uint32) CRC_1_SEED_LOWER_B__A0_REG) << 8u;
            seed |= CRC_1_SEED_LOWER_A__A0_REG;

        #elif (CRC_1_CRC_SIZE <= 48u)        /* 48 bits CRC */
            seed = ((uint32) CRC_1_SEED_LOWER_A__A1_REG) << 24u;
            seed |= ((uint32) CRC_1_SEED_LOWER_C__A0_REG) << 16u;
            seed |= ((uint32) CRC_1_SEED_LOWER_B__A0_REG) << 8u;
            seed |= CRC_1_SEED_LOWER_A__A0_REG;

        #else                                           /* 64 bits CRC */
            seed = ((uint32) CRC_1_SEED_LOWER_D__A0_REG) << 24u;
            seed |= ((uint32) CRC_1_SEED_LOWER_C__A0_REG) << 16u;
            seed |= ((uint32) CRC_1_SEED_LOWER_B__A0_REG) << 8u;
            seed |= CRC_1_SEED_LOWER_A__A0_REG;

        #endif  /* End (CRC_1_CRC_SIZE <= 32u) */

        return seed;
    }


    /*******************************************************************************
    * Function Name: CRC_1_WriteSeedUpper
    ********************************************************************************
    *
    * Summary:
    *  Writes upper half of seed value. Only generated for 33-64-bit CRC.
    *
    * Parameters:
    *  seed:  Upper half of seed value.
    *
    * Return:
    *  void
    *
    * Side Effects:
    *  The upper half of seed value is cut according to
    *  mask = 2^(Resolution - 32) - 1.
    *  For example if CRC Resolution is 35 bits the mask value is:
    *  2^(35 - 32) - 1 = 2^(3) - 1 = 0x0000 0007u.
    *  The upper half of seed value = 0x0000 00FFu is cut:
    *  upper half of seed & mask = 0x0000 00FFu & 0x0000 0007u = 0x0000 0007u.
    *
    *******************************************************************************/
    void CRC_1_WriteSeedUpper(uint32 seed) 
    {
        /* Mask the Seed Upper half to cut unused bits in case, when CRC size is not byte-aligned or equal 24 */
        #if((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u))
            seed &= CRC_1_MASK;
        #endif /* ((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u)) */

        /* Write Seed Upper */
        #if (CRC_1_CRC_SIZE <= 40u)          /* 40 bits CRC */
            CRC_1_SEED_UPPER_C__A1_REG = LO8(seed);

        #elif (CRC_1_CRC_SIZE <= 48u)        /* 48 bits CRC */
            CRC_1_SEED_UPPER_C__A1_REG = HI8(seed);
            CRC_1_SEED_UPPER_B__A1_REG = LO8(seed);

        #elif (CRC_1_CRC_SIZE <= 56u)        /* 56 bits CRC */
            CRC_1_SEED_UPPER_D__A1_REG = LO8(HI16(seed));
            CRC_1_SEED_UPPER_C__A1_REG = HI8(seed);
            CRC_1_SEED_UPPER_B__A1_REG = HI8(seed);

        #else                                           /* 64 bits CRC */
            CRC_1_SEED_UPPER_D__A1_REG = HI8(HI16(seed));
            CRC_1_SEED_UPPER_C__A1_REG = LO8(HI16(seed));
            CRC_1_SEED_UPPER_B__A1_REG = HI8(seed);
            CRC_1_SEED_UPPER_A__A1_REG = LO8(seed);

        #endif  /* End (CRC_1_CRC_SIZE <= 32u) */

        /* Resets triggers */
        CRC_1_EXECUTE_DFF_RESET;
    }


    /*******************************************************************************
    * Function Name: CRC_1_WriteSeedLower
    ********************************************************************************
    *
    * Summary:
    *  Writes lower half of seed value. Only generated for 33-64-bit CRC.
    *
    * Parameters:
    *  seed:  Lower half of seed value.
    *
    * Return:
    *  void
    *
    *******************************************************************************/
    void CRC_1_WriteSeedLower(uint32 seed) 
    {
        /* Write Seed Lower */
        #if (CRC_1_CRC_SIZE <= 40u)          /* 40 bits CRC */
            CRC_1_SEED_LOWER_B__A1_REG = HI8(HI16(seed));
            CRC_1_SEED_LOWER_C__A0_REG = LO8(HI16(seed));
            CRC_1_SEED_LOWER_B__A0_REG = HI8(seed);
            CRC_1_SEED_LOWER_A__A0_REG = LO8(seed);

        #elif (CRC_1_CRC_SIZE <= 48u)        /* 48 bits CRC */
            CRC_1_SEED_LOWER_A__A1_REG = HI8(HI16(seed));
            CRC_1_SEED_LOWER_C__A0_REG = LO8(HI16(seed));
            CRC_1_SEED_LOWER_B__A0_REG = HI8(seed);
            CRC_1_SEED_LOWER_A__A0_REG = LO8(seed);

        #else                                           /* 64 bits CRC */
            CRC_1_SEED_LOWER_D__A0_REG = HI8(HI16(seed));
            CRC_1_SEED_LOWER_C__A0_REG = LO8(HI16(seed));
            CRC_1_SEED_LOWER_B__A0_REG = HI8(seed);
            CRC_1_SEED_LOWER_A__A0_REG = LO8(seed);

        #endif  /* End (CRC_1_CRC_SIZE <= 32u) */

        /* Resets triggers */
        CRC_1_EXECUTE_DFF_RESET;
    }


    /*******************************************************************************
    * Function Name: CRC_1_ReadPolynomialUpper
    ********************************************************************************
    *
    * Summary:
    *  Reads upper half of CRC polynomial value. Only generated for 33-64-bit CRC.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  Returns upper half of CRC polynomial value.
    *
    *******************************************************************************/
    uint32 CRC_1_ReadPolynomialUpper(void) 
    {
        uint32 polynomial;

        /* Read Polynomial Upper */
        #if (CRC_1_CRC_SIZE <= 40u)          /* 40 bits CRC */
            polynomial = CRC_1_POLYNOM_UPPER_C__D1_REG;

        #elif (CRC_1_CRC_SIZE <= 48u)        /* 48 bits CRC */
            polynomial =  ((uint32) CRC_1_POLYNOM_UPPER_C__D1_REG) << 8u;
            polynomial |= CRC_1_POLYNOM_UPPER_B__D1_REG;

        #elif (CRC_1_CRC_SIZE <= 56u)        /* 56 bits CRC */
            polynomial =  ((uint32) CRC_1_POLYNOM_UPPER_D__D1_REG) << 16u;
            polynomial |= ((uint32) CRC_1_POLYNOM_UPPER_C__D1_REG) << 8u;
            polynomial |= CRC_1_POLYNOM_UPPER_B__D1_REG;

        #else                                           /* 64 bits CRC */
            polynomial =  ((uint32) CRC_1_POLYNOM_UPPER_D__D1_REG) << 24u;
            polynomial |= ((uint32) CRC_1_POLYNOM_UPPER_C__D1_REG) << 16u;
            polynomial |= ((uint32) CRC_1_POLYNOM_UPPER_B__D1_REG) << 8u;
            polynomial |= CRC_1_POLYNOM_UPPER_A__D1_REG;

        #endif  /* End (CRC_1_CRC_SIZE <= 32u) */

        return polynomial;
    }


    /*******************************************************************************
    * Function Name: CRC_1_ReadPolynomialLower
    ********************************************************************************
    *
    * Summary:
    *  Reads lower half of CRC polynomial value. Only generated for 33-64-bit CRC.
    *
    * Parameters:
    *  void
    *
    * Return:
    *  Returns lower half of CRC polynomial value.
    *
    *******************************************************************************/
    uint32 CRC_1_ReadPolynomialLower(void) 
    {
        uint32 polynomial;

        /* Read Polynomial Lower */
        #if (CRC_1_CRC_SIZE <= 40u)          /* 40 bits CRC */
            polynomial =  ((uint32) CRC_1_POLYNOM_LOWER_B__D1_REG) << 24u;
            polynomial |= ((uint32) CRC_1_POLYNOM_LOWER_C__D0_REG) << 16u;
            polynomial |= ((uint32) CRC_1_POLYNOM_LOWER_B__D0_REG) << 8u;
            polynomial |= CRC_1_POLYNOM_LOWER_A__D0_REG;

        #elif (CRC_1_CRC_SIZE <= 48u)        /* 48 bits CRC */
            polynomial =  ((uint32) CRC_1_POLYNOM_LOWER_A__D1_REG) << 24u;
            polynomial |= ((uint32) CRC_1_POLYNOM_LOWER_C__D0_REG) << 16u;
            polynomial |= ((uint32) CRC_1_POLYNOM_LOWER_B__D0_REG) << 8u;
            polynomial |= CRC_1_POLYNOM_LOWER_A__D0_REG;

        #else                                           /* 64 bits CRC */
            polynomial =  ((uint32) CRC_1_POLYNOM_LOWER_D__D0_REG) << 24u;
            polynomial |= ((uint32) CRC_1_POLYNOM_LOWER_C__D0_REG) << 16u;
            polynomial |= ((uint32) CRC_1_POLYNOM_LOWER_B__D0_REG) << 8u;
            polynomial |= CRC_1_POLYNOM_LOWER_A__D0_REG;

        #endif  /* End (CRC_1_CRC_SIZE <= 32u) */

        return polynomial;
    }


    /*******************************************************************************
    * Function Name: CRC_1_WritePolynomialUpper
    ********************************************************************************
    *
    * Summary:
    *  Writes upper half of CRC polynomial value. Only generated for 33-64-bit CRC.
    *
    * Parameters:
    *  polynomial:  Upper half CRC polynomial value.
    *
    * Return:
    *  void
    *
    * Side Effects:
    *  The upper half of polynomial value is cut according to
    *  mask = 2^(Resolution - 32) - 1.
    *  For example if CRC Resolution is 35 bits the mask value is:
    *  2^(35 - 32) - 1 = 2^(3) - 1 = 0x0000 0007u.
    *  The upper half of polynomial value = 0x0000 00FFu is cut:
    *  upper half of polynomial & mask = 0x0000 00FFu & 0x0000 0007u = 0x0000 0007u.
    *
    *******************************************************************************/
    void CRC_1_WritePolynomialUpper(uint32 polynomial)
                                                
    {
        /* Mask the polynomial upper half to cut unused bits in case, when CRC size is not byte-aligned or equal 24 */
        #if((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u))
            polynomial &= CRC_1_MASK;
        #endif /* ((0u != (CRC_1_CRC_SIZE % 8u)) || (CRC_1_CRC_SIZE == 24u)) */

        /* Write Polynomial Upper */
        #if (CRC_1_CRC_SIZE <= 40u)          /* 40 bits CRC */
            CRC_1_POLYNOM_UPPER_C__D1_REG = LO8(polynomial);

        #elif (CRC_1_CRC_SIZE <= 48u)        /* 48 bits CRC */
            CRC_1_POLYNOM_UPPER_C__D1_REG = HI8(polynomial);
            CRC_1_POLYNOM_UPPER_B__D1_REG = LO8(polynomial);

        #elif (CRC_1_CRC_SIZE <= 56u)        /* 56 bits CRC */
            CRC_1_POLYNOM_UPPER_D__D1_REG = LO8(HI16(polynomial));
            CRC_1_POLYNOM_UPPER_C__D1_REG = HI8(polynomial);
            CRC_1_POLYNOM_UPPER_B__D1_REG = LO8(polynomial);

        #else                                           /* 64 bits CRC */
            CRC_1_POLYNOM_UPPER_D__D1_REG = HI8(HI16(polynomial));
            CRC_1_POLYNOM_UPPER_C__D1_REG = LO8(HI16(polynomial));
            CRC_1_POLYNOM_UPPER_B__D1_REG = HI8(polynomial);
            CRC_1_POLYNOM_UPPER_A__D1_REG = LO8(polynomial);

        #endif  /* End (CRC_1_CRC_SIZE <= 32u) */

        /* Resets triggers */
        CRC_1_EXECUTE_DFF_RESET;
    }


    /*******************************************************************************
    * Function Name: CRC_1_WritePolynomialLower
    ********************************************************************************
    *
    * Summary:
    *  Writes lower half of CRC polynomial value. Only generated for 33-64-bit CRC.
    *
    * Parameters:
    *  polynomial:  Lower half of CRC polynomial value.
    *
    * Return:
    *  void
    *
    *******************************************************************************/
    void CRC_1_WritePolynomialLower(uint32 polynomial)
                                                
    {
        /* Write Polynomial Lower */
        #if (CRC_1_CRC_SIZE <= 40u)          /* 40 bits CRC */
            CRC_1_POLYNOM_LOWER_B__D1_REG = HI8(HI16(polynomial));
            CRC_1_POLYNOM_LOWER_C__D0_REG = LO8(HI16(polynomial));
            CRC_1_POLYNOM_LOWER_B__D0_REG = HI8(polynomial);
            CRC_1_POLYNOM_LOWER_A__D0_REG = LO8(polynomial);

        #elif (CRC_1_CRC_SIZE <= 48u)        /* 48 bits CRC */
            CRC_1_POLYNOM_LOWER_A__D1_REG = HI8(HI16(polynomial));
            CRC_1_POLYNOM_LOWER_C__D0_REG = LO8(HI16(polynomial));
            CRC_1_POLYNOM_LOWER_B__D0_REG = HI8(polynomial);
            CRC_1_POLYNOM_LOWER_A__D0_REG = LO8(polynomial);

        #else                                           /* 64 bits CRC */
            CRC_1_POLYNOM_LOWER_D__D0_REG = HI8(HI16(polynomial));
            CRC_1_POLYNOM_LOWER_C__D0_REG = LO8(HI16(polynomial));
            CRC_1_POLYNOM_LOWER_B__D0_REG = HI8(polynomial);
            CRC_1_POLYNOM_LOWER_A__D0_REG = LO8(polynomial);

        #endif  /* End (CRC_1_CRC_SIZE <= 32u) */

        /* Resets triggers */
        CRC_1_EXECUTE_DFF_RESET;
    }
#endif  /* End (CRC_1_CRC_SIZE <= 32u) */


/* [] END OF FILE */
