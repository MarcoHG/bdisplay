//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  MODULE: driverflash.C
//------------------------------------------------------------------------------
/************************************************************************/
/*                                                                      */
/*              DEVELOPED BY:   SLTF CONSULTING                         */
/*                              COLUMBIA, MARYLAND USA  21045           */
/*                              (410) 799-3915                          */
/*                              www.sltf.com                            */
/*                              info@sltf.com                           */
/*                                                                      */
/************************************************************************/
/*************************************************************************
*   $MODULE
*   flash.c
*
*   The flash memory can be read with any memory reference instructions.
*   To write to flash, you must call FlashWrite().
*
*************************************************************************/

//=======================
// INCLUDES
//=======================  
#include <string.h>
//#include "in430.h"
#include  <msp430x26x.h>
#include "msp_port.h"
#include "define.h"
//#include "mi_app.h"
#include "driverFlash.h"
#include "driverOsc.h"
#include "Error.h"

//========================
//  LOCAL DEFINES
//========================
#define FLASH_FTG   367       /* flash timing generator frequency (KHz) */


//================================
//  LOCAL PROTOTYPES. 
//================================



//========================
//  GLOBAL DATA
//========================
//extern BYTE Info1Start;         /* in CSTART.S43 */
//========================
//  LOCAL DATA
//========================

//==============================================================================
// FUNCTIONS
//==============================================================================  
 
//==============================================================================
// Routine Name: FlashInit
// Date Created: February 27, 2002
// Author: SLTF
// Description: This task will check the status byte of the EEPROM and ensure
//                that the EEPROM is in write protected mode and it will also
//                count up the number of bytes that are to be stored in each
//                site ... for future error checking use ...then it will verify
//                the memory performance of the chip.
// Input: None
// Output: None
//------------------------------------------------------------------------------
void FlashInit(void)
{
  int Freq;
  //--------------------------
  // calculate the frequency 
  // for the flash timing generator 
  Freq = ((int)(MClkHz / 1000L) + FLASH_FTG / 2) / FLASH_FTG;
  if (Freq < 0) 
  {
    Freq = 0;
  }
  Freq &= (FN5 + FN4 + FN3 + FN2 + FN1 + FN0);
  //------------------------------------
  // program the flash timing register 
  FCTL2 = FWKEY + FSSEL_1 + Freq;
  //-----------------------------
  // protect the flash memory 
  FCTL1 = FWKEY;
  FCTL3 = FWKEY + LOCK;
}

//==============================================================================
// Routine Name: FlashWrite
// Date Created: April 19, 2010
// Author: MH
// Description: This function controls the writing of information to the flash
//                info banks B,C and D (B-D) memory. The sequence is the following:
//                  - Copy Banks B-D to RAM (for temporary storage)
//                  - Erase Flash B-D
//                  - Copy from RAM to Flash banks B-D
// Input: Dest - a pointer to the location in the flash memory
//      Source - a pointer to the data to write into flash
//    NumBytes - the number of bytes to write into flash
//    Note: Address validation is done outside
//
// Output: Global Error Flag is set if Write to Flash was not succesful
//------------------------------------------------------------------------------
void FlashWrite(void *Dest, void *Source, int NumBytes)
{
  BYTE *ptr;
  WORD StatusRegister;
  WORD i;
  // define the RAM buffer
  static BYTE ram[INFO_FLASH_SIZE]; // Three sectors (candidate to malloc)
  
  //-----------------------------
  // if the memory hasn't changed, then we're done 
  if (memcmp(Dest, Source, NumBytes) == 0) 
  {
    return;
  }
  // Linker will set NVMEM_START @ 0x1000
  
  // Create a modified image of Flash in RAM
  memcpy(ram,FLASH_ADR_START, INFO_FLASH_SIZE);
  i = (WORD)Dest - (WORD)FLASH_ADR_START;                 // Find the index where to write in RAM
  memcpy(&ram[i],Source,NumBytes);
  //-----------------------------
  // Erase three banks of INFO flash B to D
  // INFOD=1000-103F, INFOC=1040-107F, INFOB=1080-10BF
  //
  // turn off interrupts 
  StatusRegister = _BIC_SR(GIE);
  //-------------------------------
  // set up the flash for writing 
  FCTL3 = FWKEY;
  //----------------------
  // Erase INFOB to INFOD in F241x
  for(i=0; i< 0xC0; i +=0x40)
  {
    FCTL1 = FWKEY + ERASE;          // Setup to Erase
    ptr = FLASH_ADR_START + i;
    *ptr= 0x00;                     // Dummy write erases the sector
  }
  //--------------------------
  // set up the flash for writing 
  FCTL3 = FWKEY;
  FCTL1 = FWKEY + WRT;
  
  // Copy Modified Ram image to Flash
  ptr = FLASH_ADR_START;  // Index 
  for(i=0; i < INFO_FLASH_SIZE; ++i)
  {
    ptr[i] = ram[i];
    while (FCTL3 & BUSY) {};  //  Wait until writting cycle
    if (ptr[i] != ram[i])     //  Verify Data
      {
        ErrorFlag(TRUE, FailureFlashWrite);
        break;                // No need to continue copying RAM
      }
  }
  
  //----------------------------------------
  // lock the flash from further writing 
  FCTL1 = FWKEY;
  FCTL3 = FWKEY + LOCK;
  //---------------------------------
  // restore the interrupt enable bit 
  _BIS_SR(StatusRegister);
  // Erase Done or FAULT

}





//==============================================================================
// Routine Name: FlashDataErased
// Date Created: December 2, 2002
// Author: SLTF
// Description: This function will return TRUE if the designated memory range
//                is set to the erased condition of the flash memory, i.e., 0xff.
// Input: Address - the starting address
//                Length - the length (number of bytes) to check.
// Output: TRUE if the data area is erased, otherwise FALSE
//------------------------------------------------------------------------------
BOOLEAN FlashDataErased(void *Address, BYTE Length)
{
  BOOLEAN fErased;
  BYTE i, *ptr;
  //------------------------------------
  // assume that the memory is erased 
  fErased = TRUE;
  ptr = Address;
  //--------------------------------------
  // check all the requested memory locations 
  for (i = 0; i < Length; i++) 
  {
    //-------------------------------------
    // if the location is not erased, then 
    // we're done 
    if (*ptr++ != (BYTE)0xff) 
    {
      fErased = FALSE;
      break;
    }
  }
  //---------------------------------------
  // return the flag for what was found 
  return fErased;
}

//==============================================================================
// Routine Name: FlashAddress
// Date Created: December 9, 2002
// Author: SLTF
// Description: This function will return TRUE if the address is in the flash
//                memory area or FALSE if it is in RAM.
// Input: Address - the memory address to check
// Output: TRUE if flash address, otherwise FALSE
//------------------------------------------------------------------------------
BOOLEAN FlashAddress(void *Address)
{
  BOOLEAN fFlag;
  //*MH This is a temporal FIX just to release 3.0.01 == Two Flash spaces for data
  if ( LOCATED_IN_DFLASH((BYTE *)Address))       // ((BYTE *)Address >= FLASH_ADR_START && (BYTE *)Address <= FLASH_ADR_END)
     
  {
    fFlag = TRUE;
  } 
  else 
  {
    fFlag = FALSE;
  }
  return fFlag;
}
