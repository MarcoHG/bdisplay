/************************************************************************/
/*                                                                      */
/*              DEVELOPED BY:   West Coast Controls Co                  */
/*                              17150 Septo st,                         */
/*                              Northridge CA 91325                     */
/*                              (818) 718 1742                          */
/*                              www.wccontrols.com                      */
/*                                                                      */
/************************************************************************/

/*************************************************************************
*   $MODULE
*   apiCounter.c
*
*************************************************************************/

/*************************************************************************
*   $INCLUDES
*************************************************************************/
#include "define.h"
#include "msp_port.h"
#include "apiCounter.h"
#include "driverTimer.h"
#include "driverFlash.h"
#include <string.h>
/**********************************************//***************************
*   $DEFINES
*************************************************************************/

//========================
//  LOCAL DEFINES
//========================



//================================
//  LOCAL PROTOTYPES. 
//================================

/*************************************************************************
*   $LOCAL PROTOTYPES
*************************************************************************/

/*************************************************************************
*   $GLOBAL VARIABLES
*************************************************************************/
TCounter Counter;
long int RcCounter;
//========================
//  LOCAL DATA
//========================
//  Globals are initialized to zero in ANSI C, i.e. linker will locate to RAM, we initialize to any value 
//  and then set load section to "No" in linker placement file
#pragma DATA_SECTION(NonVolatileUserData, ".infoD")
TCounterUserData NonVolatileUserData;// = {0};  // Dummy initializer, force compiler not to use initialized segment



static void CounterLegal(void)
{
  BOOLEAN fReset = FALSE;   // Need reset data

}

//  Routine   AutoSaveCounterUserData()
//
//  Called from ClockManager, every second
//  Handles the saving of Counter's UserData to NonVolatile
//
//  There are two versions of UserData: Working and NonVolatile.
//  Once the Working version changes, the DirtFlag is Set and when
//  Working data remains unchanged for Counter.AutoSaveSecs secs
//  the Working version is saved in Flash.
//
void AutoSaveCounterUserData(void)
{
  static WORD AutoSaveSecs=0;
  static TCounterUserData AutoSaveUserData;   // Buffer the Working version of UserData

  // Handle changes in UserData and Save if Necessary
  if(memcmp(&Counter.WorkingUserData, &NonVolatileUserData, sizeof(TCounterUserData)) ==0)
  {
    Counter.DirtyFlag = FALSE;
    AutoSaveUserData = Counter.WorkingUserData; // Make a local copy
    AutoSaveSecs =0;
  }
  else
  if( memcmp(&Counter.WorkingUserData, &AutoSaveUserData, sizeof(TCounterUserData)))
  {
    AutoSaveUserData = Counter.WorkingUserData; // Make a local copy
    AutoSaveSecs =0;
  }
  else
  if(++AutoSaveSecs >= Counter.AutoSaveSecs)
  {
    // Save to Flash
    FlashWrite(&NonVolatileUserData,&Counter.WorkingUserData, sizeof(TCounterUserData));
    Counter.DirtyFlag = FALSE;
        
  }
}


void CounterUserDataDefaults(void)
{
  TCounterUserData Defaults;
  
  Defaults.Goal     =  12345L;
  Defaults.MinPace  = 25;
  Defaults.IdleSecTime = 10;
  Defaults.PaceSecBase = 60;

  // Save Data
  FlashWrite(&NonVolatileUserData,&Defaults, sizeof(TCounterUserData));

}
  
//  Check that the UserNvData is Legal, otherwise 
//  Load Default Values UserNvData
static void CounterNvDataLegal(void)
{
  BOOLEAN fReset=FALSE;
  if(NonVolatileUserData.Goal < 0 || NonVolatileUserData.Goal > 99999L)                 // 0<= Goal <= 99999
    fReset = TRUE;
  if(NonVolatileUserData.MinPace <= 0 || NonVolatileUserData.MinPace > 1000)             // 0<= pace <= 1000 
    fReset = TRUE;
  if(NonVolatileUserData.IdleSecTime <= 0 || NonVolatileUserData.IdleSecTime > (15*60)) // 15min
    fReset = TRUE;
  if(NonVolatileUserData.PaceSecBase <= 0 || NonVolatileUserData.PaceSecBase > 60)      // 1min (no
    fReset = TRUE;
  if(fReset) 
    CounterUserDataDefaults();
}


// Initializates variables
void CounterInit(void)
{
  // Check Validit
  CounterNvDataLegal();
  
  Counter.Value   =0;         
  Counter.Rate    =0;
  Counter.IdleMs  =0;

  Counter.WorkingUserData = NonVolatileUserData;
  Counter.DirtyFlag = FALSE;
  Counter.AutoSaveSecs = 10;     // Save 10 seconds after changes to Working data are complete

}




// This Task gets the hardware counter value TimerB delta between samples
//  The routine es exceuted periodically every COUNTTASK_MSTIME mS (100)
void CounterTask(void)
{
  static WORD TimerPrevious=0;
  static BYTE StopCondition=0;
  WORD count = TBR;

  // Check if No Counts
  if( TimerPrevious == count) 
  {
   if(++StopCondition > 10)  // 1 Second Fixed for now
   {
      PORT_RELAY_STOP &= ~MASK_RELAY_STOP;  // Lo = Turn ON light
      --StopCondition;
   }
  }
  else
  {
    StopCondition=0;
    PORT_RELAY_STOP |= MASK_RELAY_STOP;   // Hi = Turns OFF Light
  }

  // This is the condition for Normal Operation = Counting
  if (count >= TimerPrevious)
    Counter.Value += count - TimerPrevious;
  else
    Counter.Value += count + (0xFFFF - TimerPrevious) + 1;
  TimerPrevious = count;

  //  Compare Actual counter with preset
  if(Counter.Value >= Counter.WorkingUserData.Goal)
    PORT_RELAY_GOAL &= ~MASK_RELAY_GOAL;  // Turn ON Goal light
  else
    PORT_RELAY_GOAL |=  MASK_RELAY_GOAL;  // Turn OFF Goal light
 
}


