#ifndef __UART_H
#define __UART_H

/*************************************************************************
*   $MODULE
*   uart.h
*
*************************************************************************/

/*************************************************************************
*   $INCLUDES
*************************************************************************/
#include "msp_port.h"

/*************************************************************************
*   $DEFINES
*************************************************************************/
//  Baud rates here are index 0 to UART_NUM_BAUD_RATES-1 to Display and
//  configurate the port with new values once selected 
#define UART_NUM_BAUD_RATES   10      /* number of different baud rates */
#define UART_DEFAULT_BAUD_RATE (UART_NUM_BAUD_RATES -5)
/*************************************************************************
*   $LOCAL VARIABLES
*************************************************************************/

/*************************************************************************
*   $GLOBAL PROTOTYPES
*************************************************************************/
void UartBaudRateCalc(long UartClock, long BaudRate);

typedef enum 
{
  CharAvailable =         0x00,   /* character available */
  NoChar        =         0x01,   /* no character available */
  ModbusSilentInterval =  0x02,   /* an RTU framing character found */
  BreakDetect   =         0x10,
  OverrunError  =         0x20,
  ParityError   =         0x40,
  FramingError  =         0x80
} TUartStatus;

typedef enum 
{
  Seven =(BYTE)0,
  Eight,
  NumSerialBits
} TSerialBits;

typedef enum {
  One =(BYTE)0,
  Two,
  NumSerialStopBits
} TSerialStopBits;

typedef enum {
  ParityNone =(BYTE)0,
  ParityEven,
  ParityOdd,
  NumSerialParity
} TSerialParity;

typedef struct {
  BYTE Baud;
  TSerialBits DataBits;
  TSerialParity Parity;
  TSerialStopBits StopBits;
  BYTE Address;                 /* valid from 1 to 255 */
} TSerialPort;

/* the following are counters used for debugging serial communications */
typedef struct {
  unsigned long TxChar;
  unsigned long RxChar;
  unsigned long FramingError;
  unsigned long ParityError;
  unsigned long OverrunError;
  unsigned long RxError;
} TSerialStatus;

typedef struct 
{
  BYTE *RxBuffer;
  BYTE RxChar;
  BYTE RxCharCount;
  BOOLEAN RxBufferEmpty;        /* UART data */
  BYTE TxCharCount;
} TUartHal;

void UartInit(void);
void UartStart(void);
void UartRxTask(void);
void UartSend(BYTE Data);
void UartResetData(void);
/*************************************************************************
*   $Global VARIABLES
*************************************************************************/
extern BYTE UartChar;           /* the most current received data out of the FIFO */
extern TUartStatus UartCharStatus;      /* the status of UartChar */
extern TSerialStatus SerialStatus;      /* for debugging purposes */

extern TSerialPort SerialPort;

/* the pragma turns off a warning for unknown external array size */
//VBASE #pragma warnings=off
extern char *const strUartBaudRates[];
extern char *const strUartBits[];
extern char *const strUartStopBits[];
extern char *const strUartParity[];
extern TUartHal UartHal;
//VBASE #pragma warnings=default


#endif
