//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  MODULE: driverTimer.c
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
#include  <msp430x241x.h>
#include  <msp430x26x.h>
#include  "define.h"
#include "msp_port.h"
#include "driverOsc.h"
#include "driverTimer.h" 
#include "apiDisplay.h"
#include "apiCounter.h"
#include "Error.h"
#include "main.h"



//========================
//  LOUART_CTL1 DEFINES
//========================

//================================
//  LOUART_CTL1 PROTOTYPES. 
//================================

//========================
//  GLOBAL DATA
//========================
BOOLEAN fTimerBlink;
 
//========================
//  LOUART_CTL1 DATA
//========================
static unsigned long TimerTicks;
static unsigned long TimerStateSec;
static unsigned char timer500msec; 
static unsigned char timer1000msec; 
unsigned char timerCount, ProfibusTimerCount;
unsigned char timerWdog; 
//==============================================================================
// FUNCTIONS
//==============================================================================  




//==============================================================================
// Routine Name: TimerInit
// Date Created:  September 8, 2011
// Author:        MH
// Description: Set timers as follows:
//              Timer A:  Clock input is MCLK/8 = 1MHz, Counting UP to CCR0 
//                CCR0  Compare to 19,999 and generate an interrupt (Time Base of 20mS)
//                CCR1  Capture both edges of RCRXD (CCI1A, TA1) to decode IR. IE
//                CCR2  Capture rising edge of ACLK (CCI2B) to adjust DCO frequency, IE
//
//TODO:         Timer B:  Count events in external pin TBCLK,
//                        16 bit, counts pp continuously to FFFF, source is TBCLK Inverted
//
// Input: None
// Output: None
//------------------------------------------------------------------------------
//#pragma codeseg("CODEH")
void TimerInit(void)
{
  BYTE InputDivider;

  // Define the Time Base Clock Input Divider ID_x 
  #ifndef MCLK_DIVIDER
    #error  "Need to define the time base MCLK_DIVIDER"
  #elif MCLK_DIVIDER == 0 
    #define ID_x  ID_ ## 0
  #elif MCLK_DIVIDER == 1 
    #define ID_x  ID_ ## 1
  #elif MCLK_DIVIDER == 2 
    #define ID_x  ID_ ## 2
  #elif MCLK_DIVIDER == 3 
    #define ID_x  ID_ ## 3
  #else
    #error "MCLK_DIVIDER can only be 0 to 3"
  #endif

  
  TACTL = TASSEL_2 | ID_x | MC_0 | TACLR;
  //TACTL = TASSEL_2 | INPUT_DIVIDER(MCLK_DIVIDER) | MC_0 | TACLR;
  #undef ID_x
  //------------------------------------
  // configure CCR0 to output a periodic interrupt every TIMEBASE_MSEC_ISR
  TACCR0 = (((long)MCLK_HZ >> MCLK_DIVIDER) * TIMEBASE_MSEC_ISR)/1000 -1;
  TACCTL0 = CCIE;           // Enable Interrupt
  
  //------------------------------------
  // configure CCR1 to Capture Both Edges of RCRXD and make a Interrupt
  TACCTL1 =   CM_3  | CCIS_0  | SCS | CAP | CCIE;

  // configure CCR2 to Capture Rising Edge of ACLK. Interrupt is controlled somwhere else
  TACCTL2 =   CM_1  | CCIS_1  | SCS | CAP;

  TACTL |= MC_1;          // Run Timer A
  

  // Configure Timer B to count from external Input ctr
  //  16 bits, Continuous Mode Up to TBCL0, No-interrupt

  TBCTL  =  MC_2 | TBSSEL_3;  // Counts continuously up to FFFF, Inverted TBCLK
  TBCTL  |= TBCLR;            // Clear counter

  fTimerBlink = FALSE;
  timer500msec = 1L;
  timer1000msec = 3L; 
  TimerStateSec = 0L;
  timerCount = 0; 
  
  DcoCounts =0;




}
//#pragma codeseg(default)



//==============================================================================
// Routine Name: TimerISR
// Date Created: September 8, 2011
// Author: MH
// Description: This is the timer interrupt service routine every TIMEBASE_MSEC_ISR
//              Some schedule flags are set here to execute in TaskScheduler()
//
// Input: None
// Output: None
//------------------------------------------------------------------------------
#pragma vector=TIMERA0_VECTOR
__interrupt void BaseTimerISR(void)
{
  static WORD DcoAdjustTimer = 0, DisplayUpdate = 0, CountItemsTimer=0, KeyPollTimer=0;
  
  schedWord |= SCHEDWORD_TIMEBASE_TICK;   // Mark the Time Base beat
  
  if(++DcoAdjustTimer > DCO_ADJUST_MSTIME/TIMEBASE_MSEC_ISR)  // Time to Adjust the DCO according ACLK
  { 
    DcoAdjustTimer =0;
    schedWord |= SCHEDWORD_DCOADJUST;   
      
  }
  if(++DisplayUpdate > DISPLAY_UPDATE_MSTIME/TIMEBASE_MSEC_ISR)  // Time to Adjust the DCO according ACLK
  { 
    DisplayUpdate =0;
    schedWord |= SCHEDWORD_UPDATEDISPLAY;   
      
  }
  if(++CountItemsTimer > COUNTTASK_MSTIME/TIMEBASE_MSEC_ISR)  // Time to Update the Software Counter by hw delta
  { 
    CountItemsTimer =0;
    schedWord |= SCHEDWORD_COUNTTASK;   
      
  }
  if(++KeyPollTimer > 60/TIMEBASE_MSEC_ISR)    // Poll Keys every 60mS
  { 
    KeyPollTimer=0;
    schedWord |= SCHEDWORD_KEYPOLL;
  }

}        

//==============================================================================
// Routine Name: RcCrystalCaptureISR
// Date Created: September 11, 2011
// Author: MH
// Description: Capture the Edges of the IR Remote Control and the Crystal
//              The IR is decoded in the Capture 1
//              The Crystal is used to adjust the DCO frequency
//
// Input: None
// Output: None
//------------------------------------------------------------------------------
#pragma vector=TIMERA1_VECTOR
__interrupt void RcCrystalCaptureISR(void)
{
  
  unsigned int readcapture, readTAIV;
  readTAIV = TAIV;  // This also resets the interrupt Flag
  // Determine if the IR Remote Control generated an interrupt
  if(readTAIV & 0x02) //TACCTL1 & CCIFG)
  {
    readcapture = RCRXD_CAPTURE_REG;   // Get the data
    // P1OUT ^= 0x01; //Just to Test we are capturing RCRXD
    ++RcCounter;  // Test We are RC
    DisplayMode = RC_TEST_DISPLAY;

    //  TODO: Extract RC key-code
  }
  // Determine if the ACLK was captured 
  if(readTAIV & 0x04) //TACCTL2 & CCIFG)
  {
    static int LastCapture;
    static BYTE statem;
    if(TACCTL2 & COV)
      statem =0;  // Start 
    switch(++statem)
    {
      case 1: 
        TACCTL2 &= ~COV;    // Clear Capture Overflow bit
      case 2:   
        LastCapture = TACCR2;//CRYSTAL_CCR;
        break;
      case 3:
        DcoCounts = CRYSTAL_CCR - LastCapture;
        if(DcoCounts < 0 )
          DcoCounts += TACCR0;
          CRYSTAL_CCTL &= ~CCIE; // Clear Interrupt until next time
          //== DCO adjustment cycle Done ==
          break;
    }
  }
}        



