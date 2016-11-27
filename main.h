//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  MODULE: MAIN.H
//------------------------------------------------------------------------------
#ifndef __MAIN_H
#define __MAIN_H

//========================
//  DEFINES
//========================
#define SUCCESSFUL    1
#define UNSUCCESSFUL  0

//==== TASKS =====
/* Task Scheduler Events in Time base ISR  */
#define SCHEDWORD_TIMEBASE_TICK           0x01
#define SCHEDWORD_DCOADJUST               0x02
#define SCHEDWORD_UARTRECEIVE             0x04
#define SCHEDWORD_UPDATEDISPLAY           0x08

#define SCHEDWORD_COUNTTASK               0x10
#define SCHEDWORD_INCDEC_BUTTONS          0x20
#define SCHEDWORD_KEYPOLL                 0x40
#define SCHEDWORD_RESET_BUTTON            0x80

#define SCHEDWORD_TICKBLINK               0x0100
#define SCHEDWORD_TICKSEC                 0x0200
#define SCHEDWORD_TICKMILLISEC            0x0400
#define SCHEDWORD_READINGSTATEMACHINE     0x0800

// Time to Take decisions on Counter value
#define COUNTTASK_MSTIME 100
#define MAX_COUNTER 99999L
#define MIN_COUNTER 0L


//========================
//  GLOBAL DATA PROTOTYPES
//========================
//  Globals are initialized to zero in ANSI C, i.e. linker will locate to RAM, we initialize to any value 
//  and then set load section to "No" in linker placement file
typedef struct 
{
  WORD a,b,c;
} tresamigos;

//========================
//  GLOBAL VARIABLES
//========================
extern tresamigos MyFriend;
extern unsigned int schedWord;
extern int  buttons_delta;



//========================
//  GLOBAL FUNCTION PROTOTYPES
//========================
void delay(unsigned int d);
void TimerClockManager(void);
void TaskNull(void);
void TaskScheduler(void);

#endif


