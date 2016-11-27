#ifndef __FLASH_H
#define __FLASH_H

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
//
//*MH These are the definitions for Non-Volatile Memory in INFO Flash
// 
#define INFO_FLASH_START 0x1000
#define INFO_FLASH_END   0x10BF
#define INFO_FLASH_SIZE (INFO_FLASH_END - INFO_FLASH_START +1)
#define FLASH_ADR_START ((BYTE *)INFO_FLASH_START)
#define FLASH_ADR_END   ((BYTE *)INFO_FLASH_END)
#define LOCATED_IN_DFLASH(A) (A >= FLASH_ADR_START && A <= FLASH_ADR_END)

/*************************************************************************
*   $GLOBAL PROTOTYPES
*************************************************************************/
void FlashInit(void);
void FlashWrite(void *Dest, void *Source, int NumBytes);
void FlashErase(int Segment);
BOOLEAN FlashDataErased(void *Address, BYTE Length);
BOOLEAN FlashAddress(void *Address);

/*************************************************************************
*   $GLOBAL VARIABLES
*************************************************************************/


/************************************************************************/

#endif
