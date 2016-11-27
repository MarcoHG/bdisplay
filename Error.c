//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  MODULE: Error.c
//------------------------------------------------------------------------------
/************************************************************************/
/*                                                                      */
/*              DEVELOPED BY:   West Coast Controls Co                  */
/*                              17150 Septo st,                         */
/*                              Northridge CA 91325                     */
/*                              (818) 718 1742                          */
/*                              www.wccontrols.com                      */
/*                                                                      */
/************************************************************************/

//=======================
// INCLUDES
//=======================  
#include <stdio.h>
#include "define.h"
#include "Error.h"

//========================
//  LOCAL DEFINES
//========================
 
//================================
//  LOCAL PROTOTYPES. 
//================================

//========================
//  GLOBAL DATA
//========================
TError Errors;                  /* the storage location for all error-related info */

//========================
//  LOCAL DATA
//========================
 
//==============================================================================
// FUNCTIONS
//==============================================================================  
 
//==============================================================================
// Routine Name: ErrorInit
// Date Created: February 11, 2008 
// Author: EMH
// Description: 
//------------------------------------------------------------------------------
void ErrorInit(void)
{
  Errors.WarningCode = 0;
  Errors.ErrorCode = 0;
  Errors.FailureCode = 0;
  Errors.StatusCode = 0;
  Errors.fErrorFlag = FALSE;
  // OptionsCheck();           // Check instrument options are programmed (False or True)
}



//==============================================================================
// Routine Name: ErrorTimeUpdate
// Date Created: August 22, 2001
// Author: SLTF
// Description: This function will monitor the ErrorCode flag and will launch
//                an event if there are any errors. executed every 1 second. 
// Input: None
// Output: None
//------------------------------------------------------------------------------
void ErrorTimeUpdate(void)
{
  Errors.StatusCode = 0x00; /* a compendium of instrument state */
  if (Errors.fErrorFlag == TRUE) 
  {
    Errors.StatusCode |= StatusErrors;
  }
  #if 0
  if (Alarms[0].Condition == TRUE) 
  {
     Errors.StatusCode |= StatusAlarm1;
  }
  if (Alarms[1].Condition == TRUE) 
  {
    Errors.StatusCode |= StatusAlarm2;
  }
  if (SensorRam.fCalError == TRUE) 
  {
    Errors.StatusCode |= StatusCalError;
  }
  //---------------------------
  // look for any MiCos errors 
  if (MiFatalError != 0) 
  {
    ErrorFlag(TRUE, FailureMicosQueueOverflow);
  }
  //----------------------------
  // see if any errors in the system 
  if (Errors.FailureCode != FailureNone) 
  {
    MiEventWrite(EV_ERROR_FATAL);
  }
  
  //V2.3.13 ----- BOC 
  if ((Errors.ErrorCode!= 0)||(Errors.FailureCode != 0))
  {
    AlarmOverrideOn(); 
    AnalogErrorOn();  
  }
  else
  {
      AlarmOverrideOff();  
      AnalogErrorOff();
  }  
  //V2.3.13 ---- EOC  
  #endif
}

//==============================================================================
// Routine Name: ErrorFlag
// Date Created: September 6, 2001
// Author: SLTF
// Description: This will allow other functions to set or clear an "error"
//                flag, error meaning Warning, Error, or Failure.
// Input: fFlag - TRUE to set and FALSE to clear
//                Number - the error code
// Output: 
//------------------------------------------------------------------------------
void ErrorFlag(BOOLEAN fFlag, WORD Number)
{
#if 0
  WORD HashType, HashFlag;
  BOOLEAN fBefore, fAfter;
  WORD *CodePtr, Code;
  MICOS_EVENT EventExist, EventNonexist;

  HashType = (Number & ERROR_MASK_TYPE);
  HashFlag = (Number & ERROR_MASK_FLAGS);
  switch (HashType) 
  {
    case ERROR_TYPE_WARNING:
      CodePtr = &Errors.WarningCode;
      EventExist = EV_WARNING_SET;
      EventNonexist = EV_WARNING_CLEAR;
      break;

    case ERROR_TYPE_ERROR:
      CodePtr = &Errors.ErrorCode;
      EventExist = EV_ERROR_SET;
      EventNonexist = EV_ERROR_CLEAR;
      break;

    case ERROR_TYPE_FAILURE:
      CodePtr = &Errors.FailureCode;
      EventExist = EV_FAILURE_SET;
      EventNonexist = EV_FAILURE_CLEAR;
      break;
  }

/* change the code based on the flag */
  Code = *CodePtr;
  fBefore = (Code == 0);
  Code &= ~HashFlag;
  if (fFlag == TRUE) {
    Code |= HashFlag;
  }
  fAfter = (Code == 0);
  *CodePtr = Code;

/* see if we have a change in state of the error detection stuff */
  if ((fBefore == TRUE) && (fAfter == FALSE)) 
  {
    MiEventWrite(EventExist);
    if ((EventExist == EV_ERROR_SET) || (EventExist == EV_FAILURE_SET))
    {
      AlarmOverrideOn(); 
      AnalogErrorOn();
    }  
  }
  if ((fBefore == FALSE) && (fAfter == TRUE)) 
  {
    MiEventWrite(EventNonexist);
    if ((EventExist == EV_ERROR_CLEAR) || (EventExist == EV_FAILURE_CLEAR))
    {
      AlarmOverrideOff();  
      AnalogErrorOff();
    }  
  }
#endif
/* set the global error flag, if there is an error */
  Errors.fErrorFlag = (Errors.ErrorCode == 0) ? FALSE : TRUE;
}

