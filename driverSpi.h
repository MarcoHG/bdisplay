#ifndef __SPI_OPS_H
#define __SPI_OPS_H
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

/*************************************************************************
*   $MODULE
*   SPI_ops.h
*
*************************************************************************/

/*************************************************************************
*   $INCLUDES
*************************************************************************/
#include "define.h"
#include "apiDisplay.h"

/*************************************************************************
*   $DEFINES
*************************************************************************/
#define SPI_LEDDRV    0
#define LEDDRV__NORMAL_MODE  0
#define LEDDRV_SPECIAL_MODE  1

/*************************************************************************
*   $LOCAL PROTOTYPES
*************************************************************************/

/*************************************************************************
*   $GLOBAL Structure
*************************************************************************/


/*************************************************************************
*   $GLOBAL PROTOTYPES
*************************************************************************/
void SpiComm(BYTE NumBytes, BYTE *MsgPtr, BYTE ICforComm);
void OpenSpiChannel(BYTE NumBytesHeader, BYTE *HeaderPtr, BYTE ICforComm);
void XchByteSpiChannel(BYTE *bXdata);
BYTE SendReceiveByteSpiChannel(BYTE TxData);
void CloseSpiChannel(BYTE ICforComm);
SetCurrentGainTlc5917(TDisplaySettings *pDisplay);
void SwitchTLC5917Mode(BYTE Mode);

#endif 
