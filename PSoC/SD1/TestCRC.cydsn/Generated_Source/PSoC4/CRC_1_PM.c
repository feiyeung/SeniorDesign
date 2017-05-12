/*******************************************************************************
* File Name: CRC_1_PM.c
* Version 2.40
*
* Description:
*  This file provides Sleep APIs for CRC component.
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


/*******************************************************************************
* Function Name: CRC_1_SaveConfig
********************************************************************************
*
* Summary:
*  Saves seed and polynomial registers.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global Variables:
*  CRC_1_backup - modified when non-retention registers are saved.
*
*******************************************************************************/
void CRC_1_SaveConfig(void) 
{       
    /* Save D0 and D1 registers are none-retention for Panther ES1 */
    #if (CY_UDB_V0)
    
        #if(CRC_1_CRC_SIZE <= 32u)
            CRC_1_backup.polynomial = CRC_1_ReadPolynomial();
            
        #else
            CRC_1_backup.polynomialUpper = CRC_1_ReadPolynomialUpper();
            CRC_1_backup.polynomialLower = CRC_1_ReadPolynomialLower();
            
        #endif  /* End (CRC_1_CRC_SIZE <= 32u) */
        
    #endif  /* End (CY_UDB_V0) */
}


/*******************************************************************************
* Function Name: CRC_1_Sleep
********************************************************************************
*
* Summary:
*  Stops CRC computation and saves CRC configuration.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global Variables:
*  CRC_1_backup - modified when non-retention registers are saved.
*
*******************************************************************************/
void CRC_1_Sleep(void) 
{
    /* Store CRC enable state */
    if(0u != (CRC_1_CONTROL_REG & CRC_1_CTRL_ENABLE))
    {
        CRC_1_backup.enableState = 1u;
        CRC_1_Stop();
    }
    else
    {
        CRC_1_backup.enableState = 0u;
    }
    
    CRC_1_SaveConfig();
}


/*******************************************************************************
* Function Name: CRC_1_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores seed and polynomial registers.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void CRC_1_RestoreConfig(void) 
{      
    /* Restore D0 and D1 registers are none-retention for Panther ES1 */
    #if (CY_UDB_V0)
    
        #if(CRC_1_CRC_SIZE <= 32u)
            CRC_1_WritePolynomial(CRC_1_backup.polynomial);
            
        #else
            CRC_1_WritePolynomialUpper(CRC_1_backup.polynomialUpper);
            CRC_1_WritePolynomialLower(CRC_1_backup.polynomialLower);
            
        #endif  /* End (CY_UDB_V0) */
        
    #endif  /* End (CRC_1_CRC_SIZE <= 32u) */
    
    #if (CRC_1_CRC_SIZE <= 32u)
        CRC_1_WriteSeed(CRC_1_DEFAULT_SEED);                        
    #else
        CRC_1_WriteSeedUpper(CRC_1_DEFAULT_SEED_UPPER);
        CRC_1_WriteSeedLower(CRC_1_DEFAULT_SEED_LOWER); 
    #endif  /* End (CRC_1_CRC_SIZE <= 32u) */
    
    #if (CRC_1_CRC_SIZE <= 32u)
        CRC_1_ResetSeedInit(CRC_1_DEFAULT_SEED);                        
    #else
        CRC_1_ResetSeedInitUpper(CRC_1_DEFAULT_SEED_UPPER);
        CRC_1_ResetSeedInitLower(CRC_1_DEFAULT_SEED_LOWER); 
    #endif  /* End (CRC_1_CRC_SIZE <= 32u) */
}


/*******************************************************************************
* Function Name: CRC_1_Wakeup
********************************************************************************
*
* Summary:
*  Restores CRC configuration and starts CRC computation. 
*
* Parameters:
*  void
*
* Return:
*  void
* 
*******************************************************************************/
void CRC_1_Wakeup(void) 
{
    CRC_1_RestoreConfig();
    
    /* Restore CRC enable state */
    if (CRC_1_backup.enableState != 0u)
    {
        CRC_1_Enable();
    }
}


/* [] END OF FILE */
