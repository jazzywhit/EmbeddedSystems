/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 5   -   Jesse Whitworth
-----------------------------------------------------------------------
			         Bfr.c
-----------------------------------------------------------------------
From Program 3, modified as needed. You must remove the member “closed” from
type CircBfr, and also remove the 3 functions “BfrClosed(),” “BfrClose(),” and
“BfrOpen().” You may add new functions to supply any needed functionality
*/

#include "Bfr.h"

/*-------------------- B f r I n i t ( ) -------------------------------------
	Purpose:	Initialize a circular buffer: record the size; set in, out, and numBytes
                        to zero; and mark the buffer open. Make bfr point to bfrSpace.
        Parameters:     buffer address, address of the buffer data space, buffer capacity in bytes
        Return Value:   None
*/
CPU_VOID BfrInit( CircBfr *bfr, CPU_INT08U *bfrSpace, CPU_INT16U bfrSize){
  bfr->size = bfrSize;
  bfr->bfr = bfrSpace;
  BfrReset(bfr);
}

/*-------------------- B f r R e s e t ( ) -------------------------------------
	Purpose:	Reset the buffer: reset in, out, and numBytes to zero.
        Parameters:     buffer address
        Return Value:   None
*/
CPU_VOID BfrReset(CircBfr *bfr){
  bfr->out = bfr->in = bfr->numBytes = 0;
}

/*-------------------- B f r F u l l ( ) -------------------------------------
	Purpose:	Test whether or not a buffer is full.
        Parameters:     buffer address
        Return Value:   Success - TRUE if full
                        Failure - Otherwise FALSE
*/
CPU_BOOLEAN BfrFull(CircBfr *bfr){
    return (bfr->numBytes >= bfr->size);
}

/*-------------------- B f r E m p t y ( ) -------------------------------------
	Purpose:	Test whether or not a buffer is empty.
        Parameters:     buffer address
        Return Value:   Success - TRUE if empty
                        Failure - Otherwise FALSE
*/
CPU_BOOLEAN BfrEmpty(CircBfr *bfr){
    return (bfr->numBytes <= 0);
}

/*-------------------- B f r A d d B y t e ( ) -------------------------------------
	Purpose:	Add a byte to a buffer at position “in” and increment “in” by 1.
        Parameters:     buffer address, byte to be added
        Return Value:   Success - Byte that has been added, unless buffer is full
                        Failure - Buffer is full, return -1
*/
CPU_INT16S BfrAddByte(CircBfr *bfr, CPU_INT16S theByte){
    CPU_SR_ALLOC();   //CPU_INT32U sr = 0; Necessary to disable/enable interrupts in uC/OSIII
      
    if (BfrFull(bfr))
        return -1;
    
    *(bfr->bfr+bfr->in) = theByte;
    bfr->in = (bfr->in + 1) % bfr->size;
    
    //Disable interrupts to avoid data hazards
    OS_CRITICAL_ENTER();
    bfr->numBytes++;
    OS_CRITICAL_EXIT();
    
    return theByte;
}

/*-------------------- B f r R e m B y t e ( ) -------------------------------------
	Purpose:	Return the byte from position “out” and increment “out” by 1
        Parameters:     buffer address
        Return Value:   Success - Returns the byte from position 'out' unless empty
                        Failure - If empty return -1
*/
CPU_INT16S BfrRemByte(CircBfr *bfr){
    CPU_SR_ALLOC();   //CPU_INT32U sr = 0; Necessary to disable/enable interrupts in uC/OSIII
    
    if (BfrEmpty(bfr))
        return -1;
    
    CPU_INT16S tempByte = *(bfr->bfr+(bfr->out));
    bfr->out = (bfr->out + 1) % bfr->size;
    
    //Disable interrupts to avoid data hazards
    OS_CRITICAL_ENTER();
    bfr->numBytes--;
    OS_CRITICAL_EXIT();
    
    return tempByte;
}

/*-------------------- B f r N e x t B y t e ( ) -------------------------------------
	Purpose:	Return the byte from position “out” or return -1 if the buffer is empty.
        Parameters:     buffer address
        Return Value:   Success - Returns the byte from position 'out' unless empty
                        Failure - If empty return -1
*/
CPU_INT16S BfrNextByte(CircBfr *bfr){
    if (BfrEmpty(bfr))
        return -1;
    return *(bfr->bfr+(bfr->out));
}

/*-------------------- B f r W r i t e ( ) -------------------------------------
	Purpose:	Write a block of bytes to the buffer
        Parameters:     buffer address, address of block to be written, number of bytes to write
        Return Value:   None
*/
CPU_VOID BfrWrite( CircBfr *bfr, CPU_VOID *rec, CPU_INT08U size){
    CPU_INT08S *bytes = rec;
    for(;size > 0; size--){
        BfrAddByte(bfr, *bytes++);
    }
}

/*-------------------- B f r R e a d ( ) -------------------------------------
	Purpose:	Read a block of bytes from the buffer
        Parameters:     buffer address, address of returned block, number of bytes to write
        Return Value:   None
*/
CPU_VOID BfrRead( CircBfr *bfr, CPU_VOID *rec, CPU_INT08U size){
    CPU_INT08S *bytes = rec;
    for(;size > 0; size--){
        *bytes++ = BfrRemByte(bfr);
    }
}