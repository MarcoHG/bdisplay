#ifndef __ERROR_H
#define __ERROR_H

/*************************************************************************
*   $MODULE
*
*
*************************************************************************/

/*************************************************************************
*   $INCLUDES
*************************************************************************/

/*************************************************************************
*   $DEFINES
*************************************************************************/
#define ERROR_MASK_TYPE           0xf000
#define ERROR_MASK_FLAGS          0x0fff

#define ERROR_TYPE_WARNING        0x1000
#define WarningNone               0x0000
#define WarningHumidity           (0x0001 | ERROR_TYPE_WARNING)

#define ERROR_TYPE_ERROR          0x2000
#define ErrorNone                 0x0000
#define ErrorHumidity             (0x0001 | ERROR_TYPE_ERROR)

#define ERROR_TYPE_FAILURE        0x3000
#define FailureNone               0x0000
#define FailureADC                (0x0001 | ERROR_TYPE_FAILURE)
#define FailureFlashRead          (0x0002 | ERROR_TYPE_FAILURE)
#define FailureFlashWrite         (0x0004 | ERROR_TYPE_FAILURE)
#define FailureFlashWriteByte     (0x0080 | ERROR_TYPE_FAILURE)

#define StatusErrors              0x0001
#define StatusAlarm1              0x0002
#define StatusAlarm2              0x0004
#define StatusCalError            0x0008

/*************************************************************************
*   $GLOBAL PROTOTYPES
*************************************************************************/
typedef struct {
  WORD ErrorCode;
  WORD WarningCode;
  WORD FailureCode;
  WORD StatusCode;              /* a compendium of instrument information */
  BOOLEAN fErrorFlag;
} TError;

// No MiCos TMiResult stateError(void);
void ErrorInit(void);
void ErrorTimeUpdate(void);
void ErrorFlag(BOOLEAN fFlag, WORD Number);

/*************************************************************************
*   $GLOBAL VARIABLES
*************************************************************************/
extern TError Errors;

/************************************************************************/

#endif
