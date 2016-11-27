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
*   apiButtons.c
*
*************************************************************************/

/*************************************************************************
*   $INCLUDES
*************************************************************************/
#include "define.h"
#include "msp_port.h"
#include "main.h"
#include "apiCounter.h"
#include "apiButtons.h"
#include "apiDisplay.h"
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
BYTE KeyPressed;
WORD NoKeyTimer;
//========================
//  LOCAL DATA
//========================


//  

//==============================================================================
// Routine Name: IncDecButtonsISR
// Date Created: September 24, 2011
// Author: MH
// Description: Interrupt routine when any of the buttons are pressed
//              An event is sent to TaskScheduler()
//
// Input: None
// Output: None
//------------------------------------------------------------------------------
#pragma vector=PORT2_VECTOR
__interrupt void IncDecButtonsISR(void)
{
  schedWord |= SCHEDWORD_INCDEC_BUTTONS;   
  if(INCDEC_BUTTONS_IFG & MASK_INC_BUTTON)
  {
    KeyPressed |= MASK_INC_KEY_ISR;
    INCDEC_BUTTONS_IFG &= ~MASK_INC_BUTTON; // Clear Interrupt Flag
  }
  if(INCDEC_BUTTONS_IFG & MASK_DEC_BUTTON)
  {
    KeyPressed |= MASK_DEC_KEY_ISR;
    INCDEC_BUTTONS_IFG &= ~MASK_DEC_BUTTON; // Clear Interrupt Flag
  }
}

//==============================================================================
// Routine Name: ResetCounterButtonISR
// Date Created: September 24, 2011
// Author: MH
// Description: Interrupt routine when any the Reset Counter buttons is pressed
//              
//
// Input: None
// Output: None
//------------------------------------------------------------------------------
#pragma vector=PORT1_VECTOR
__interrupt void ResetCounterButtonISR(void)
{
  schedWord |= SCHEDWORD_RESET_BUTTON;  
  RESET_BUTTON_IFG &=  ~MASK_RESET_BUTTON;  // Clear the interrupt Flag
  

}
// This changes one by one
void KeyPressedTask(void)
{
  static BYTE KeyRepeatTick =0;
  
  if(DisplayMode != GOAL_MODIFY_DISPLAY)  //  Change to Mode Preset
  {
    KeyRepeatTick=0;
    NoKeyTimer = 0;                     // AllKeys released
    DisplayMode = GOAL_MODIFY_DISPLAY;  // Change To Goal Modify mode
    // No Increase for now
  }
  else
  {
    if(KeyPressed & MASK_INC_KEY_ISR && Counter.WorkingUserData.Goal < MAX_GOAL_COUNT)
      ++Counter.WorkingUserData.Goal;
    if(KeyPressed & MASK_DEC_KEY_ISR && Counter.WorkingUserData.Goal > 0)
      --Counter.WorkingUserData.Goal;
    
    KeyPressed =0;  // We loss other key
  }
}
//  KeyPollingTask is executed every 60mS
void KeyPolllingTask(void)
{
  static BYTE AutoRepeatTimer=0, AutoInc=1, IncDelay=0;
  BYTE  lKeys = ~PORT_INCDEC_BUTTONS_IN & (MASK_INC_BUTTON | MASK_DEC_BUTTON); //  Read Buttons
 
  if(!lKeys)
   {
     AutoRepeatTimer =0;
     AutoInc =1;
     IncDelay =0;
     if(++NoKeyTimer > 100)  // 6 Secs
      if(DisplayMode != RC_TEST_DISPLAY) // TODO: Remove Later
        DisplayMode = COUNTER_DISPLAY;
   }
   else
   if(++IncDelay > 20)
   {
      IncDelay = 20;
      if( lKeys & MASK_INC_BUTTON)
        Counter.WorkingUserData.Goal += AutoInc;
      else
        Counter.WorkingUserData.Goal -= AutoInc;
      if(++AutoRepeatTimer >= 10)
      {
        AutoRepeatTimer =0;
        if(AutoInc < 0xff) ++AutoInc;
      }
   }
}

// This is the Periodic task for Increase or Decrease
void IncDecButtonAuto(void)
{
}



// This routine Resets the Count value from outside an Interrupt
void ResetCounterButton(void)
{
  Counter.Value =0;
}
        
