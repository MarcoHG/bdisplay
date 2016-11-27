//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  MODULE: MAIN.C
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

#include  <msp430x26x.h>
#include "msp_port.h"
#include  "define.h"
#include "driverFlash.h"
#include "driverOsc.h"
#include "driverTimer.h"
#include "driverUart.h"
#include "driverSpi.h"
#include "Error.h"
#include "main.h"
#include "apiDisplay.h"
#include "apiCounter.h"
#include "apiButtons.h"

unsigned int schedWord; 

// Brief history 
//  v0.0  Base with basic drivers: Oscillator, Flash, Uart
//  v0.1  Timers System Designed, 
//  v0.2  Test UART - Note: DCO adjust can't be used while communicate at high speed.
//  v0.3  Special mode SPI to communicate with Led Driver. Send Data and Configuration 
//  v0.4  Finish Basic Firmware Design to ship tested hardware - MSPFET was tested (wine)
//  v0.5  Test NV values and POS (use LED1), RC functionality
//        Ported from Crosswork to CCS 5
//==============================================================================
// Routine Name: MainInitIOPorts(void)
// Description: This routine will initialize all port pins. 


//------------------------------------------------------------------------------
void MainInitIOPorts(void)
{

// P1.0/TACLK     12  O     H       Status LED 
// P1.1/TA0       13  O     H       BL_TXD
// P1.2/TA1       14  I     H       RCRXD (TA1)
// P1.3/TA2       15  O     L       RS485-DE/RE
// P1.4/SMCLK     16  I     -       Test point 
// P1.5/TA0       17  I     -       Test point
// P1.6/TA1       18  I     -       Test point
// P1.7/TA2       19  I     H       MRES(eset) counter

  P1REN = 0x84;                 /* 1=Enable Pull Up/Down resistor             */
  P1OUT = 0x87;                 /* default output port values */
  P1DIR = 0x0B;                 /* direction register (1 is output, 0 is input) */
  P1IES = 0x00;                 /* interrupt edge select (1 is hi-low, 0 is low-hi) */
  P1IE =  0x00;                  /* interrupt enable (1 is enabled, 0 is disabled) */
  P1IFG = 0x00;                 /* clear interrupt flags */
  P1SEL = 0x00;                 /* function select (1 is function, 0 is port) */

  RCRXD_PORT_SEL  |= RCRXD_MASK_RCRXD;  // P1SEL more abstracted
  

  RESET_BUTTON_IE |= MASK_RESET_BUTTON;   // Reset Button Can Interrupt
  RESET_BUTTON_IES |= MASK_RESET_BUTTON;  // 1 = on the Hi to Low



// P2.0/ACLK      20  I     H       INC(crease)
// P2.1/TAINCLK   21  I     H       DEC(crease)
// P2.2/CAOUT/TAO 22  I     H       BL_RXD
// P2.3/CA0/TA1   23  I     -       NC
// P2.4/CA1/TA2   24  I     -       NC
// P2.5/ROSC      25  I     -       Test point
// P2.6           26  I     -       Test point
// P2.7/TA0       27  I     -       Test point
	P2OUT = 0x07;
  P2REN = 0x03;                 /* 1= Enable Pull Up/Down Resistor                 */
  P2DIR = 0x00;                 /* direction register (1 is output, 0 is input) */
	P2IES = 0x00;                 /* interrupt edge select (1 is hi-low, 0 is low-hi) */
  P2IE =  0x00;                 /* interrupt enable (1 is enabled, 0 is disabled) */
  
  P2IFG = 0x00;                 /* clear interrupt flags */
  P2SEL = 0x00;                 /* function select (1 is function, 0 is port) */
  
  INCDEC_BUTTONS_REN |= MASK_INC_BUTTON | MASK_DEC_BUTTON;  // Resistor Enable
  PORT_INCDEC_BUTTONS_OUT |= MASK_INC_BUTTON | MASK_DEC_BUTTON; // Pull-Ups

  INCDEC_BUTTONS_IE |= MASK_INC_BUTTON | MASK_DEC_BUTTON; // Both Buttons make interrupt
  INCDEC_BUTTONS_IES |= MASK_INC_BUTTON | MASK_DEC_BUTTON;  // Interrupt in Hi-Lo
  
  
  

// P3.0/STEO      28  I     -       NC
// P3.1/SIMO      29  O     L       UCB0SIMO 
// P3.2/SOMI      30  I     H       UCB0SOMI 
// P3.3/UCLK      31  O     L       UCB0CLK
// P3.4/UTXD0     32  I     -       Test point
// P3.5/URXD0     33  I     -       Test point
// P3.6/UTXD1     34  O     L       Serial TXD
// P3.7/URXD1     35  I     H       Serial RXD
//

  P3OUT = 0x00;                 /* default output port values */
  P3DIR = 0x4a;                 /* direction register (1 is output, 0 is input) */
  P3SEL = 0xce;                 /* function select (1 is function, 0 is port) */
	
// P4.0/TB0       36  O     L       LE (Latch Enable)
// P4.1/TB1       37  O     H       OE# (Output Enable)
// P4.2/TB2       38  I     -       NC
// P4.3/TB3       39  I     -       NC
// P4.4/TB4       40  I     -       NC
// P4.5/TB5       41  O     H       L1 (Lamp1 Goal reached, LO= Turn ON Light)
// P4.6/TB6       42  O     H       L2 (Lamp2 Machine Stopped, LO= Turns ON Light)
// P4.7/TBCLK     43  I     H       External Counter Input  (TBCLK)

//
  P4OUT = 0xE2;   /* default output port values */
  P4DIR = 0x63;   /* direction register (1 is output, 0 is input) */
  P4SEL = 0x80;   /* function select (1 is function, 0 is port) */

  PORT_XCOUNTER_SEL |= MASK_XCOUNTER;

// P5.0/STE1      44  I     -       Test point
// P5.1/SIMO      45  I     -       Test point
// P5.2/SOMI      46  I     -       Test point
// P5.3/UCLK1     47  I     -       Test point
// P5.4/MCLK      48  I     -       Test point
// P5.5/SMCLK     49  I     -       NC
// P5.6/ACLK      50  I     -       NC
// P5.7/TBOUTH    51  I     -       NC
  P5OUT = 0x00;                 /* default output port values */
  P5DIR = 0x00;                 /* direction register (1 is output, 0 is input) */
  P5SEL = 0x00;                 /* function select (1 is function, 0 is port) */

// P6.x Not defined yet
  P6OUT = 0x00;   /* default output port values */
  P6DIR = 0x00;   /* direction register (1 is output, 0 is input) */
  P6SEL = 0x00;   /* function select (1 is function, 0 is port) */

}



//==============================================================================
// Routine Name: SysInit
// Date Created: October 11, 2001
// Author: SLTF
// Description: This will initialize the system.
// Input: None
// Output: None
//------------------------------------------------------------------------------
static void SysInit(void)
{
  //MainWatchdog();

/* set the clock to the right rate */
  OscillatorInit();

/* initialize the flash memory */
  FlashInit();


/* intialize I/O ports */
  MainInitIOPorts();

/*  Initilialize the Timer System */
  TimerInit();

/*  Initilialize the Timer System */
  
 
/* enable interrupts */
  __enable_interrupt();
}
//#pragma codeseg("CODEH")
void MainTaskInit(void)
{ 
    //JumperInit(); Get the available options
    ErrorInit();    
    // KeypadInit();
    TimerInit();
    UartInit();
    CounterInit();
    DisplayInit();

  
}  



//  Globals are initialized to zero in ANSI C, i.e. linker will locate to RAM, we initialize to any value 
//  and then set load section to "No" in linker placement file
#pragma DATA_SECTION(MyFriend, ".infoD")
tresamigos MyFriend;    // = {1,2,3};

const char string[] = "Marco Henry"; 

void delay(unsigned int d)
{
  volatile unsigned int i,j;
  for(j=0; j<d; ++j)
  {
    for(i =0; i < 25000; ++i )
      __no_operation();
  }
}

volatile tresamigos MyFriendInRam;

void TaskNull(void)
{

}


//  This routine uses the time base tick to generate different 
//  periodic time related events. This runs at TIMEBASE_MSEC_ISR
//
void TimerClockManager(void)
{
  static WORD   AutoSaveSecTick=0,
                IncDecButtonsTick=0;

  if(++AutoSaveSecTick >= (1000/TIMEBASE_MSEC_ISR) )   // Every Second
  {
      AutoSaveSecTick =0;
      AutoSaveCounterUserData();
      //++Counter.Value;  // Check Precision
  }
  if(++IncDecButtonsTick >=(100/TIMEBASE_MSEC_ISR))  // Every 100 mS
  {
    IncDecButtonsTick=0;
    IncDecButtonAuto();
  }


  
  
}
// This Task Adjusts the DCO values internal resistors RSEL
void DcoAdjustTask(void)
{
  
  //TODO: Changes  OscillatorTimeUpdate();
  //  Note: Oscillator can not be adjusted when communicating at
  //  high baud rates. Either use lower baud or adjust in silent
  //  intervals
}


typedef void SWTASK(void);
// Task list should follow bit order from lsb to msb in main.h
SWTASK *const SwTaskList[] = 
{
  //UartProcess, //TaskNull,
  TimerClockManager,
  DcoAdjustTask,
  UartRxTask,
  UpdateDisplay,
  CounterTask,
  KeyPressedTask,
  KeyPolllingTask,
  ResetCounterButton,
  
  TaskNull
  
};

// Get the Task from the list 
void TaskScheduler(void)
{
  BYTE count; 
  count = 0; 
  while (count<DIM(SwTaskList))
  {
    if ((schedWord & (1<<count))!= 0)
    {
      __disable_interrupt();  //_DINT();
      schedWord &= (~(1<<count));
      __enable_interrupt();   //_EINT();
      SwTaskList[count]();
    }  
    count++;
  }  
  // ReadingStateMachineUpdate();
}  
int DISPLAY_UPDATE_MSTIME;
void main(void)
{
  BOOLEAN fSave = FALSE, displayon=TRUE, displaystatus;
  BYTE i=0, gain=120, gain_old=0;
  int Freq, stupids=0;
  
  MyFriendInRam = MyFriend;
  DISPLAY_UPDATE_MSTIME =50;
  SysInit();
  MainTaskInit();

  //P5SEL = 0x20;
  displaystatus = !displayon;

  Counter.Value = 88888L; // Test Hw
  DisplayMode = COUNTER_DISPLAY;
  while(1)
  {
    if( displayon != displaystatus)
    {
      if(displayon)
        DisplayOn();
      else
        DisplayOff();
      displaystatus = displayon;

    }
    if(fSave)
    {
      fSave= FALSE;
      ++stupids;
      FlashWrite((void *)&MyFriend, (void *)&MyFriendInRam, sizeof(tresamigos));
      FlashWrite(&SerialPort, &SerialPort, sizeof(TSerialPort));
    }
    
    // Send some data to Serial port to test
    if(!UartHal.RxBufferEmpty)
    {
      // Send char
      UartSend(UartHal.RxChar);       // Echo the most recent received char
      UartHal.RxBufferEmpty = TRUE;   // This is not the proper way to do things
    }
    // Manage the Task 
    TaskScheduler();
  }

    
}

