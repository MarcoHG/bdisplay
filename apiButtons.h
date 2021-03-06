#ifndef __BUTTONS_API_H
#define __BUTTONS_API_H
/************************************************************************/
/*                                                                      */
/*              DEVELOPED BY:   West Coast Controls Co                  */
/*                              17150 Septo st,                         */
/*                              Northridge CA 91325                     */
/*                              (818) 718 1742                          */
/*                              www.wccontrols.com                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/

/*************************************************************************
*   $MODULE
*   apiButtons.h
*
*************************************************************************/

/*************************************************************************
*   $INCLUDES
*************************************************************************/

/*************************************************************************
*   $DEFINES
*************************************************************************/
#define NO_KEY  0x00
#define MASK_INC_KEY_ISR 0x01
#define MASK_DEC_KEY_ISR 0x02

/*************************************************************************
*   $LOCAL PROTOTYPES
*************************************************************************/

/*************************************************************************
*   $GLOBAL PROTOTYPES
*************************************************************************/
void KeyPressedTask(void);
void KeyPolllingTask(void);
void ResetCounterButton(void);
void IncDecButtonAuto(void);
/*************************************************************************
*   $GLOBAL VARIABLES
*************************************************************************/
extern BYTE KeyPressed;
extern WORD NoKeyTimer;
#endif
