#ifndef __COUNTER_API_H
#define __COUNTER_API_H
/************************************************************************/
/*                                                                      */
/*              DEVELOPED BY:   West Coast Controls Co                  */
/*                              17150 Septo st,                         */
/*                              Northridge CA 91325                     */
/*                              (818) 718 1742                          */
/*                              www.wccontrols.com                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/

/*************************************************************************
*   $MODULE
*   apiCounter.h
*
*************************************************************************/

/*************************************************************************
*   $INCLUDES
*************************************************************************/

/*************************************************************************
*   $DEFINES
*************************************************************************/
#define MAX_GOAL_COUNT 99999L

//  This structure has the "NonVolatile" data of Counter
typedef struct
{
  long int Goal;      //  Preset Counter to achieve Goal
  WORD MinPace;       //  Bellow this value, machine is considered STOP
  WORD IdleSecTime;   //  Seconds to generate a "Stop" event
  WORD PaceSecBase;   //  Pace is calculated in counts/PaceTimeBase
} TCounterUserData;     

typedef struct
{
  long int  Value;       //  Actual count value 
  WORD      Rate;
  WORD      IdleMs;
                        // This section is the Working and NV User Data
  TCounterUserData 
      WorkingUserData;    
  BOOLEAN DirtyFlag;
  WORD  AutoSaveSecs;   // Seconds for performing autosave
}TCounter;
/*************************************************************************
*   $LOCAL PROTOTYPES
*************************************************************************/

/*************************************************************************
*   $GLOBAL PROTOTYPES
*************************************************************************/
void CounterTask(void);
void CounterInit(void);
void CounterResetData(void);
void AutoSaveCounterUserData(void);

extern TCounter Counter;
extern long int RcCounter;

/*************************************************************************
*   $LOCAL VARIABLES
*************************************************************************/

#endif
