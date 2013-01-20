/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 3   -   Jesse Whitworth
-----------------------------------------------------------------------
			        BfrQ.c
-----------------------------------------------------------------------*/

#include <assert.h>
#include <string.h>

#include "BfrQ.h"

/*-------------------- B f r Q I n i t ( ) -------------------------------------
	Purpose:	Initialize a buffer queue. Initialize the “numBfrs” and “bfrSize” members. Set
                        readBfrNum and writeBfrNum to zero. Finally, perform BfrInit() on all of the buffers in the queue.
        Parameters:     buffer queue address, number of buffers, size of each buffer (bytes), address of a block of memory large enough
        Return Value:   None
*/
CPU_VOID BfrQInit( BfrQ *bfrQ, CPU_INT08U numBfrs, CPU_INT08U bfrSize, CPU_INT08U *bfrSpace){
    CPU_INT08S i;
    
    bfrQ->numBfrs = numBfrs;
    bfrQ->bfrSize = bfrSize;
    bfrQ->readBfrNum = 0;
    bfrQ->writeBfrNum = 0;
    
    for (i = 0; i < numBfrs; i++){
        BfrInit(&bfrQ->buffers[i], bfrSpace+(i*bfrSize)/*+1*/, bfrSize); 
    }
}

/*-------------------- B f r Q W r i t e B y t e s R e m a i n( ) ----------------------
	Purpose:	Calculates the bytes left in the BfrQ that can be written to
        Parameters:     buffer queue address
        Return Value:   Maximum number of bytes that can be written
*/
CPU_INT16U BfrQWriteBytesRemain(BfrQ *bfrQ){
    CPU_INT16U bytesLeft = 0;
    CPU_INT08U i;
    
    for(i = 0; i < bfrQ->numBfrs; i++){
        if(!BfrClosed(&bfrQ->buffers[i]))
            bytesLeft += (bfrQ->bfrSize - bfrQ->buffers[i].numBytes);
    }
    return bytesLeft;
}

/*-------------------- B f r Q R e a d R e s e t( ) -------------------------------------
	Purpose:	Reset the current read buffer
        Parameters:     buffer queue address
        Return Value:   None
*/
CPU_VOID BfrQReadReset(BfrQ *bfrQ){
  BfrReset(BfrQReadBfrAddr(bfrQ));
}

/*-------------------- B f r Q W r i t e B f r A d d r( ) -------------------------------------
	Purpose:	Obtain the address of the buffer space for the current write buffer.
        Parameters:     buffer queue address
        Return Value:   Address of the write buffer space
*/
CPU_VOID *BfrQWriteBfrAddr(BfrQ *bfrQ){
  return &bfrQ->buffers[bfrQ->writeBfrNum];
}

/*-------------------- *B f r Q R e a d B f r A d d r( ) -------------------------------------
	Purpose:	Obtain the address of the buffer space for the current read buffer.
        Parameters:     buffer queue address
        Return Value:   Address of the read buffer space
*/
CPU_VOID *BfrQReadBfrAddr(BfrQ *bfrQ){
  return &bfrQ->buffers[bfrQ->readBfrNum];
}

/*-------------------- B f r Q C l o s e W r i t e( ) -------------------------------------
	Purpose:	Mark the current write buffer closed and increment the current write buffer number.
        Parameters:     buffer queue address
        Return Value:   None
*/
CPU_VOID BfrQCloseWrite(BfrQ *bfrQ){
  BfrClose(BfrQWriteBfrAddr(bfrQ));
  bfrQ->writeBfrNum = (bfrQ->writeBfrNum + 1) % bfrQ->numBfrs;
}

/*-------------------- B f r Q O p e n R e a d( ) -------------------------------------
	Purpose:	Mark the current read buffer open. Reset the current read buffer.
                        Finally, increment the current read buffer number
        Parameters:     buffer queue address
        Return Value:   None
*/
CPU_VOID BfrQOpenRead(BfrQ *bfrQ){
  BfrOpen(BfrQReadBfrAddr(bfrQ));
  BfrQReadReset(bfrQ);
  bfrQ->readBfrNum = (bfrQ->readBfrNum + 1) % bfrQ->numBfrs;
}

/*-------------------- B f r Q W r i t e C l o s e d( ) -------------------------------------
	Purpose:	Test whether the current write buffer is closed.
        Parameters:     buffer queue address
        Return Value:   Success - TRUE if the current write butter is closed
                        Failure - FALSE otherwise
*/
CPU_BOOLEAN BfrQWriteClosed(BfrQ *bfrQ){
  return BfrClosed(BfrQWriteBfrAddr(bfrQ));
}

/*-------------------- B f r Q R e a d C l o s e d( ) ------------------------------
	Purpose:	Test whether the current read buffer is closed.
        Parameters:     buffer queue address
        Return Value:   Success - TRUE if the current read buffer is closed
                        Failure - FALSE otherwise
*/
CPU_BOOLEAN BfrQReadClosed(BfrQ *bfrQ){
  return BfrClosed(BfrQReadBfrAddr(bfrQ));
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