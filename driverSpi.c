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
*   driverSpi.c
*
*************************************************************************/

/*************************************************************************
*   $INCLUDES
*************************************************************************/
#include <stdio.h>
#include  <msp430x26x.h>
#include "define.h"
#include "msp_port.h"
#include "driverSpi.h"
#include "driverOsc.h"
#include "apiDisplay.h"



/*************************************************************************
*   $DEFINES
*************************************************************************/
/*  Spi Clock status when inactive */
#define SPI_CLKPL_INACTIVE_LO  0x00
#define SPI_CLKPL_INACTIVE_HI  UCCKPL
/* Spi Data at first Clock Edge */  
#define SPI_CLKPH_CHANGE_1ST  0x00
#define SPI_CLKPH_CAPTURE_1ST UCCKPH
/*  Spi data is Least/Most Significative Bit First */
#define SPI_LSB_FIRST 0x00
#define SPI_MSB_FIRST UCMSB


/*************************************************************************
*   $LOCAL PROTOTYPES
*************************************************************************/
typedef struct {
  WORD BrClk_Divider;
  BYTE ClockPolarity;           /* SPI_CLK_DATA_OUT edge */
  BYTE ClockPhase;              /* SPI_CLK normal or delayed 1/2 cycle */
  BYTE *PORT;                   /* address of port used by the IC */
  BYTE CS;                      /* bit location of chip select on PORT for IC */
  BYTE CSLevel;                 /* parameter indicating low, or high, CS */
  BYTE CSDuration;              /* Indicates the Chip Select (Latch Enable) activation:
                                    0 Activated before data xchange and last whole cycle (normal),
                                    <>0 CS is activated after transmission for SMCLK cycles 
                                */
  BYTE BitOrder;                /* LSB/MSB sent first: 1 = MSB sent first */
} TSpiData;

// Special Serial Data - Information to put the chip into special modes
// Uses Pins as GPIO for switching the Chip between Special and Normal modes
typedef struct {
  BYTE *CLKPORT;                /* address of port used as CLK by the IC */
  BYTE *CLKPORTSEL;             /* SELect peripheral address port used by SCLK */
  BYTE ClkPin;                  /* bit location of the Output Enable C */
  BYTE *LEPORT;                 /* address of port used as CLK by the IC */
  BYTE LePin;                   /* bit location of the Output Enable C */
  BYTE *OEPORT;                 /* address of port used as CLK by the IC */
  BYTE OePin;                   /* bit location of the Output Enable C */
  
} TSSerialData;


static void SpiInit(TSpiData *SpiPtr);
static void ClkHiLo(void);
static void WriteTLC5917Configuration(BYTE n, BYTE data);
void SwitchTLC5917Mode(BYTE Mode);
/*************************************************************************
*   $GLOBAL VARIABLES
*************************************************************************/

/*************************************************************************
*   $LOCAL VARIABLES
*************************************************************************/
static const TSpiData SPISettings[] = 
{

/* the Led Driver TLC5917 settings */
  {
    (MCLK_HZ/500000),     /* 500 KHz  */
    SPI_CLKPL_INACTIVE_LO,
    SPI_CLKPH_CAPTURE_1ST,
    (BYTE *)PORT_LEDDRV_LE, /* Port location of Latch Enable */
    MASK_LEDDRV_LE,         /* Bit location of LE in the Led driver */
    1,                      /* low or high chip select:0 = CS NOT */
    8,                      /* Latch enable at the end of serial stream  2/8MHz = 1uS*/
    SPI_MSB_FIRST           /* Led Driver sends MSB first         */
  }
};
// 
static const TSSerialData SSPISettings[] =
{
  {
    (BYTE *)PORT_LEDDRV_SCLK,     // Serial Clock data
    (BYTE *)PORT_LEDDRV_SEL,
    MASK_LEDDRV_SCLK,

    (BYTE *)PORT_LEDDRV_LE,       //  Latch Enable Data
    MASK_LEDDRV_LE,

    (BYTE *)PORT_LEDDRV_OE,       // Output Enable Data
    MASK_LEDDRV_OE
  }
  
};

/************************************************************************/


/***************************************************************************

  Routine Name: SpiInit

  Date Created: February 6, 2002

        Author: SLTF

   Description: This function configures the SPI port.

         Input: SpiPtr - a pointer of TSpiData to the configuration info

        Output: None

                    Changes
   Number       Date        Initials    Description
  1             3/31/11     *MH       Change SPI registers from F149 to F2417 

**************************************************************************/

static void SpiInit(TSpiData *SpiPtr)

{
	
	/* turn on the SPIMSP 430 receiver and transmitter enables */
  // ME1 = USPIE0;  //*MH Not supported

/* initialize the UCB0 control register (also clears ORed bits) */
  UCB0CTL1 = UCSWRST;            /* reset the control register */
  UCB0CTL0 = UCMST;              /* master mode, 3-pin SPI, 8-bit data */

  // Clock inactive status and Data at 1st clk edge, which bit LSB (0) or MSB(1) is sent first
  UCB0CTL0 |= SpiPtr->ClockPolarity | SpiPtr->ClockPhase | SpiPtr->BitOrder | SpiPtr->BitOrder;
  UCB0CTL1 |= UCSSEL0 | UCSSEL1;                // Spi uses SMCLK clock
  
/* set the baudrate */
  UCB0BR0 = SpiPtr->BrClk_Divider;    
  UCB0BR1 = SpiPtr->BrClk_Divider >> 8;
  
/* release the reset bit */
  UCB0CTL1 &= ~UCSWRST;            //*MH
}


static void ClkHiLo(void)
{
  P3OUT |= MASK_LEDDRV_SCLK;
  __no_operation();
  P3OUT &= ~MASK_LEDDRV_SCLK;
}

void SwitchTLC5917Mode(BYTE Mode)
{
  //  Set Clk SPI pins to IO Port mode
  P3SEL &=  ~MASK_LEDDRV_SCLK;
  
  // Start with CLK=0 (P3OUT)  LE=0 OE=1 (P4OUT) 
  P3OUT &=  ~MASK_LEDDRV_SCLK;
  P4OUT =  (P4OUT | MASK_LEDDRV_OE ) & ~MASK_LEDDRV_LE;
  
  // Make 101 sequence in OE
  P4OUT |=  MASK_LEDDRV_OE;   //1st:  1
  ClkHiLo();
  P4OUT &=  ~MASK_LEDDRV_OE;   //2nd:  0
  ClkHiLo();
  P4OUT |=  MASK_LEDDRV_OE;   //3rd:  1
  ClkHiLo();

  //  Switch to Mode in LE
  if(Mode == LEDDRV_SPECIAL_MODE) 
    P4OUT |=  MASK_LEDDRV_LE;   // 4th:  LE= 1 : Special Mode
  else
    P4OUT &=  ~MASK_LEDDRV_LE;  // 4th:  LE= 0 : Special Mode
  ClkHiLo();

  // Return LE to 0 at end of cycle
  P4OUT &=  ~MASK_LEDDRV_LE;    // 5th: Datasheet doesn't say much
  ClkHiLo();

  
  //  Return Clk SPI pins to Peripheral Function
  P3SEL |=  MASK_LEDDRV_SCLK;
  
  
}
//  Bit-bang Configuration Data into SPI bus
static void WriteTLC5917Configuration(BYTE n, BYTE data)
{
  BYTE i,j, mask;
  //  Set Clk & MOSI  SPI pins to normal IO mode
  P3SEL &=  ~MASK_LEDDRV_SCLK & ~MASK_LEDDRV_SIMO;  // MASK_LEDDRV_SIMO is already an Output
  //  OE =1 and LE=0
  P4OUT = (P4OUT | MASK_LEDDRV_OE) & ~MASK_LEDDRV_LE;
  
  // Transmit data in SIMO pin, MSB first
  for(j=0; j< n;++j)
  for(i=0, mask = data; i < 8; ++i)
  {
    if(mask & 0x80)  
      P3OUT |= MASK_LEDDRV_SIMO;
    else
      P3OUT &= ~MASK_LEDDRV_SIMO;;
    if(i ==7 && j== n-1) // If Last bit (of Last Byte) we latch data in configuration latch
    {
      P4OUT |= MASK_LEDDRV_LE;
      ClkHiLo();
      P4OUT &= ~MASK_LEDDRV_LE;
    }
    else
      ClkHiLo();
    mask <<=1;
    
  }

  //  Return Clk & MOSI  SPI pins to Peripheral Function
  P3SEL |=  MASK_LEDDRV_SCLK | MASK_LEDDRV_SIMO;
  
  
}
//  Configuration byte:
//  0   1   2   3   4   5   6   7
//  CM  HC  CC0 CC1 CC2 CC3 CC4 CC5
//  VG = (1 + HC) × (1 + D/64) / 4,   D = as given by CC[5:0] 
//  
//  Total current per segment is: Io= 1.26 V/Rext × VG × 15 × 3CM – 1
//  Or simply:
//    Io= 1.26 V/Rext × VG × 15   LedHiCurrentGain = 1
//    Io= 1.26 V/Rext × VG × 5    LedHiCurrentGain = 0

SetCurrentGainTlc5917(TDisplaySettings *pDisplay)
{
  BYTE config;

  SwitchTLC5917Mode(LEDDRV_SPECIAL_MODE);

  // Config has gain and 6 bits coded 
  config = pDisplay->CurrentGain << 2;  // 6 bits
  if(pDisplay->CurrentGain & 0x40)      // 7th
    config |= 0x02;
                                        // 8th is made by Hi Current Gain                
  if(pDisplay->HiCurrentGain) 
    config |= 0x01;
  
  WriteTLC5917Configuration(pDisplay->NumDisplayDigits,config);
  SwitchTLC5917Mode(LEDDRV__NORMAL_MODE);

}
      

/***************************************************************************

  Routine Name: SpiComm

  Date Created: February 6, 2002

        Author: SLTF

   Description: This function controls aqll the communications on the SPI
                bus.

         Input: NumBytes - the number of bytes to send and receive
                MsgPtr - pointer to the BYTE buffer to send and receive into
                ICforComm - index of the IC in the UART SPI Settings routine

        Output: None

                    Changes
   Number       Date        Initials    Description
  1             3/31/11     *MH       Change register names from F14x to F241x

**************************************************************************/

void SpiComm(BYTE NumBytes, BYTE *MsgPtr, BYTE ICforComm)

{
  BYTE *ptr;
  BYTE ChipSelectOn, ChipSelectOff;
  TSpiData *SpiPtr;


  /* intialize the MSP 430 software settings for the SPI port */
  SpiPtr = (TSpiData *)&SPISettings[ICforComm];
	SpiInit(SpiPtr);

  ptr = SpiPtr->PORT;
  /* set the IC chip select(CS) */
  ChipSelectOn = *ptr & ~SpiPtr->CS;
  if (SpiPtr->CSLevel == 1) 
  {
    ChipSelectOn |= SpiPtr->CS;
  }
  ChipSelectOff = ChipSelectOn ^ SpiPtr->CS;

  if(SpiPtr->CSDuration == 0)             /* Activate CS before transmission */
    *ptr = ChipSelectOn;
  
  while ((IFG2 & UCB0TXIFG) == 0x00);     //*MH allow finishing any pending transmision
    
/* send and receive all the data (highly optimized!) */
  do 
  {
    UCB0TXBUF = *MsgPtr;                    //*MH
    //asm("nop");asm("nop");                  //*MH Add small delay before polling
    while ((IFG2 & UCB0TXIFG) == 0x00) {    //*MH  Closer equivalent: U0TCTL & TXEPT  
   };
   //*MH Read what is in the receiver buffer
   //*MsgPtr = UCB0RXBUF;                    //*MH clears UCxRXIFG 
   //asm("nop");asm("nop");                  //*MH Add small delay before polling
   while((IFG2 &  UCB0RXIFG)==0);           //*MH  Wait for reception complete
   *MsgPtr = UCB0RXBUF;                     //*MH This also clears UCxRXIFG 


/* next location in the buffer */
    MsgPtr++;
  } while (--NumBytes != 0);
  
  if(SpiPtr->CSDuration)
  {
    BYTE cycles;
    *ptr = ChipSelectOn;
    // We need some minimum time here; give at least ONE clock cycle
    for(cycles = 0; cycles < SpiPtr->CSDuration; ++cycles)
      __no_operation();

  }
  *ptr = ChipSelectOff;                  /* Deactivate chip select */
  
}


/*---------------------------------------------------------------------------*/
/* function: OpenSpiChannel                                                  */
/*---------------------------------------------------------------------------*/
/*!
  \brief  Open the communication Channel of an Intelligent SPI device
  User sends a Header with a command to open the communication channel to a SPI device
  and get it ready to transfer data 
  Author: MH, May 7, 2011
  
  \param NumBytesHeader - lenght of header command in bytes
  \param HeaderPtr - Predefined Header to be sent
  \Param ICforComm - index of the IC in the UART SPI Settings routine

  \retval As a secondary result, spi device get initialized and waiting for data exchange

*/
void OpenSpiChannel(BYTE NumBytesHeader, BYTE *HeaderPtr, BYTE ICforComm)
{
  BYTE *ptr,a;
  BYTE ChipSelectOn;
  TSpiData *SpiPtr;

  /* intialize the MSP 430 software settings for the SPI port */
  SpiPtr = (TSpiData *)&SPISettings[ICforComm];
  SpiInit(SpiPtr);

  ptr = SpiPtr->PORT;

  /* set the IC chip select(CS) */
  ChipSelectOn = *ptr & ~SpiPtr->CS;
  if (SpiPtr->CSLevel == 1) 
  {
    ChipSelectOn |= SpiPtr->CS;
  }
  /* turn on the chip select signal */
  *ptr = ChipSelectOn;

  while ((IFG2 & UCB0TXIFG) == 0x00);     //*MH allow finishing any pending transmision
    
  /* send and receive all the data (highly optimized!) */
  do 
  {
    UCB0TXBUF = *HeaderPtr; 
    while ((IFG2 & UCB0TXIFG) == 0x00);   //  Tx buffer is empty and starts clocking out
    while((IFG2 &  UCB0RXIFG)==0);        //  Wait for reception complete 
    a = UCB0RXBUF;                  
    HeaderPtr++;
  } while (--NumBytesHeader != 0);

}

/*---------------------------------------------------------------------------*/
/* function: XchByteSpiChannel()                                             */
/*---------------------------------------------------------------------------*/
/*!
  \brief  Exchange one byte of data in a opened spi channel
  The user sends and receives in exchange, a single byte in an open spi channel 
  Author: MH, May 7, 2011
  
  \param pBdata - points to the byte to be exchanged

*/
void XchByteSpiChannel(BYTE *pBdata)
{
  BYTE RxData;
  while ((IFG2 & UCB0TXIFG) == 0x00);         // allow finishing any pending transmision, wait until tx buffer is empty
  /* send and receive one byte of data */
  UCB0TXBUF = *pBdata; 
  while ((IFG2 & UCB0TXIFG) == 0);         // Tx buffer is empty and starts clocking out
  while((IFG2 &  UCB0RXIFG)==0);           // All received bits ar in Rx buffer
  *pBdata = UCB0RXBUF;                      // return what is got at RX buffer
}

/*---------------------------------------------------------------------------*/
/* function: SendReceiveByteSpiChannel()                                             */
/*---------------------------------------------------------------------------*/
/*!
  \brief  Send and Receive one byte of data on a opened spi channel
  User calls this routine to send and receive one byte of data on a spi channel
  Author: MH, May 7, 2011
  
  \param TxData is the byte to send to the open spi channel

  \retval The received byte at the spi channel is retunr
*/
BYTE SendReceiveByteSpiChannel(BYTE TxData)
{
  BYTE RxData;
  while ((IFG2 & UCB0TXIFG) == 0x00);         // allow finishing any pending transmision, wait until tx buffer is empty
  /* send and receive one byte of data */
  UCB0TXBUF = TxData; 
  while ((IFG2 & UCB0TXIFG) == 0);         // Tx buffer is empty and starts clocking out
  while((IFG2 &  UCB0RXIFG)==0);           // All received bits ar in Rx buffer
  RxData = UCB0RXBUF;                      // return what is got at RX buffer
  return RxData;
}



/*---------------------------------------------------------------------------*/
/* function: CloseSpiChannel                                                  */
/*---------------------------------------------------------------------------*/
/*!
  \brief  Close the communication Channel of a Intelligent SPI device
  This command closes the communication with an intelligent SPI, deselecting it from the bus
  Author: MH, May 7, 2011
  
  \Param ICforComm - index of the IC in the UART SPI Settings routine

  \retval None, but as a secondary result the spi device gets unselected

*/
void CloseSpiChannel(BYTE ICforComm)
{
  BYTE *ptr;
  BYTE ChipSelectOff;
  TSpiData *SpiPtr;

  /* intialize the MSP 430 software settings for the SPI port */
  SpiPtr = (TSpiData *)&SPISettings[ICforComm];
  ptr = SpiPtr->PORT;

  /* set the IC chip select(CS) */
  if(SpiPtr->CSLevel == 1)
    ChipSelectOff = *ptr & ~SpiPtr->CS;
  else
    ChipSelectOff = *ptr | SpiPtr->CS;

  /* turn on the chip select signal */
  *ptr = ChipSelectOff;
}

