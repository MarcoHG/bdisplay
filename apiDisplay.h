#ifndef __DISPLAY_API_H
#define __DISPLAY_API_H
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
*   apiDisplay.h
*
*************************************************************************/

/*************************************************************************
*   $INCLUDES
*************************************************************************/

/*************************************************************************
*   $DEFINES
*************************************************************************/
//#define DISPLAY_UPDATE_MSTIME   50   /* Time mS to Update Display */
#define NUM_DISPLAY_DIGITS      5     /*TODO: Future versions variable  */
typedef struct
{
  BYTE  NumDisplayDigits;
  BYTE  CurrentGain;
  BOOLEAN HiCurrentGain;
} TDisplaySettings;

typedef enum 
{
  BLANK_DISPLAY=0,
  INIT_DISPLAY,
  COUNTER_DISPLAY,
  GOAL_MODIFY_DISPLAY,
  RC_TEST_DISPLAY
} TDisplayMode;

/*************************************************************************
*   $LOCAL PROTOTYPES
*************************************************************************/

/*************************************************************************
*   $GLOBAL PROTOTYPES
*************************************************************************/
void  DisplayInit(void);
void  UpdateDisplay(void);
void  DisplayOn(void);
void  DisplayOff(void);
void DisplayDefaults(void);


/*************************************************************************
*   $GLOBAL VARIABLES
*************************************************************************/
extern long int counter;    //TODO: cleanup
extern int DISPLAY_UPDATE_MSTIME;//   50   /* Time mS to Update Display */
extern TDisplayMode DisplayMode;
extern TDisplaySettings DisplaySettings;
/*************************************************************************
*   $LOCAL VARIABLES
*************************************************************************/

#endif
