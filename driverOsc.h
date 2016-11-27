#ifndef __OSC_H
#define __OSC_H

/*************************************************************************
*   $MODULE
*
*
*************************************************************************/

/*************************************************************************
*   $INCLUDES
*************************************************************************/

/*************************************************************************
*   $DEFINES
*************************************************************************/

// Define the frequency of MCLK and SMCLK
#define MCLK_HZ   8000000
#define CALBC1    CALBC1_8MHZ  /* Calibrate DCO to 1MHz using calibration constants */
#define CALDCO    CALDCO_8MHZ
#define XCRYSTAL  32768
  



/*************************************************************************
*   $GLOBAL PROTOTYPES
*************************************************************************/
//void OscillatorTimeUpdate(void);  need to correct DCO if possible
void OscillatorInit(void);

/*************************************************************************
*   $GLOBAL VARIABLES
*************************************************************************/
extern long MClkHz;             /* MCLK frequency in Hz */
extern const long AClkHz;       /* ACLK frequency in Hz */
extern const long DcoHz[];     /* Preset DCO frequencies */
extern int DcoCounts;

/************************************************************************/


#endif  // ifndef __OSC_H
