/************************************************************************
 ** File:
 **   $Id: cs_cmds.h 1.4 2017/03/15 16:54:58EDT mdeschu Exp  $
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
 **   Specification for the CFS generic cmds
 **
 ** References:
 **   Flight Software Branch C Coding Standard Version 1.2
 **   CFS Development Standards Document
 **   CFS CS Heritage Analysis Document
 **   CFS CS CDR Package
 ** 
 *************************************************************************/
#ifndef _cs_cmds_
#define _cs_cmds_

/**************************************************************************
 **
 ** Include section
 **
 **************************************************************************/
#include "cfe.h"


/************************************************************************/
/** \brief Process noop command
 **  
 **  \par Description
 **       Processes a noop ground command.
 **
 **  \par Assumptions, External Events, and Notes:
 **       None
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_NOOP_CC
 **
 *************************************************************************/
void CS_NoopCmd (const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Process reset counters command
 **  
 **  \par Description
 **       Processes a reset counters ground command which will reset
 **       the checksum commmand error and command execution counters
 **       to zero. It also resets all checksum error counters and
 **       the passes completed counter.
 **
 **  \par Assumptions, External Events, and Notes:
 **       None
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_RESET_CC
 **
 *************************************************************************/
void CS_ResetCmd (const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief process a background checking cycle
 **  
 **  \par Description
 **       Processes a background checking cycle when the scheduler 
 **       tell CS.
 **
 **  \par Assumptions, External Events, and Notes:
 **       None
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 *************************************************************************/
void CS_BackgroundCheckCycle (const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Process a disable overall background checking command
 **  
 **  \par Description
 **     Disables all background checking in CS
 **       
 **
 **  \par Assumptions, External Events, and Notes:
 **       
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_DISABLE_ALL_CS_CC
 **
 *************************************************************************/
void CS_DisableAllCSCmd (const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Process a enable overall background checking command 
 **  
 **  \par Description
 **       Allows background checking to take place.
 **       
 **  \par Assumptions, External Events, and Notes:
 **       None
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_ENABLE_ALL_CS_CC
 **
 *************************************************************************/
void CS_EnableAllCSCmd (const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Process a disable background checking for the cFE core 
 **        code segment command 
 **  
 **  \par Description
 **       Disables background checking for the cFE core code segment
 **       
 **  \par Assumptions, External Events, and Notes:
 **       In order for background checking of individual areas
 **       to checksum (OS code segment, cFE core, Eeprom, Memory,
 **       Apps, and Tables) to occurr, the table must be enabled
 **       and overall checksumming must be enabled.
 **
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_DISABLE_CFECORE_CC
 **
 *************************************************************************/
void CS_DisableCfeCoreCmd(const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Process a enable background checking for the cFE core code
 **        segment command
 **  
 **  \par Description
 **       Allows the cFE Core code segment to be background checksummed.
 **
 **  \par Assumptions, External Events, and Notes:
 **       In order for background checking of individual areas
 **       to checksum (OS code segment, cFE core, Eeprom, Memory,
 **       Apps, and Tables) to occurr, the table must be enabled
 **       and overall checksumming must be enabled.
 **      
 **
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_ENABLE_CFECORE_CC
 **
 *************************************************************************/
void CS_EnableCfeCoreCmd(const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Process a disable background checking for the OS code
 **        segment command
 **  
 **  \par Description
 **       Disables background checking for the OS code segment 
 **       
 **  \par Assumptions, External Events, and Notes:
 **       In order for background checking of individual areas
 **       to checksum (OS code segment, cFE core, Eeprom, Memory,
 **       Apps, and Tables) to occurr, the table must be enabled
 **       and overall checksumming must be enabled.
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_DISABLE_OS_CC
 **
 *************************************************************************/
void CS_DisableOSCmd(const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Process a enable background checking for the OS code 
 **        segment command 
 **  
 **  \par Description
 **       Allows the OS code segment to be background checksummed.
 **
 **  \par Assumptions, External Events, and Notes:
 **       In order for background checking of individual areas
 **       to checksum (OS code segment, cFE core, Eeprom, Memory,
 **       Apps, and Tables) to occurr, the table must be enabled
 **       and overall checksumming must be enabled.
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 ** 
 **  \sa #CS_ENABLE_OS_CC
 **
 *************************************************************************/
void CS_EnableOSCmd(const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Process a report baseline of the cFE Core command 
 **  
 **  \par Description
 **        Reports the baseline checksum of the cFE core code segment
 **        if it has already been computed
 **       
 **  \par Assumptions, External Events, and Notes:
 **       None
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_REPORT_BASELINE_CFECORE_CC
 **
 *************************************************************************/
void CS_ReportBaselineCfeCoreCmd(const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Proccess a report baseline of the OS command 
 **  
 **  \par Description
 **        Reports the baseline checksum of the OS code segment
 **        if it has already been computed
 **       
 **  \par Assumptions, External Events, and Notes:
 **        None
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_REPORT_BASELINE_OS_CC
 **
 *************************************************************************/
void CS_ReportBaselineOSCmd(const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Process a recopmute baseline of the cFE core code segment command
 **  
 **  \par Description
 **        Recomputes the checksum of the cFE core code segment and use that 
 **        value as the new baseline for the cFE core.
 **        
 **  \par Assumptions, External Events, and Notes:
 **       None
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_RECOMPUTE_BASELINE_CFECORE_CC
 **
 *************************************************************************/
void CS_RecomputeBaselineCfeCoreCmd (const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Process a recopmute baseline of the OS command
 **  
 **  \par Description
 **        Recomputes the checksum of the OS code segment and use that 
 **        value as the new baseline for the OS.
 **        
 **  \par Assumptions, External Events, and Notes:
 **       None
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_RECOMPUTE_BASELINE_OS_CC
 **
 *************************************************************************/
void CS_RecomputeBaselineOSCmd (const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Process a start a one shot checksum command
 **  
 **  \par Description
 **        Starts a one shot checksum on given address and size, 
 **        and reports checksum in telemetry and an event message.
 **
 **  \par Assumptions, External Events, and Notes:
 **
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_ONESHOT_CC
 **
 *************************************************************************/
void CS_OneShotCmd (const CFE_SB_Buffer_t* BufPtr);

/************************************************************************/
/** \brief Process a cancel one shot checksum command 
 **  
 **  \par Description
 **        Cancel a one shot command, if a one shot calculation is 
 **        taking place
 **
 **  \par Assumptions, External Events, and Notes:
 **
 **       
 **  \param [in]   BufPtr   A #CFE_SB_Buffer_t* pointer that
 **                         references the software bus message. The
 **                         BufPtr is verified non-null in CS_AppMain.
 **
 **  \sa #CS_CANCEL_ONESHOT_CC
 **
 *************************************************************************/
void CS_CancelOneShotCmd (const CFE_SB_Buffer_t* BufPtr);


#endif /* _cs_cmds_ */
/************************/
/*  End of File Comment */
/************************/
