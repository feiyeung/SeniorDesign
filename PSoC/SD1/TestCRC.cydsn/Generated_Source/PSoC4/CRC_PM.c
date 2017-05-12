/*******************************************************************************
* File Name: CRC_PM.c
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

#include "CRC.h"


/*******************************************************************************
* Function Name: CRC_SaveConfig
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
*  CRC_backup - modified when non-retention registers are saved.
*
*******************************************************************************/
void CRC_SaveConfig(void) 
{       
    /* Save D0 and D1 registers are none-retention for Panther ES1 */
    #if (CY_UDB_V0)
    
        #if(CRC_CRC_SIZE <= 32u)
            CRC_backup.polynomial = CRC_ReadPolynomial();
            
        #else
            CRC_backup.polynomialUpper = CRC_ReadPolynomialUpper();
            CRC_backup.polynomialLower = CRC_ReadPolynomialLower();
            
        #endif  /* End (CRC_CRC_SIZE <= 32u) */
        
    #endif  /* End (CY_UDB_V0) */
}


/*******************************************************************************
* Function Name: CRC_Sleep
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
*  CRC_backup - modified when non-retention registers are saved.
*
*******************************************************************************/
void CRC_Sleep(void) 
{
    /* Store CRC enable state */
    if(0u != (CRC_CONTROL_REG & CRC_CTRL_ENABLE))
    {
        CRC_backup.enableState = 1u;
        CRC_Stop();
    }
    else
    {
        CRC_backup.enableState = 0u;
    }
    
    CRC_SaveConfig();
}


/*******************************************************************************
* Function Name: CRC_RestoreConfig
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
void CRC_RestoreConfig(void) 
{      
    /* Restore D0 and D1 registers are none-retention for Panther ES1 */
    #if (CY_UDB_V0)
    
        #if(CRC_CRC_SIZE <= 32u)
            CRC_WritePolynomial(CRC_backup.polynomial);
            
        #else
            CRC_WritePolynomialUpper(CRC_backup.polynomialUpper);
            CRC_WritePolynomialLower(CRC_backup.polynomialLower);
            
        #endif  /* End (CY_UDB_V0) */
        
    #endif  /* End (CRC_CRC_SIZE <= 32u) */
    
    #if (CRC_CRC_SIZE <= 32u)
        CRC_WriteSeed(CRC_DEFAULT_SEED);                        
    #else
        CRC_WriteSeedUpper(CRC_DEFAULT_SEED_UPPER);
        CRC_WriteSeedLower(CRC_DEFAULT_SEED_LOWER); 
    #endif  /* End (CRC_CRC_SIZE <= 32u) */
    
    #if (CRC_CRC_SIZE <= 32u)
        CRC_ResetSeedInit(CRC_DEFAULT_SEED);                        
    #else
        CRC_ResetSeedInitUpper(CRC_DEFAULT_SEED_UPPER);
        CRC_ResetSeedInitLower(CRC_DEFAULT_SEED_LOWER); 
    #endif  /* End (CRC_CRC_SIZE <= 32u) */
}


/*******************************************************************************
* Function Name: CRC_Wakeup
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
void CRC_Wakeup(void) 
{
    CRC_RestoreConfig();
    
    /* Restore CRC enable state */
    if (CRC_backup.enableState != 0u)
    {
        CRC_Enable();
    }
}


/* [] END OF FILE */
