//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  MODULE: driverOsc.C
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
/*************************************************************************
*   $MODULE
*   osc.c - this will set up and maintain the oscillator in the processor.
*
*************************************************************************/

//=======================
// INCLUDES
//=======================  
#include "msp_port.h"
#include  <msp430x241x.h>
#include "define.h"
#include "driverOsc.h"
#include "driverTimer.h"
#include "main.h"
//========================
//  LOCAL DEFINES
//========================



//================================
//  LOCAL PROTOTYPES. 
//================================

typedef struct 
{
  long MCLK;
  unsigned char *BCSCTL1;
  unsigned char *DCOCTL;
} TFreqSetting;


static void Set_DCO(unsigned int Delta);
static void recoverOscilatorCalibration(void);

//========================
//  GLOBAL DATA
//========================
long MClkHz;                    /* MCLK frequency in Hz */
const long AClkHz = XCRYSTAL;   /* ACLK frequency in Hz */

//========================
//  LOCAL DATA
//========================
int DcoCounts;

//==============================================================================
// FUNCTIONS
//==============================================================================  
 
//==============================================================================
// Routine Name: OscillatorInit
// Date Created: September 1,2011
// Author:        MH
// Description: This will set the system clocks to their proper values.
//              Frequency is adjusted by constants in info flash
//                The clocks are set to the following:
//                  ACLK -  external (32768 KHz) crystal
//                  MCLK -  set to DCOCLK  (8MHz)
//                  SMCLK - set to DCOCLK (8MHz)
// Input: None
// Output: None
//  TODO: Add 4MHz in calibrations
//------------------------------------------------------------------------------
void OscillatorInit(void)
{
  BYTE test=0;

  volatile WORD i;
  
  
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  //  This Delay shouldn't be necessary when External Reset Circuit 
  //  The purpose is to generate a delay for DCO and XTAL stabilization
  //  it uses internal DCO at default frequency

  for(i=0; i< 0xFFFE; ++i)  __no_operation();

  #ifndef DEBUG_SIMULATE_HW
  // Check if the flash data section A with calibration data has been erased, if so re-calibrate oscillator constants
	if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF || test)  // make test <>0 to test routine
        recoverOscilatorCalibration();  // If limited license code, just hang here and code a separate "calibration" project
	#endif
  
  // Setup the DCO to 8MHz and inform Main clock value
  MClkHz =    MCLK_HZ;
  BCSCTL1 =   CALBC1;  // Calibrate DCO to 1MHz using calibration constants
  DCOCTL =    CALDCO;
    
  // By default should be Zero but just to clarify XTAL Divider 
  BCSCTL1 &= ~DIVA_3;       //  ACLK divider is 1
  BCSCTL1 |= XT2OFF;        //   X2 is OFF

  BCSCTL2 = 0x00;             // As default SELMx=00, DIVMx=00 (MCLK = DCO/1), SELS=0, DIVS=0 (SMCLK= DCO/1), DCOR=0 (internal R)
  BCSCTL3 |= XCAP1 | XCAP0;   // Add a 12.5pF  load cap at LFXT1
                              // Others as default: LFXT1Sx=0 (LFXT1 = 32768Hz xtal) 
	
  
  //TODO: Need to initialize Timer B Module for counting
  
}

//  Description: This code re-programs the F2xx DCO calibration constants.
//  A software FLL mechanism is used to set the DCO based on an external
//  32kHz reference clock. After each calibration, the values from the
//  clock system are read out and stored in a temporary variable. The final
//  frequency the DCO is set to is 1MHz, and this frequency is also used
//  during Flash programming of the constants. The program end is indicated
//  by the blinking LED.
//
// Constants to Recover oscillator constants
#define DELTA_1MHZ    244                   // 244 x 4096Hz = 999.4Hz
#define DELTA_8MHZ    1953                  // 1953 x 4096Hz = 7.99MHz
#define DELTA_12MHZ   2930                  // 2930 x 4096Hz = 12.00MHz
#define DELTA_16MHZ   3906                  // 3906 x 4096Hz = 15.99MHz

static void recoverOscilatorCalibration(void)
{
  unsigned char CAL_DATA[8];                  // Temp. storage for constants
  volatile unsigned int i;
  int j;
  char *Flash_ptrA;                           // Segment A pointer

  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  for (i = 0; i < 0xfffe; i++);             // Delay for XTAL stabilization
  P1OUT = 0x00;                             // Clear P1 output latches
  P1DIR = 0x01;                             // P1.0 output
  //P2SEL |= 0x02;                            // P2.1 SMCLK output
  //P2DIR |= 0x02;                            // P2.1 output

  j = 0;                                    // Reset pointer

  Set_DCO(DELTA_16MHZ);                     // Set DCO and obtain constants
  CAL_DATA[j++] = DCOCTL;
  CAL_DATA[j++] = BCSCTL1;

  Set_DCO(DELTA_12MHZ);                     // Set DCO and obtain constants
  CAL_DATA[j++] = DCOCTL;
  CAL_DATA[j++] = BCSCTL1;

  Set_DCO(DELTA_8MHZ);                      // Set DCO and obtain constants
  CAL_DATA[j++] = DCOCTL;
  CAL_DATA[j++] = BCSCTL1;

  Set_DCO(DELTA_1MHZ);                      // Set DCO and obtain constants
  CAL_DATA[j++] = DCOCTL;
  CAL_DATA[j++] = BCSCTL1;

  Flash_ptrA = (char *)0x10C0;              // Point to beginning of seg A
  FCTL2 = FWKEY + FSSEL0 + FN1;             // MCLK/3 for Flash Timing Generator
  FCTL1 = FWKEY + ERASE;                    // Set Erase bit
  FCTL3 = FWKEY + LOCKA;                    // Clear LOCK & LOCKA bits
  *Flash_ptrA = 0x00;                       // Dummy write to erase Flash seg A
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation
  Flash_ptrA = (char *)0x10F8;              // Point to beginning of cal consts
  for (j = 0; j < 8; j++)
    *Flash_ptrA++ = CAL_DATA[j];            // re-flash DCO calibration data
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCKA + LOCK;             // Set LOCK & LOCKA bit

  while (1)
  {
    P1OUT |= 0x01;                          // Turn ON LED
    for (i = 0; i < 5000; i++);           // SW Delay
    P1OUT &= ~0x01;                          // Turn OFF LED
    for( j=0; j <10; ++j)
    for (i = 0; i < 50000L; i++);           // SW Delay
    
    
  }
}

//   ACLK = LFXT1/8 = 32768/8, MCLK = SMCLK = target DCO
//  External watch crystal installed on XIN XOUT is required for ACLK 
static void Set_DCO(unsigned int Delta)            // Set DCO to selected frequency
{
  unsigned int Compare, Oldcapture = 0;

  BCSCTL1 |= DIVA_3;                        // ACLK = LFXT1CLK/8 = 32768/8 = 4096 Hz
  TACCTL2 = CM_1 + CCIS_1 + CAP;            // CAP, ACLK
  TACTL = TASSEL_2 + MC_2 + TACLR;          // SMCLK, cont-mode, clear

  while (1)
  {
    while (!(CCIFG & TACCTL2));             // Wait until capture occured
    TACCTL2 &= ~CCIFG;                      // Capture occured, clear flag
    Compare = TACCR2;                       // Get current captured SMCLK
    Compare = Compare - Oldcapture;         // SMCLK difference
    Oldcapture = TACCR2;                    // Save current captured SMCLK

    if (Delta == Compare)
      break;                                // If equal, leave "while(1)"
    else if (Delta < Compare)
    {
      DCOCTL--;                             // DCO is too fast, slow it down
      if (DCOCTL == 0xFF)                   // Did DCO roll under?
        if (BCSCTL1 & 0x0f)
          BCSCTL1--;                        // Select lower RSEL
    }
    else
    {
      DCOCTL++;                             // DCO is too slow, speed it up
      if (DCOCTL == 0x00)                   // Did DCO roll over?
        if ((BCSCTL1 & 0x0f) != 0x0f)
          BCSCTL1++;                        // Sel higher RSEL
    }
  }
  TACCTL2 = 0;                              // Stop TACCR2
  TACTL = 0;                                // Stop Timer_A
  BCSCTL1 &= ~DIVA_3;                       // ACLK = LFXT1CLK
}

//==============================================================================
// Routine Name: OscillatorTimeUpdate
// Date Created: January 6, 2003
// Author: MH
// Description: This task will maintain the DCO clock frequency by checking
//                the number of SMCLK counts between AClk (32,768 Hz)
//                period given by Crystal.
// Input: None
// Output: None
//------------------------------------------------------------------------------
void OscillatorTimeUpdate(void)
{
  int OscFreqError;
  //*MH F241x has 4 bits, RSEL3 added in next line
  #define RSEL  (RSEL3 + RSEL2 + RSEL1 + RSEL0)
  #define DCO_MAX     (0xe0)
  #define DCO_MIN     (0x05)

  //--------------------------------
  // if DCO value is too high, go to 
  // the next resistor value 
  if (DCOCTL > DCO_MAX) 
  {
    if ((BCSCTL1 & RSEL) != RSEL) 
    {
      BCSCTL1 = BCSCTL1 + RSEL0;
      DCOCTL = (DCO_MAX - DCO_MIN) / 2;   /* start in the middle of the range */
    }
  }
  else
  {
    //----------------------------
    // if DCO value is too low, go to the previous resistor value 
    if (DCOCTL < DCO_MIN) 
    {
      if ((BCSCTL1 & RSEL) != 0) 
      {
        BCSCTL1 = BCSCTL1 - RSEL0;
        DCOCTL = (DCO_MAX - DCO_MIN) / 2;   /* start in the middle of the range */
      }
    }
    else
    {
      //---------------------------
      // if there is a frequency error then adjust the DCO value by 1 step 
      //TODO: Check if this is the correct target
      OscFreqError = (MCLK_HZ >> MCLK_DIVIDER)/XCRYSTAL - DcoCounts;  //- ((MClkHz + AClkHz / 2) / AClkHz); 
      if (OscFreqError < 0) 
      {
        DCOCTL--;     // Decrease DCO frequency 
      } 
      else if (OscFreqError > 0) 
      {
        DCOCTL++;   // Increase DCO frequency
      }
      CRYSTAL_CCTL |= CCIE;          /* interrupts on */
    }
  }
}

