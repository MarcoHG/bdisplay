#ifndef __TIMER_H
#define __TIMER_H
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
*   timer.h
*
*************************************************************************/
  
/*************************************************************************
*   $DEFINES
*************************************************************************/
//  Timer A uses the MCLK clock divided by 2^MCLK_DIVIDER :
//  - Generate a Time Base tick
//  - Decode the key pressed in IR Remote Control captured TACC1 
//  - Adjust DCO frequency using the XCRYSTAL reference on TACC2
//
#define MCLK_DIVIDER     3        /* Divide by 8 */
#define DCO_ADJUST_MSTIME  200    /* Adjust DCO every 200 ms  */
#define TIMEBASE_MSEC_ISR   20    /* Generate a time base tick every 20 mS */



/*************************************************************************
*   $LOUART_CTL1 PROTOTYPES
*************************************************************************/

/*************************************************************************
*   $GLOBAL PROTOTYPES
*************************************************************************/
void TimerInit(void);

/*************************************************************************
*   $LOUART_CTL1 VARIABLES
*************************************************************************/

/*************************************************************************
*   $GLOBAL VARIABLES
*************************************************************************/
extern BOOLEAN fTimerBlink;

#endif
