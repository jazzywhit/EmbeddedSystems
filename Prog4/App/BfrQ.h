/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 4   -   Jesse Whitworth
-----------------------------------------------------------------------
			        BfrQ.h
-----------------------------------------------------------------------
Purpose: Creates a queue of buffers so that producers and consumers have a place
to store and retrieve buffers
Functions are defined in program 3 guidelines.
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

typedef struct
{
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
CPU_VOID BfrQCloseWrite(BfrQ *bfrQ);
CPU_VOID BfrQOpenRead(BfrQ *bfrQ);
CPU_BOOLEAN BfrQWriteClosed(BfrQ *bfrQ);
CPU_BOOLEAN BfrQReadClosed(BfrQ *bfrQ);
CPU_INT16S BfrQAddByte(BfrQ *bfrQ, CPU_INT16S theByte);
CPU_INT16S BfrQNextByte(BfrQ *bfrQ);
CPU_INT16S BfrQRemByte(BfrQ *bfrQ);
CPU_VOID BfrQWrite( BfrQ *bfrQ, CPU_VOID *rec, CPU_INT08U size);
CPU_VOID BfrQRead( BfrQ *bfrQ, CPU_VOID *rec, CPU_INT08U size);

#endif