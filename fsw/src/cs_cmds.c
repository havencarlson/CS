/************************************************************************
 ** File:
 **   $Id: cs_cmds.c 1.11 2017/03/30 16:05:39EDT mdeschu Exp  $
 **
 **   Copyright (c) 2007-2020 United States Government as represented by the 
 **   Administrator of the National Aeronautics and Space Administration. 
 **   All Other Rights Reserved.  
 **
 **   This software was created at NASA's Goddard Space Flight Center.
 **   This software is governed by the NASA Open Source Agreement and may be 
 **   used, distributed and modified only pursuant to the terms of that 
 **   agreement.
 **
 ** Purpose: 
 **   The CFS Checksum (CS) Application's commands for OS code segement,
 **   the cFE core code segment, and for CS in general
 ** 
 *************************************************************************/

/**************************************************************************
 **
 ** Include section
 **
 **************************************************************************/
#include "cfe.h"
#include "cs_app.h"
#include "cs_events.h"
#include "cs_cmds.h"
#include "cs_utils.h"
#include "cs_compute.h"

/**************************************************************************
 **
 ** Function Prototypes
 **
 **************************************************************************/


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS no operation command                                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_NoopCmd (const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);

    /* Verify command packet length */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )  
    {
        CS_AppData.HkPacket.CmdCounter++;
        
        CFE_EVS_SendEvent (CS_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                           "No-op command. Version %d.%d.%d.%d",
                           CS_MAJOR_VERSION,
                           CS_MINOR_VERSION,
                           CS_REVISION,
                           CS_MISSION_REV);
    }
    return;
} /* End of CS_NoopCmd () */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Reset Application counters command                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_ResetCmd (const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);

    /* Verify command packet length */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )  
    {
        CS_AppData.HkPacket.CmdCounter          = 0;
        CS_AppData.HkPacket.CmdErrCounter       = 0;
        
        CS_AppData.HkPacket.EepromCSErrCounter  = 0;
        CS_AppData.HkPacket.MemoryCSErrCounter  = 0;
        CS_AppData.HkPacket.TablesCSErrCounter  = 0;
        CS_AppData.HkPacket.AppCSErrCounter     = 0;
        CS_AppData.HkPacket.CfeCoreCSErrCounter = 0;
        CS_AppData.HkPacket.OSCSErrCounter      = 0;
        CS_AppData.HkPacket.PassCounter         = 0;        
        
        CFE_EVS_SendEvent (CS_RESET_DBG_EID, CFE_EVS_EventType_DEBUG,
                           "Reset Counters command recieved");
    }
    return;
} /* End of CS_ResetCmd () */


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS's background checksumming cycle                              */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_BackgroundCheckCycle (const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);
    bool                                    DoneWithCycle = false   ;
    bool                                    EndOfList = false   ;
    CFE_SB_MsgId_t MessageID = CFE_SB_INVALID_MSG_ID;;
    CFE_MSG_FcnCode_t  CommandCode = 0;
    size_t     ActualLength = 0; 

    CFE_MSG_GetSize(&BufPtr->Msg, &ActualLength);

    /* Verify the command packet length */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(&BufPtr->Msg, &MessageID);
        CFE_MSG_GetFcnCode(&BufPtr->Msg, &CommandCode);
        
        CFE_EVS_SendEvent(CS_LEN_ERR_EID,
                          CFE_EVS_EventType_ERROR,
                          "Invalid msg length: ID = 0x%08X, CC = %d, Len = %lu, Expected = %lu",
                          MessageID,
                          CommandCode,
                          (unsigned long) ActualLength,
                          (unsigned long) ExpectedLength);
    }    
    else
    {
        if (CS_AppData.HkPacket.ChecksumState == CS_STATE_ENABLED)
        {

            DoneWithCycle = false   ;
            EndOfList = false   ;

            /* Skip this background cycle if there's a recompute or one shot in
             * progress */
            if (CS_AppData.HkPacket.RecomputeInProgress == true || CS_AppData.HkPacket.OneShotInProgress == true)
            {
                CFE_EVS_SendEvent(CS_BKGND_COMPUTE_PROG_INF_EID,
                                  CFE_EVS_EventType_INFORMATION,
                                  "Skipping background cycle. Recompute or oneshot in progress.");
                DoneWithCycle = true;
            }

            /* We check for end-of-list because we don't necessarily know the
               order in which the table entries are defined, and we don't
               want to keep looping through the list */
            
            while ((DoneWithCycle != true   ) && (EndOfList != true   ))
            {
                /* We need to check the current table value here because
                   it is updated (and possibly reset to zero) inside each
                   function called */
                if (CS_AppData.HkPacket.CurrentCSTable >= (CS_NUM_TABLES - 1))
                {
                    EndOfList = true   ;
                }
                
                /* Call the appropriate background function based on the current table
                   value.  The value is updated inside each function */
                switch (CS_AppData.HkPacket.CurrentCSTable)
                {
                    case (CS_CFECORE):
                        DoneWithCycle = CS_BackgroundCfeCore();
                        break;
                        
                    case(CS_OSCORE):
                        
                        DoneWithCycle = CS_BackgroundOS();
                        break;
                        
                    case (CS_EEPROM_TABLE):
                        DoneWithCycle = CS_BackgroundEeprom();
                        break;
                        
                    case (CS_MEMORY_TABLE):
                        DoneWithCycle = CS_BackgroundMemory();
                        break;
                        
                    case (CS_TABLES_TABLE):
                        DoneWithCycle = CS_BackgroundTables();
                        break;
                        
                    case (CS_APP_TABLE):
                        
                        DoneWithCycle = CS_BackgroundApp();
                        break;
                        
                        /* default case in case CS_AppData.HkPacket.CurrentCSTable is some random bad value */
                    default:
                        
                        /* We are at the end of the line */
                        CS_AppData.HkPacket.CurrentCSTable = 0;
                        CS_AppData.HkPacket.CurrentEntryInTable = 0;
                        CS_AppData.HkPacket.PassCounter++;
                        DoneWithCycle = true   ;
                        break;
                        
                        
                }/* end switch */
            } /* end while */
        }
        else
        {
            /* CS is disabled, Application-wide */
        }
    }
    return;
} /* End of CS_BackgroundCheckCycle () */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Disable all background checksumming command                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_DisableAllCSCmd (const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);
    
    /* Verify command packet length */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )  
    {
        CS_AppData.HkPacket.ChecksumState = CS_STATE_DISABLED;
        
        /* zero out the temp values in all the tables
         so that when the checksums are re-enabled,
         they don't start with a half-old value */
        CS_ZeroEepromTempValues();
        CS_ZeroMemoryTempValues();
        CS_ZeroTablesTempValues();
        CS_ZeroAppTempValues();
        CS_ZeroCfeCoreTempValues();
        CS_ZeroOSTempValues();
        
        CS_AppData.HkPacket.CmdCounter++;
        
        CFE_EVS_SendEvent (CS_DISABLE_ALL_INF_EID,
                           CFE_EVS_EventType_INFORMATION,
                           "Background Checksumming Disabled");
    }
    return;
} /* End of CS_DisableAllCSCmd () */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Enable all background checksumming command                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_EnableAllCSCmd (const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);
    
    /* Verify command packet length */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )  
    {
        CS_AppData.HkPacket.ChecksumState = CS_STATE_ENABLED;
        
        CS_AppData.HkPacket.CmdCounter++;
        
        CFE_EVS_SendEvent (CS_ENABLE_ALL_INF_EID,
                           CFE_EVS_EventType_INFORMATION,
                           "Background Checksumming Enabled");
    }
    return;
} /* End of CS_EnableAllCSCmd () */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Disable background checking of the cFE core command          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_DisableCfeCoreCmd(const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);
    
    /* Verify command packet length */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )  
    {
        CS_AppData.HkPacket.CfeCoreCSState = CS_STATE_DISABLED;
        CS_ZeroCfeCoreTempValues();
        
#if (CS_PRESERVE_STATES_ON_PROCESSOR_RESET == true   )
        CS_UpdateCDS();
#endif
        
        CFE_EVS_SendEvent (CS_DISABLE_CFECORE_INF_EID,
                           CFE_EVS_EventType_INFORMATION, 
                           "Checksumming of cFE Core is Disabled");
        
        CS_AppData.HkPacket.CmdCounter++;
    }
    return;
} /* End of CS_DisableCfeCoreCmd () */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Enable background checking of the cFE core command           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_EnableCfeCoreCmd(const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);
    
    /* Verify command packet length */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )  
    {
        CS_AppData.HkPacket.CfeCoreCSState = CS_STATE_ENABLED;
        
#if (CS_PRESERVE_STATES_ON_PROCESSOR_RESET == true   )
        CS_UpdateCDS();
#endif
        
        CFE_EVS_SendEvent (CS_ENABLE_CFECORE_INF_EID,
                           CFE_EVS_EventType_INFORMATION, 
                           "Checksumming of cFE Core is Enabled");
        
        CS_AppData.HkPacket.CmdCounter++;
    }
    
    return;
    
} /* End of CS_EnableCfeCoreCmd () */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Disable background checking of the OS code segment command   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_DisableOSCmd(const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);
    
    /* Verify command packet length */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )  
    {
        CS_AppData.HkPacket.OSCSState = CS_STATE_DISABLED;
        CS_ZeroOSTempValues();
        
#if (CS_PRESERVE_STATES_ON_PROCESSOR_RESET == true   )
        CS_UpdateCDS();
#endif
        
        CFE_EVS_SendEvent (CS_DISABLE_OS_INF_EID,
                           CFE_EVS_EventType_INFORMATION, 
                           "Checksumming of OS code segment is Disabled");
        
        CS_AppData.HkPacket.CmdCounter++;
    }
    return;
} /* End of CS_DisableOSCmd () */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Enable background checking of the OS code segment command    */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_EnableOSCmd(const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);
    
    /* Verify command packet length */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )   
    {
        CS_AppData.HkPacket.OSCSState = CS_STATE_ENABLED;
        
#if (CS_PRESERVE_STATES_ON_PROCESSOR_RESET == true   )
        CS_UpdateCDS();
#endif
        
        CFE_EVS_SendEvent (CS_ENABLE_OS_INF_EID,
                           CFE_EVS_EventType_INFORMATION, 
                           "Checksumming of OS code segment is Enabled");
        
        CS_AppData.HkPacket.CmdCounter++;
    }
    return;
} /* End of CS_OSEnableCmd () */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Report the baseline checksum for the cFE core command        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_ReportBaselineCfeCoreCmd(const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);
    
    
    /* Verify command packet length */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )
    {
        if (CS_AppData.CfeCoreCodeSeg.ComputedYet == true   )
        {
            CFE_EVS_SendEvent (CS_BASELINE_CFECORE_INF_EID,
                               CFE_EVS_EventType_INFORMATION, 
                               "Baseline of cFE Core is 0x%08X", 
                               (unsigned int)CS_AppData.CfeCoreCodeSeg.ComparisonValue);
        }
        else
        {
            CFE_EVS_SendEvent (CS_NO_BASELINE_CFECORE_INF_EID,
                               CFE_EVS_EventType_INFORMATION, 
                               "Baseline of cFE Core has not been computed yet");   
        }
        CS_AppData.HkPacket.CmdCounter++;
    }
    return;
} /* End of CS_ReportBaselineCfeCoreCmd () */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Report the baseline checksum for the OS code segment command */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_ReportBaselineOSCmd(const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);
    
    /* Verify command packet length */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )
    {
        if (CS_AppData.OSCodeSeg.ComputedYet == true   )
        {
            CFE_EVS_SendEvent (CS_BASELINE_OS_INF_EID,
                               CFE_EVS_EventType_INFORMATION, 
                               "Baseline of OS code segment is 0x%08X", 
                               (unsigned int)CS_AppData.OSCodeSeg.ComparisonValue);
        }
        else
        {
            CFE_EVS_SendEvent (CS_NO_BASELINE_OS_INF_EID, 
                               CFE_EVS_EventType_INFORMATION, 
                               "Baseline of OS code segment has not been computed yet");   
        }
        CS_AppData.HkPacket.CmdCounter++;
    }
    return;
} /* End of CS_ReportBaselineOSCmd () */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Recompute the baseline checksum for the cFE core command     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_RecomputeBaselineCfeCoreCmd (const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);
    uint32              ChildTaskID;
    int32               Status;
    
    /* Verify command packet length... */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )
    {
        if (CS_AppData.HkPacket.RecomputeInProgress == false    && CS_AppData.HkPacket.OneShotInProgress == false   )
        {
            /* There is no child task running right now, we can use it*/
            CS_AppData.HkPacket.RecomputeInProgress           = true   ;
            
            /* fill in child task variables */
            CS_AppData.ChildTaskTable                = CS_CFECORE;
            CS_AppData.ChildTaskEntryID              = 0;
            CS_AppData.RecomputeEepromMemoryEntryPtr = &CS_AppData.CfeCoreCodeSeg;
            
            
            Status= CFE_ES_CreateChildTask(&ChildTaskID,
                                           CS_RECOMP_CFECORE_TASK_NAME,
                                           CS_RecomputeEepromMemoryChildTask,
                                           NULL,
                                           CFE_PLATFORM_ES_DEFAULT_STACK_SIZE,
                                           CS_CHILD_TASK_PRIORITY,
                                           0);
            
            if (Status == CFE_SUCCESS)
            {
                CFE_EVS_SendEvent (CS_RECOMPUTE_CFECORE_STARTED_DBG_EID,
                                   CFE_EVS_EventType_DEBUG, 
                                   "Recompute of cFE core started");
                CS_AppData.HkPacket.CmdCounter++;
            }
            else/* child task creation failed */
            {
                CFE_EVS_SendEvent (CS_RECOMPUTE_CFECORE_CREATE_CHDTASK_ERR_EID,
                                   CFE_EVS_EventType_ERROR,
                                   "Recompute cFE core failed, CFE_ES_CreateChildTask returned: 0x%08X",
                                   (unsigned int)Status);
                CS_AppData.HkPacket.CmdErrCounter++;
                CS_AppData.HkPacket.RecomputeInProgress = false   ;
            }
        }
        else
        {
            /*send event that we can't start another task right now */
            CFE_EVS_SendEvent (CS_RECOMPUTE_CFECORE_CHDTASK_ERR_EID,
                               CFE_EVS_EventType_ERROR,
                               "Recompute cFE core failed: child task in use");
            CS_AppData.HkPacket.CmdErrCounter++;
        }
    }
    return;
}/* end CS_RecomputeBaselineCfeCoreCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Recompute the baseline checksum for the OS code seg command  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_RecomputeBaselineOSCmd (const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);
    uint32              ChildTaskID;
    int32               Status;
    
    /* Verify command packet length... */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )
    {
        if (CS_AppData.HkPacket.RecomputeInProgress == false    && CS_AppData.HkPacket.OneShotInProgress == false   )
        {
            /* There is no child task running right now, we can use it*/
            CS_AppData.HkPacket.RecomputeInProgress                = true   ;
            
            /* fill in child task variables */
            CS_AppData.ChildTaskTable                = CS_OSCORE;
            CS_AppData.ChildTaskEntryID              = 0;
            CS_AppData.RecomputeEepromMemoryEntryPtr = &CS_AppData.OSCodeSeg;
            
            
            Status= CFE_ES_CreateChildTask(&ChildTaskID,
                                           CS_RECOMP_OS_TASK_NAME,
                                           CS_RecomputeEepromMemoryChildTask,
                                           NULL,
                                           CFE_PLATFORM_ES_DEFAULT_STACK_SIZE,
                                           CS_CHILD_TASK_PRIORITY,
                                           0);
            if (Status == CFE_SUCCESS)
            {
                CFE_EVS_SendEvent (CS_RECOMPUTE_OS_STARTED_DBG_EID, 
                                   CFE_EVS_EventType_DEBUG, 
                                   "Recompute of OS code segment started");
                CS_AppData.HkPacket.CmdCounter++;
            }
            else/* child task creation failed */
            {
                CFE_EVS_SendEvent (CS_RECOMPUTE_OS_CREATE_CHDTASK_ERR_EID,
                                   CFE_EVS_EventType_ERROR,
                                   "Recompute OS code segment failed, CFE_ES_CreateChildTask returned: 0x%08X",
                                   (unsigned int)Status);
                CS_AppData.HkPacket.CmdErrCounter++;
                CS_AppData.HkPacket.RecomputeInProgress = false   ;
            }
        }
        else
        {
            /*send event that we can't start another task right now */
            CFE_EVS_SendEvent (CS_RECOMPUTE_OS_CHDTASK_ERR_EID,
                               CFE_EVS_EventType_ERROR,
                               "Recompute OS code segment failed: child task in use");
            CS_AppData.HkPacket.CmdErrCounter++;
        }
    }
    return;
}/* end CS_RecomputeBaselineOSCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Compute the OneShot checksum command                         */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_OneShotCmd (const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_OneShotCmd_t);
    uint32              ChildTaskID;
    int32               Status;
    CS_OneShotCmd_t   * CmdPtr;
    
    /* Verify command packet length... */    
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )
    {
        CmdPtr = (CS_OneShotCmd_t*) BufPtr;
        
        /* validate size and address */
        Status = CFE_PSP_MemValidateRange(CmdPtr -> Address, CmdPtr -> Size, CFE_PSP_MEM_ANY);
        
        if (Status == CFE_SUCCESS)
        {
            if (CS_AppData.HkPacket.RecomputeInProgress == false    && CS_AppData.HkPacket.OneShotInProgress == false   )
            {
                /* There is no child task running right now, we can use it*/
                CS_AppData.HkPacket.RecomputeInProgress                   = false   ;
                CS_AppData.HkPacket.OneShotInProgress                 = true   ;
                
                CS_AppData.HkPacket.LastOneShotAddress   = CmdPtr -> Address;
                CS_AppData.HkPacket.LastOneShotSize      = CmdPtr -> Size;
                if (CmdPtr -> MaxBytesPerCycle == 0)
                {
                    CS_AppData.HkPacket.LastOneShotMaxBytesPerCycle    = CS_AppData.MaxBytesPerCycle;
                }
                else
                {
                    CS_AppData.HkPacket.LastOneShotMaxBytesPerCycle    = CmdPtr -> MaxBytesPerCycle;
                }

                CS_AppData.HkPacket.LastOneShotChecksum  = 0;
                
                Status = CFE_ES_CreateChildTask(&ChildTaskID,
                                                CS_ONESHOT_TASK_NAME,
                                                CS_OneShotChildTask,
                                                NULL,
                                                CFE_PLATFORM_ES_DEFAULT_STACK_SIZE,
                                                CS_CHILD_TASK_PRIORITY,
                                                0);
                if (Status == CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent (CS_ONESHOT_STARTED_DBG_EID,
                                       CFE_EVS_EventType_DEBUG,
                                       "OneShot checksum started on address: 0x%08X, size: %d",
                                       (unsigned int)(CmdPtr -> Address),
                                       (int)(CmdPtr -> Size));
                    
                    CS_AppData.ChildTaskID = ChildTaskID;
                    CS_AppData.HkPacket.CmdCounter++;
                }
                else/* child task creation failed */
                {
                    CFE_EVS_SendEvent (CS_ONESHOT_CREATE_CHDTASK_ERR_EID,
                                       CFE_EVS_EventType_ERROR,
                                       "OneShot checkum failed, CFE_ES_CreateChildTask returned: 0x%08X",
                                       (unsigned int)Status);
                    
                    CS_AppData.HkPacket.CmdErrCounter++;
                    CS_AppData.HkPacket.RecomputeInProgress   = false   ;
                    CS_AppData.HkPacket.OneShotInProgress = false   ;
                }
            }
            else
            {
                /*send event that we can't start another task right now */
                CFE_EVS_SendEvent (CS_ONESHOT_CHDTASK_ERR_EID,
                                   CFE_EVS_EventType_ERROR,
                                   "OneShot checksum failed: child task in use");
                
                CS_AppData.HkPacket.CmdErrCounter++;
            }
        }/* end if CFE_PSP_MemValidateRange */
        else
        {
            CFE_EVS_SendEvent (CS_ONESHOT_MEMVALIDATE_ERR_EID,
                               CFE_EVS_EventType_ERROR,
                               "OneShot checksum failed, CFE_PSP_MemValidateRange returned: 0x%08X",
                               (unsigned int)Status);
            
            CS_AppData.HkPacket.CmdErrCounter++;
        }
    }
    return;
}/* end CS_OneShotCmd */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* CS Cancel the OneShot checksum command                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void CS_CancelOneShotCmd (const CFE_SB_Buffer_t* BufPtr)
{
    /* command verification variables */
    size_t ExpectedLength = sizeof(CS_NoArgsCmd_t);
    int32               Status;
    
    /* Verify command packet length... */
    if ( CS_VerifyCmdLength (&BufPtr->Msg, ExpectedLength) )
    {
        /* Make sure there is a OneShot command in use */
        if (CS_AppData.HkPacket.RecomputeInProgress == false     && CS_AppData.HkPacket.OneShotInProgress == true   )
        {
            Status= CFE_ES_DeleteChildTask(CS_AppData.ChildTaskID);
            
            if (Status == CFE_SUCCESS)
            {
                CS_AppData.ChildTaskID          = 0;
                CS_AppData.HkPacket.RecomputeInProgress       = false   ;
                CS_AppData.HkPacket.OneShotInProgress     = false   ;
                CS_AppData.HkPacket.CmdCounter++;
                CFE_EVS_SendEvent (CS_ONESHOT_CANCELLED_INF_EID,
                                   CFE_EVS_EventType_INFORMATION,
                                   "OneShot checksum calculation has been cancelled");
            }
            else
            {
                CFE_EVS_SendEvent (CS_ONESHOT_CANCEL_DELETE_CHDTASK_ERR_EID,
                                   CFE_EVS_EventType_ERROR,
                                   "Cancel OneShot checksum failed, CFE_ES_DeleteChildTask returned:  0x%08X",
                                   (unsigned int)Status);
                CS_AppData.HkPacket.CmdErrCounter++;
            }
        }
        else
        {
            CFE_EVS_SendEvent (CS_ONESHOT_CANCEL_NO_CHDTASK_ERR_EID,
                               CFE_EVS_EventType_ERROR,
                               "Cancel OneShot checksum failed. No OneShot active");
            CS_AppData.HkPacket.CmdErrCounter++;
        }
    }
    return;
}/* end CS_CancelOneShotCmd */


/************************/
/*  End of File Comment */
/************************/
