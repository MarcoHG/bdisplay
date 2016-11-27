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
*   apiDisplay.c
*
*************************************************************************/

/*************************************************************************
*   $INCLUDES
*************************************************************************/
#include "define.h"
#include "msp_port.h"
#include "apiDisplay.h"
#include "main.h"
#include "driverSpi.h"
#include "driverFlash.h"
#include "apiCounter.h"
#include <stdio.h>
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
TDisplayMode DisplayMode;
#pragma loaction(DisplaySettings, "INFO_D")
TDisplaySettings DisplaySettings ={0xFF};  // Dummy (non-zero) Initialization



//========================
//  LOCAL DATA
//========================
// 	Order for Normal orientation: Decimal point at Right Hand
//	Common cathode, 10 Pin using a 16 bit led driver
//  Connection is as follows:
//                
const BYTE bcdto7segm[] = 
{           //                7                             0
            //  DP  G   F   E     D   C   B   A   = 0
  0x3F,     //  0   0   1   1     1   1   1   1
  0x06,     //  0   0   0   0     0   1   1   0
  0x5B,     //  0   1   0   1     1   0   1   1
  0x4F,     //  0   1   0   0     1   1   1   1
  0x66,     //  0   1   1   0     0   1   1   0

  0x6D,     //  0   1   1   0     1   1   0   1   = 5
  0x7D,     //  0   1   1   1     1   1   0   1
  0x07,     //  0   0   0   0     0   1   1   1
  0x7F,     //  0   1   1   1     1   1   1   1
  0x6F,     //  0   1   1   0     1   1   1   1
  

};
void DisplayDefaults(void)
{
  TDisplaySettings Defaults;
  Defaults.NumDisplayDigits =5;
  Defaults.CurrentGain =100;
  Defaults.HiCurrentGain = FALSE;
  FlashWrite(&DisplaySettings,&Defaults, sizeof(TDisplaySettings));
    
}

static void DisplayLegal(void)
{
  BOOLEAN fReset  = FALSE;
  if(DisplaySettings.NumDisplayDigits ==0 || DisplaySettings.NumDisplayDigits > 5)
    fReset= TRUE;
  if(DisplaySettings.CurrentGain ==0 || DisplaySettings.CurrentGain >= 127)
    fReset= TRUE;
  if(DisplaySettings.HiCurrentGain !=0 && DisplaySettings.HiCurrentGain != 1)
    fReset= TRUE;
  if(fReset)
    DisplayDefaults();
}
void DisplayInit(void)
{
  DisplayLegal();

  // Adjust the Intensity, etc
  DisplayMode = COUNTER_DISPLAY;
  SetCurrentGainTlc5917(&DisplaySettings);
  DisplayOn();
}

//==============================================================================
// Routine Name: UpdateDisplay
// Date Created: September 14, 2011
// Author: MH
// Description: Task to update the 7 segment display
//
// Input: None
// Output: None
//------------------------------------------------------------------------------

void UpdateDisplay(void)
{
  char spidata[6];
  long int value;
  static long int RcCounterOld; // TODO: Remove later
  static BYTE RcTimeout =0;
  const char normal_str[] = {'%', NUM_DISPLAY_DIGITS+'0','l','d',0};
  BYTE i;

  // This is the Normal Mode
  if(DisplayMode == COUNTER_DISPLAY)
  {
    value = Counter.Value;
    if(value < 0) value =0;
    if(value > 99999) value =99999; 
    sprintf(spidata,normal_str,value); 
    for(i=0; i< NUM_DISPLAY_DIGITS; ++i)
    spidata[i] = '0' <= spidata[i] && spidata[i] <='9' ? bcdto7segm[spidata[i]-'0'] : 0;
    SpiComm(NUM_DISPLAY_DIGITS, (unsigned char *)&spidata[0], SPI_LEDDRV);
  }
  else
  // This is the Goal Modify Mode
  if(DisplayMode == GOAL_MODIFY_DISPLAY)
  {
    value = Counter.WorkingUserData.Goal;
    if(value < 0) value =0;
    if(value > 99999) value =99999; 
    sprintf(spidata,normal_str,value); 
    for(i=0; i< NUM_DISPLAY_DIGITS; ++i)
    spidata[i] = '0' <= spidata[i] && spidata[i] <='9' ? bcdto7segm[spidata[i]-'0'] : 0;
    SpiComm(NUM_DISPLAY_DIGITS, (unsigned char *)&spidata[0], SPI_LEDDRV);
  }
  else
  if(DisplayMode == RC_TEST_DISPLAY)
  {
    sprintf(spidata,"00000"); 
    for(i=0; i< NUM_DISPLAY_DIGITS; ++i)
    spidata[i] = '0' <= spidata[i] && spidata[i] <='9' ? bcdto7segm[spidata[i]-'0'] : 0;
    SpiComm(NUM_DISPLAY_DIGITS, (unsigned char *)&spidata[0], SPI_LEDDRV);
    if(RcCounter == RcCounterOld && ++RcTimeout > 50)
     {
      DisplayMode = COUNTER_DISPLAY;
      RcTimeout=0;
      
     }
     RcCounterOld = RcCounter; 

  }



#undef _FORMAT_STR
}

void  DisplayOn(void)
{
  BYTE *p = (BYTE *)PORT_LEDDRV_OE;
  *p &=  ~MASK_LEDDRV_OE;
}

void DisplayOff(void)
{
  BYTE *p = (BYTE *)PORT_LEDDRV_OE;

  *p  |=  MASK_LEDDRV_OE;
}
