/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 5   -   Jesse Whitworth
-----------------------------------------------------------------------
			        BfrQ.h
-----------------------------------------------------------------------
From Program 3, modified as needed. The two semaphores “readBfrs” and “writeBfrs”
are defined in this module as members of type BfrQ. You may add new functions to
supply any needed functionality.
*/

#ifndef BFRQ_H
#define BFRQ_H

#include "includes.h"
#include "Bfr.h"

#ifndef BfrQSize
#define BfrQSize 80  //Size of the buffers in the BufferQ
#endif

#ifndef NumBfrs
#define NumBfrs 3
#endif

#pragma pack() //Ensure this is not packed.
typedef struct
{
    OS_SEM readBfrs; /* A producer task posts to this semaphore to signal a consumer task
                        that it is finished with a write buffer and a new read buffer is available.
                        A consumer task pends on this semaphore, awaiting an available read buffer.
                     */
    OS_SEM writeBfrs; /*A consumer task posts to this semaphore to signal a producer task
                        that it is finished with a read buffer and a new write buffer is available.
                        A producer task pends on this semaphore, awaiting an available write buffer.
                      */
    CPU_INT08U numBfrs; /* -- Number of buffers in the queue */
    CPU_INT08U bfrSize; /* -- Buffer capacity in bytes */
    CPU_INT08U readBfrNum; /* -- The index of the read buffer */
    CPU_INT08U writeBfrNum; /* -- The index of the write buffer */
    CircBfr buffers[NumBfrs]; /* -- The buffers */ //These should not be treated as CircBfrs
} BfrQ;

CPU_VOID BfrQInit( BfrQ *bfrQ, CPU_INT08U numBfrs, CPU_INT08U bfrSize, CPU_INT08U *bfrSpace);
CPU_VOID BfrQReadReset(BfrQ *bfrQ);
CPU_VOID *BfrQWriteBfrAddr(BfrQ *bfrQ);
CPU_VOID *BfrQReadBfrAddr(BfrQ *bfrQ);
CPU_INT16S BfrQAddByte(BfrQ *bfrQ, CPU_INT16S theByte);
CPU_INT16S BfrQRemByte(BfrQ *bfrQ);
CPU_VOID BfrQWrite( BfrQ *bfrQ, CPU_VOID *rec, CPU_INT08U size);
CPU_VOID BfrQRead( BfrQ *bfrQ, CPU_VOID *rec, CPU_INT08U size);
CPU_INT16S BfrQNextByte(BfrQ *bfrQ);

CPU_VOID BfrQPendRead(BfrQ *bfrQ);
CPU_VOID BfrQPendWrite(BfrQ *bfrQ);
CPU_VOID BfrQPostRead(BfrQ *bfrQ);
CPU_VOID BfrQPostWrite(BfrQ *bfrQ);

#endif