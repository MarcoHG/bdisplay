//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  MODULE: driverUart.C
//------------------------------------------------------------------------------
/************************************************************************/
/*                                                                      */
/*              DEVELOPED BY:   HF SCIENTIFIC, INC                      */
/*                              3170 METRO PARKWAY                      */
/*                              FT. MYERS, FLORIDA USA  33916-7597      */
/*                              (941) 337-2116                          */
/*                              www.hfscientific.com                    */
/*                              info@hfscientific.com                   */
/*                                                                      */
/************************************************************************/

//=======================
// INCLUDES
//=======================  
#include <stdlib.h>
#include <stdio.h>
#include  <msp430x26x.h>
#include <string.h>
#include "define.h"
#include "msp_port.h"
#include "driverSpi.h"
#include "driverOsc.h"
#include "driverFlash.h"
#include "apiMessages.h"
#include "driverUart.h"
#include "main.h"

//========================
//  LOCAL DEFINES
//========================
#define OutQLen 128
#define InQLen  255  //v2.2.6 changed from 256 due to warning and useless comparison 
                     // to a byte value, need this size for Modbus net monitoring */
#define UART_N71        (0x00)              /* 8-bit, 1 stop, no parity */
#define UART_N72        (SPB)               /* 8-bit, 2 stop, no parity */
#define UART_O71        (PENA)              /* 8-bit, 1 stop, odd parity */
#define UART_E71        (PENA | PEV)        /* 8-bit, 1 stop, even parity */
#define UART_N81        (CHAR)              /* 8-bit, 1 stop, no parity */
#define UART_N82        (CHAR | SPB)        /* 8-bit, 2 stop, no parity */
#define UART_O81        (CHAR | PENA)       /* 8-bit, 1 stop, odd parity */
#define UART_E81        (CHAR | PENA | PEV) /* 8-bit, 1 stop, even parity */


//================================
//  LOCAL PROTOTYPES. 
//================================
typedef struct {
  BYTE Data;                    /* UART data */
  BYTE Status;                  /* UART data status */
} TUartData;


static BYTE UartBaudRate(void);
static void UartLegal(void);


//========================
//  GLOBAL DATA
//========================
TUartHal UartHal;
//TSerialStatus SerialStatus;     /* a running summary of the UART */

char *const strUartBaudRates[UART_NUM_BAUD_RATES] = 
{
  "300",
  "1200",
  "2400",
  "4800",
  "9600",
  "19200",  // <== UART_DEFAULT_BAUD_RATE = (UART_NUM_BAUD_RATES - 5)
  "38400",
  "57600",
  "115200",
  "230400"
};

char *const strUartBits[NumSerialBits] =
{   
  "7",
  "8",
};

char *const strUartStopBits[NumSerialStopBits] = 
{   
  "1",
  "2",
};

char *const strUartParity[NumSerialParity] =     
{ 
  (char *)MsgComParityNone,
  (char *)MsgComParityEven,
  (char *)MsgComParityOdd,
};


#pragma DATA_SECTION(SerialPort, ".infoD")
TSerialPort SerialPort;// = {0xFF}; // Dummy initializer, INFO_D is not loaded


//========================
//  LOCAL DATA
//========================
static BOOLEAN XMitFlag;
static BYTE TxBuffer[OutQLen];
static BYTE OutQueT;
static BYTE OutQueH;
//static TUartData RxBuffer[InQLen];   /* includes not only the data but the status */
static BYTE RxBuffer[InQLen];
static BYTE InQueT;
static BYTE InQueH;
static int UartTxDelay;         /* the transmit driver enable time delay */
static int UartModbusSilentDelay;       /* Modbus silent delay */

//==============================================================================
// FUNCTIONS
//==============================================================================  
 
//==============================================================================
// Routine Name: UartSystemInit
// Date Created: February 11, 2008 
// Author: EMH
// Description: 
//------------------------------------------------------------------------------
void UartInit(void)
{
  UartLegal();
  UartStart();
}  


//==============================================================================
// Routine Name: UartRxTask (UartProcess)
// Date Created: August 29, 2002
// Author: SLTF
// Description: This will handle the UART in the processor for serial
//                communications to the external world. In the OSI model,
//                this is similar to the Data Link layer.
//                This task helps the communication protocols be allowing a
//                quicker response to received characters than could normally
//                be handled with MiCOS. The task list that runs are all the
//                tasks that access a received character (EV_UART_DATA). From
//                the other tasks point of view, nothing is different.
// Input: None
// Output: None
//------------------------------------------------------------------------------
void UartRxTask(void)
{
  BYTE QueueHead;
  /* get the current queue head pointer */
  __disable_interrupt();
  QueueHead = InQueH;
  __enable_interrupt();

  /* See if there is any data to retrieve and if so, get it (one at a time) */
  if (QueueHead != InQueT)      // If a chunk of data is necessary, we can have a second buffer
  {                             // and use a while() instead
    if (++InQueT >= InQLen) 
    {
      InQueT = 0;
    }
    UartHal.RxChar = RxBuffer[InQueT];
    UartHal.RxBufferEmpty = FALSE;
    ++UartHal.RxCharCount;    /* one more character received */
    
  } 
}
  


//==============================================================================
// Routine Name: UartStart
// Date Reviewed: September 13, 2011
// Author: MH
// Description: This task will reset and initialize the UART
// Input: None
// Output: None
//------------------------------------------------------------------------------
void UartStart(void)
{
  //--------------------------------------
  // initialize the memory buffers and pointers 
  XMitFlag = FALSE;
  InQueT = 0;
  InQueH = 0;
  OutQueT = 0;
  OutQueH = 0;
  //Changes UartCharStatus = NoChar;
  UartHal.RxBufferEmpty = TRUE;
  UartHal.RxCharCount =0;
  UartHal.TxCharCount =0;
  //--------------------------------------
  // disable the interrupt enable flags 
  UART_IE &= ~( UART_RXIE | UART_TXIE ); 
  //------------------------------
  // clear all the counters 
  //Changes memset(&SerialStatus, 0, sizeof(SerialStatus));
  //---------------------------------------
  // initialize the USART control register reset the control register. 
  UART_CTL1 = UCSWRST;
  //-----------------------------------
  // select the number of data bits 
  switch (SerialPort.DataBits) 
  {
    case Seven:
      UART_CTL0 |= UC7BIT;
      break;

    case Eight:
      UART_CTL0 |= 0x00;
      break;
  }
  //---------------------------------
  // select the number of stop bits 
  switch (SerialPort.StopBits) 
  {
    case One:
      UART_CTL0 |= 0x00;
      break;

    case Two:
      UART_CTL0 |= UCSPB;
      break;
  }
  //--------------------------
  // select the parity 
  switch (SerialPort.Parity) 
  {
    case ParityNone:
      UART_CTL0 |= 0x00; 
      break;

    case ParityEven:
      UART_CTL0 |= (UCPEN | UCPAR);
      break;

    case ParityOdd:
      UART_CTL0 |= UCPEN;
      break;
  }
  //---------------------------
  // set the UART baud rate and Clock Source 
  if (SerialPort.Baud != UartBaudRate())
  {
    //  TODO: Investigate and report new baud Rate in SerialPort.BaudBaud
  }
  
  // turn on the UART I/O pins 
  UART_PORT_SEL |= (MASK_SERIAL_TX | MASK_SERIAL_RX);
  
  //----------------------------
  // release the reset bit 
  UART_CTL1 &= ~UCSWRST;                         //*MH U1CTL &= ~SWRST;
  //--------------------------------------
  
  // set the RX interrupt enable flag only, UART_TXIE is enabled later, when char is sent
  UART_IE |= UART_RXIE;
}

//==============================================================================
// Routine Name: UartBaudRate
// Date Reviewed: October 15, 2002, September 2011
// Author:        MH
// Description:   This function will set the USCI baud rate in registers UCBR0, 
//                UCBR1 and UCBMOD.
//                Two clocks ACLK (Lo) and SMCLK (Hi) should be available for calculation 
//                The algorithm selects the proper clock and oversampling if necessary
//                Routine should be called during serial setup (state machine is stopped)
//              
//
// Input: Globals: ACLK and System Clock MClkHz. The SerialPort should be defined
//
// Output: Returns the (index to) Baud rate used 
//                
//------------------------------------------------------------------------------
static BYTE UartBaudRate(void)
{
  long BaudRate;
  BOOLEAN OverSampling;
  unsigned long int Nx16;  // Divider
  enum { None, UartUseAclk, UartUseSmclk};
  unsigned int n=0;
  BYTE ClkSrc = None, FinalBaud;
  
    
  // Select the proper Clock source and Oversample
  while(ClkSrc == None)
  {
    BaudRate = atol(strUartBaudRates[SerialPort.Baud]);
    if(AClkHz / BaudRate >= 48 )  // 1st Choice is Aclk and see if enough rate
    {
      OverSampling = FALSE;
      ClkSrc = UartUseAclk;
    }
    else
    if((n = MClkHz / BaudRate) >= 48  || n == 34)  // 2nd is the SMCLK with Ovsersampling
    {
      OverSampling = TRUE;
      ClkSrc = UartUseSmclk;
    }
    else
      --SerialPort.Baud;    // Original Baud can not be achieved, Lower result Baudrate
  }
  FinalBaud= SerialPort.Baud; 

  // Calculate N Factor, actually a x16 version to avoid floats
  Nx16  = ((ClkSrc==UartUseAclk? AClkHz : MClkHz) << 4)/BaudRate;
  
  //  If Divisor correspond to special numbers (69) or (34), use Oversampling
  //  with both modulators to reduce error (SLA144 Table 15-5, UCBRx 4 or 2)
  //  UCBR=69   UCBRFx=3 UCBRSx=5   UCOS16=1
  //  UCBR=34   UCBRFx=2 UCBRSx=3   UCOS16=1
  if( n == 69  || n == 34 )  // 
  {
    UART_BR0 = ((n==69)? 4 : 2);
    UART_BR1 = 0;
    UART_BRMCTL = ((n==69)? 0x3A : 0x26) | UCOS16;
  }
  else
  if(OverSampling)   // Set First Modulation register UCBRFx 
  {
    // Calculate Prescaler UCBRx = INT(N/16).
    UART_BR0 = Nx16>>8;     //  UCBR = N/16, 
    UART_BR1 = Nx16>>16;    
    //  Calculate First modulation at MSP430_UCBRMCTL bits 7 to 4 and UCOS at bit 0
    //  UCBRFx = Round ((N/16 -INT(N/16)) *16 )
    UART_BRMCTL = ((( (Nx16 & 0x00F8) + 0x08)) & 0xF0) | UCOS16;

  }
  else              // Set Second Modulation Register UCBRSx
  { 
    // Calculate Prescaler UCBRx = INT(N).
    UART_BR0 = Nx16>>4;     //  UCBR = N, 
    UART_BR1 = Nx16>>12;    //  
    
    //  Second Modulation register at MSP430_UCBRMCTL bits 3 to 1
    //  UCBRSx = round( ( N − INT(N) ) * 8 )
    UART_BRMCTL = ((Nx16 & 0xF) +1 ) & 0x0E;
  }
  if(ClkSrc == UartUseSmclk)
    UART_CTL1 |=UCSSEL1;                            // transmit control - use MCLK 
  else
    UART_CTL1 |=UCSSEL0;                           // Use UartUseAclk
  
  return  FinalBaud;
}


//==============================================================================
// Routine Name: UartSend
// Date Created: February 7, 2002
// Author: SLTF
// Description: This function will send a character to the serial port via the
//                transmit interrupt.
// Input: Data - character to be transmitted
// Output: None
//------------------------------------------------------------------------------
void UartSend(BYTE Data)
{
  BYTE i;
 

/* get the next place in the buffer for storing a byte */
  i = OutQueH + 1;
  if (i >= OutQLen) {
    i = 0;
  }

/* if the buffer is full, wait for it to empty by one character */
  while (i == OutQueT) {
  };

  __disable_interrupt();

/* add the data into the buffer for output */
  OutQueH = i;
  TxBuffer[OutQueH] = Data;

/* if the system hasn't been started, then the start it */
  if (XMitFlag == FALSE) {
    UC1IE |=  UCA1TXIE;                         //*MH IFG2 |= UTXIFG1;
  }

/* character being sent */
  XMitFlag = TRUE;

  __enable_interrupt();

/* keep track of characters sent */
  UartHal.TxCharCount++;
}



//==============================================================================
// Routine Name: UartTxIsr
// Date Created: November 1, 2001
// Author: SLTF
// Description: This is the transmit interrupt service routine for the UART.
// Input: None
// Output: None
//------------------------------------------------------------------------------
#pragma vector=USCIAB1TX_VECTOR           //*MH
__interrupt void UartTxIsr(void)
{
  WORD i;

/* see if there is any data to write, if not then return */
  if (OutQueH == OutQueT) {
    XMitFlag = FALSE;
    UC1IE &= ~UCA1TXIE;   //*MH F2417 uses single flag. Disable interrupt if TxQueue is empty
  } else {

#if 0
/* enable the RS-485 driver for one character time */
    PORT_SERIAL_DIR_SEL |= MASK_SERIAL_DIR;
    i = TAR + UartTxDelay ;     /* driver enable delay */
    if (i >= CCR0) {
      i -= CCR0;
    }
    CCR2 = i;                   /* at this time, clear the driver enable */
    CCTL2 = OUTMOD_0 | OUT;     /* enable the driver */
    CCTL2 = OUTMOD_5;
#endif
/* get the data to be written to the UART */
    if (++OutQueT >= OutQLen) {
      OutQueT = 0;
    }

/* now output the data */
    UCA1TXBUF = TxBuffer[OutQueT];              //*MH
  }
}

//==============================================================================
// Routine Name: UartRxIsr
// Date Created: February 8, 2002
// Author: SLTF
// Description: This is the receive interrupt service routine for the UART.
// Input: None
// Output: None
//------------------------------------------------------------------------------
#pragma vector=USCIAB1RX_VECTOR           //*MH 
__interrupt void UartRxIsr(void)
{
  BYTE i, Data;
  
  //------------------------------------------
  // get the data to clear the interrupt source 
  Data = UCA1RXBUF;                       //*MH
  
  /* go to the next location in the buffer */
  i = InQueH + 1;
  if (i >= InQLen) 
    i = 0;
  /* if room in the buffer, go ahead and save the data in it */
  if (i != InQueT) 
  {
    InQueH = i;
    RxBuffer[InQueH] = Data;
  }
  //--------------------------------
  schedWord |= SCHEDWORD_UARTRECEIVE; 
}




/***************************************************************************

  Routine Name: UartLegal

  Date Created: December 2, 2002

        Author: SLTF

   Description: This function will check the legality of the data and if a
                problem, this will reset the data.

         Input: None

        Output: None

                    Changes
   Number       Date        Initials    Description

**************************************************************************/

static void UartLegal(void)

{
  BOOLEAN fReset;

/* look for an erased flash data area */
  fReset = FlashDataErased(&SerialPort, sizeof(TSerialPort));

/* check the data for legality */
  if (SerialPort.Baud >= UART_NUM_BAUD_RATES) {
    fReset = TRUE;
  }
  if (SerialPort.DataBits >= NumSerialBits) {
    fReset = TRUE;
  }
  if (SerialPort.Parity >= NumSerialParity) {
    fReset = TRUE;
  }
  if (SerialPort.StopBits >= NumSerialStopBits) {
    fReset = TRUE;
  }

/* if a problem, reset the data */
  if (fReset == TRUE) 
  {
    UartResetData();
  }
}


//==============================================================================
// Routine Name: UartResetData
// Date Created: Revised September 12, 2011
// Author: MH
// Description: This function will set the data back to a known default
//                condition.
// Input: None
// Output: None
//------------------------------------------------------------------------------
void UartResetData(void)
{
  
  static const TSerialPort TempSerialPortModbus = {
    UART_DEFAULT_BAUD_RATE,    /* baud rate */
    Eight,                      /* data bits */
    ParityNone,                 /* parity */
    One,                        /* stop bits */
    1                           /* device address */
  };

/* initialize the data based on the protocol */
    FlashWrite(&SerialPort, (void *)&TempSerialPortModbus, sizeof(TSerialPort));
  
}


