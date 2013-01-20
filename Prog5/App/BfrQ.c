/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 5   -   Jesse Whitworth
-----------------------------------------------------------------------
			        BfrQ.c
-----------------------------------------------------------------------
From Program 3, modified as needed. The two semaphores “readBfrs” and “writeBfrs”
are defined in this module as members of type BfrQ. You may add new functions to
supply any needed functionality.
*/

#include <string.h>
#include "Assert.h"
#include "BfrQ.h"

#define SuspendTimeout 100	    // Timeout for semaphore wait

/*-------------------- B f r Q I n i t ( ) -------------------------------------
	Purpose:	Initialize a buffer queue. Initialize the “numBfrs” and “bfrSize” members. Set
                        readBfrNum and writeBfrNum to zero. Finally, perform BfrInit() on all of the buffers in the queue.
        Parameters:     buffer queue address, number of buffers, size of each buffer (bytes), address of a block of memory large enough
        Return Value:   None
*/
CPU_VOID BfrQInit( BfrQ *bfrQ, CPU_INT08U numBfrs, CPU_INT08U bfrSize, CPU_INT08U *bfrSpace){
  CPU_INT08S i;
  
  /* O/S error code */
  OS_ERR  osErr;
  
  bfrQ->numBfrs = numBfrs;
  bfrQ->bfrSize = bfrSize;
  bfrQ->readBfrNum = 0;
  bfrQ->writeBfrNum = 0;
  
  for (i = 0; i < numBfrs; i++){
    BfrInit(&bfrQ->buffers[i], bfrSpace+(i*bfrSize)/*+1*/, bfrSize); 
  }
  
  /* Create and initialize semaphores. */
  OSSemCreate(&bfrQ->writeBfrs, "Write Bfrs Avail", NumBfrs, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  OSSemCreate(&bfrQ->readBfrs, "Read Bfrs Avail", 0, &osErr);
  assert(osErr == OS_ERR_NONE);
}


/*-------------------- B f r Q P e n d R e a d ( ) -------------------------------------
	Purpose:	Block if the buffer queue read buffer is not ready.
        Parameters:     buffer queue address
        Return Value:   None
*/
CPU_VOID BfrQPendRead(BfrQ *bfrQ){
    OS_ERR osErr;  
  
    OSSemPend(&bfrQ->readBfrs, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr); //No timeout for Rx, so keyboard entry is possible
    assert(osErr == OS_ERR_NONE);
}

/*-------------------- B f r Q P e n d W r i t e ( ) -------------------------------------
	Purpose:	Block if the buffer queue write buffer is not ready.
        Parameters:     buffer queue address
        Return Value:   None
*/
CPU_VOID BfrQPendWrite(BfrQ *bfrQ){
    OS_ERR osErr;  
  
    OSSemPend(&bfrQ->writeBfrs, SuspendTimeout, OS_OPT_PEND_BLOCKING, NULL, &osErr);
    assert(osErr == OS_ERR_NONE);
}
/*-------------------- B f r Q P o s t W r i t e( ) -------------------------------------
	Purpose:	Post that there is another write buffer available.
                        Finally, increment the current write buffer number.
        Parameters:     buffer queue address
        Return Value:   None
*/
CPU_VOID BfrQPostWrite(BfrQ *bfrQ){
  OS_ERR osErr;  //Semaphore Error Code.
  
  OSSemPost(&bfrQ->writeBfrs, OS_OPT_POST_1, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  BfrQReadReset(bfrQ);
  bfrQ->readBfrNum = (bfrQ->readBfrNum + 1) % bfrQ->numBfrs;
}

/*-------------------- B f r Q P o s t R e a d( ) -------------------------------------
	Purpose:	Post that there is another read buffer available.
                        Finally, increment the current read buffer number.
        Parameters:     buffer queue address
        Return Value:   None
*/
CPU_VOID BfrQPostRead(BfrQ *bfrQ){
  OS_ERR osErr;  //Semaphore Error Code.
  
  OSSemPost(&bfrQ->readBfrs, OS_OPT_POST_1, &osErr);
  assert(osErr == OS_ERR_NONE);
  
  bfrQ->writeBfrNum = (bfrQ->writeBfrNum + 1) % bfrQ->numBfrs;
}

/*-------------------- B f r Q N e x t B y t e( ) -------------------------------------
	Purpose:	Obtain but do not remove the next byte from the current read buffer, or -1
                        if the buffer is empty.
        Parameters:     buffer queue address
        Return Value:   Success - Return the next byte
                        Failure - Return -1 if the buffer was empty
*/
CPU_INT16S BfrQNextByte(BfrQ *bfrQ){
  return BfrNextByte(BfrQReadBfrAddr(bfrQ));
}
/*-------------------- B f r Q R e a d R e s e t( ) -------------------------------------
	Purpose:	Reset the current read buffer
        Parameters:     buffer queue address
        Return Value:   None
*/
CPU_VOID BfrQReadReset(BfrQ *bfrQ){
  BfrReset(BfrQReadBfrAddr(bfrQ));
}

/*-------------------- B f r Q W r i t e R e s e t( ) -------------------------------------
	Purpose:	Reset the current write buffer
        Parameters:     buffer queue address
        Return Value:   None
*/
CPU_VOID BfrQWriteReset(BfrQ *bfrQ){
  BfrReset(BfrQWriteBfrAddr(bfrQ));
}

/*-------------------- * B f r Q W r i t e B f r A d d r( ) -------------------------------------
	Purpose:	Obtain the address of the buffer space for the current write buffer.
        Parameters:     buffer queue address
        Return Value:   Address of the write buffer space
*/
CPU_VOID *BfrQWriteBfrAddr(BfrQ *bfrQ){
  return &bfrQ->buffers[bfrQ->writeBfrNum];
}

/*-------------------- * B f r Q R e a d B f r A d d r( ) -------------------------------------
	Purpose:	Obtain the address of the buffer space for the current read buffer.
        Parameters:     buffer queue address
        Return Value:   Address of the read buffer space
*/
CPU_VOID *BfrQReadBfrAddr(BfrQ *bfrQ){
  return &bfrQ->buffers[bfrQ->readBfrNum];
}

/*-------------------- B f r Q A d d B y t e( ) -------------------------------------
	Purpose:	Add a byte to the current write buffer. If successful, return the
                        same byte as added; otherwise return -1.
        Parameters:     buffer queue address, the byte being added
        Return Value:   Success - Return the added byte
                        Failure - Return -1 if the buffer was full
*/
CPU_INT16S BfrQAddByte(BfrQ *bfrQ, CPU_INT16S theByte){
    return BfrAddByte(BfrQWriteBfrAddr(bfrQ), theByte);
}

/*-------------------- B f r Q R e m B y t e( ) -------------------------------------
	Purpose:	Remove the next byte from the current read buffer.
        Parameters:     buffer queue address
        Return Value:   Success - Return the next byte
                        Failure - Return -1 if the buffer was empty
*/
CPU_INT16S BfrQRemByte(BfrQ *bfrQ){
    return BfrRemByte(BfrQReadBfrAddr(bfrQ));
}

/*-------------------- B f r Q W r i t e( ) -------------------------------------
	Purpose:	Write a block of bytes into the buffer space of the current write buffer.
        Parameters:     buffer queue address, address of block to write, number of bytes to write
        Return Value:   None
*/
CPU_VOID BfrQWrite( BfrQ *bfrQ, CPU_VOID *rec, CPU_INT08U size){
    CPU_INT08U i;
    CPU_INT08S *byte = (CPU_INT08S*)rec;
    
    for(i = 0; i < size; i++){
        BfrQAddByte(bfrQ, *(byte+i));
    }
}

/*-------------------- B f r Q R e a d( ) -------------------------------------
	Purpose:	Read a block of bytes from the buffer space of the current read buffer.
        Parameters:     buffer queue address, address of returned block, number of bytes to read
        Return Value:   None
*/
CPU_VOID BfrQRead( BfrQ *bfrQ, CPU_VOID *rec, CPU_INT08U size){
    CPU_INT08U i;
    CPU_INT08S *byte = (CPU_INT08S*)rec;
    
    for(i = 0; i < size; i++){
        *(byte+i) = BfrQRemByte(bfrQ);
    }
}