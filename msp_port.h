#ifndef __MSP_PORT_H
#define __MSP_PORT_H

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
*   msp_port.h
*   Module intends to abstract the msp430 hardware from the application 
*
*************************************************************************/

/*************************************************************************
*   $INCLUDES
*************************************************************************/
#include <msp430x241x.h>

/*************************************************************************
*   $DEFINES
*************************************************************************/
//  We are using the 64 pin version TPM
//  PIN ID        NO. I/O Default   Description         
//
// P1.0/TACLK     12  O     H       Status LED 
// P1.1/TA0       13  O     H       BL_TXD
// P1.2/TA1       14  I     H       RCRXD (TA1)
// P1.3/TA2       15  O     L       RS485-DE/RE
// P1.4/SMCLK     16  I     -       Test point 
// P1.5/TA0       17  I     -       Test point
// P1.6/TA1       18  I     -       Test point
// P1.7/TA2       19  I     H       MRES(eset) counter
//
// P2.0/ACLK      20  I     H       INC(crease)
// P2.1/TAINCLK   21  I     H       DEC(crease)
// P2.2/CAOUT/TAO 22  I     H       BL_RXD
// P2.3/CA0/TA1   23  I     -       NC
// P2.4/CA1/TA2   24  I     -       NC
// P2.5/ROSC      25  I     -       Test point
// P2.6           26  I     -       Test point
// P2.7/TA0       27  I     -       Test point

// P3.0/STEO      28  I     -       NC
// P3.1/SIMO      29  O     L       UCB0SIMO 
// P3.2/SOMI      30  I     H       UCB0SOMI 
// P3.3/UCLK      31  O     L       UCB0CLK
// P3.4/UTXD0     32  I     -       Test point
// P3.5/URXD0     33  I     -       Test point
// P3.6/UTXD1     34  O     L       Serial TXD
// P3.7/URXD1     35  I     H       Serial RXD
//
// P4.0/TB0       36  O     L       LE (Latch Enable)
// P4.1/TB1       37  O     H       OE# (Output Enable)
// P4.2/TB2       38  I     -       NC
// P4.3/TB3       39  I     -       NC
// P4.4/TB4       40  I     -       NC
// P4.5/TB5       41  O     H       L1 (Lamp1 Goal reached, LO= Turn ON Light)
// P4.6/TB6       42  O     H       L2 (Lamp2 Machine Stopped, LO= Turns ON Light)
// P4.7/TBCLK     43  I     H       External Counter Input  (TBCLK)
//
// P5.0/STE1      44  I     -       Test point
// P5.1/SIMO      45  I     -       Test point
// P5.2/SOMI      46  I     -       Test point
// P5.3/UCLK1     47  I     -       Test point
// P5.4/MCLK      48  I     -       Test point
// P5.5/SMCLK     49  I     -       NC
// P5.6/ACLK      50  I     -       NC
// P5.7/TBOUTH    51  I     -       NC
//
// P6.0/A0        59  I     L       AIN0
// P6.1/A1        60  I     L       AIN1
// P6.2/A2        61  I     -       NC
// P6.3/A3        2   I     -       NC
// P6.4           3   I     -       NC
// P6.5           4   I     -       NC
// P6.6           5   I     -       NC
// P6.7           6   I     -       NC
//

/* CCS doesn't provide the const address of Port */
#define P3OUT_  (0x0019)  /* Port 3 Output */
#define P3SEL_  (0x001B)  /* Port 3 Selection */
#define P4OUT_  (0x001Du)  /* Port 4 Output */



/*  Digital I/O   */
//
//  Increase - Decrase Goal
#define PORT_INCDEC_BUTTONS_OUT P2OUT
#define PORT_INCDEC_BUTTONS_IN P2IN
#define MASK_INC_BUTTON 0x01
#define MASK_DEC_BUTTON 0x02
#define INCDEC_BUTTONS_IES  P2IES
#define INCDEC_BUTTONS_REN  P2REN
#define INCDEC_BUTTONS_IE   P2IE
#define INCDEC_BUTTONS_IFG  P2IFG


/*  SPI definitions         */
//#define UCB0

/* Led Drivers I/O              */
#define PORT_LEDDRV_OE    P4OUT_
#define MASK_LEDDRV_OE    0x02
#define PORT_LEDDRV_LE    P4OUT_
#define MASK_LEDDRV_LE    0x01

/* Led Driver SPI function selection */

#define PORT_LEDDRV_SCLK            P3OUT_
#define PORT_LEDDRV_SOMI            P3IN
#define PORT_LEDDRV_SEL             P3SEL_
#define MASK_LEDDRV_SCLK            0x08
#define MASK_LEDDRV_SOMI            0x04
#define MASK_LEDDRV_SIMO            0x02

/*  IR Remote Control Receiver */
// P1.2/TA1       14  I     H       RCRXD (TA1)
#define RCRXD_PORT_SEL  P1SEL
#define RCRXD_MASK_RCRXD 0x04
#define RCRXD_CAPTURE_REG TACCR1

/*  DCO adjustment by Crystal */
#define CRYSTAL_CCR TACCR2
#define CRYSTAL_CCTL TACCTL2

/*  Relays GPIO */
#define PORT_RELAY_GOAL    P4OUT
#define MASK_RELAY_GOAL    0x40
#define PORT_RELAY_STOP    P4OUT
#define MASK_RELAY_STOP    0x20

/* Counter Control  */
// P1.7/TA2       19  I     H       MRES(eset) counter
#define MASK_RESET_BUTTON 0x80
#define RESET_BUTTON_IE   P1IE
#define RESET_BUTTON_IES  P1IES
#define RESET_BUTTON_IFG  P1IFG

// P4.7/TBCLK     43  I     H       External Counter Input  (TBCLK)
#define PORT_XCOUNTER_SEL   P4SEL
#define MASK_XCOUNTER       0x80


//
// P2.0/ACLK      20  I     H       INC(crease)
// P2.1/TAINCLK   21  I     H       DEC(crease)
// P4.7/TBCLK     43  I     H       External Counter Input  (TBCLK)

/*  
    Universal Serial Communication Interface
    UART is implemented in Module USCI_A1
    Pins: 
        P3.7 UCA1RXD
        P3.6 UCA1TXD
*/

// Baud Rate Control Register
#define UART_BR0 UCA1BR0
#define UART_BR1 UCA1BR1
//
//  Baud Rate Modulation Control Register
//  7                   4    3                1         0
//  +----------------------------------------------------------+
//  | First Mod. Register | Second Mod. Register |  Oversample |
//  |        UCBFx        |         UCBSx        |   UCOS16    |
//  +----------------------------------------------------------+
#define UART_BRMCTL  UCA1MCTL

// USCI Control Register 0 and 1
#define UART_CTL0 UCA1CTL0
#define UART_CTL1 UCA1CTL1

// USCI Interrupts
#define UART_IE UC1IE
#define UART_RXIE UCA1RXIE
#define UART_TXIE UCA1TXIE 

// USCI pins function and I/O Direction
#define UART_PORT_SEL P3SEL
#define UART_MASK_TX          0x40
#define UART_MASK_RX          0x80


#define PORT_SERIAL_DIR_SEL     P1SEL
#define PORT_SERIAL_DIR         P1OUT
#define MASK_SERIAL_DIR         0x08
//*MH added Peripheral Function Select
#define MASK_SERIAL_TX          0x40
#define MASK_SERIAL_RX          0x80

/*************************************************************************
*   $LOCAL PROTOTYPES
*************************************************************************/
typedef struct {
  unsigned char *Port;
  unsigned char Mask;
} TIOPort;

/*************************************************************************
*   $GLOBAL VARIABLES
*************************************************************************/

/*************************************************************************
*   $LOCAL VARIABLES
*************************************************************************/

void InitIOPorts(void);



#endif
