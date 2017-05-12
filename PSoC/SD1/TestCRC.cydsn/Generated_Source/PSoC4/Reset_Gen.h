/*******************************************************************************
* File Name: Reset_Gen.h  
* Version 1.70
*
* Description:
*  This file containts Control Register function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CONTROL_REG_Reset_Gen_H) /* CY_CONTROL_REG_Reset_Gen_H */
#define CY_CONTROL_REG_Reset_Gen_H

#include "cytypes.h"


/***************************************
*         Function Prototypes 
***************************************/

void    Reset_Gen_Write(uint8 control) ;
uint8   Reset_Gen_Read(void) ;


/***************************************
*            Registers        
***************************************/

/* Control Register */
#define Reset_Gen_Control        (* (reg8 *) Reset_Gen_Sync_ctrl_reg__CONTROL_REG )
#define Reset_Gen_Control_PTR    (  (reg8 *) Reset_Gen_Sync_ctrl_reg__CONTROL_REG )

#endif /* End CY_CONTROL_REG_Reset_Gen_H */


/* [] END OF FILE */
