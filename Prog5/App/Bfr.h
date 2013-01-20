/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 5   -   Jesse Whitworth
-----------------------------------------------------------------------
			         Bfr.h
-----------------------------------------------------------------------
From Program 3, modified as needed. You must remove the member “closed” from
type CircBfr, and also remove the 3 functions “BfrClosed(),” “BfrClose(),” and
“BfrOpen().” You may add new functions to supply any needed functionality
*/
#ifndef BFR_H
#define BFR_H

#include "includes.h"

#pragma pack(1)
typedef struct
{
    CPU_INT16U size; /* -- Size of buffer space in bytes */
    volatile CPU_INT16U out; /* -- Position of next removed byte */
    volatile CPU_INT16U in; /* -- Position for next added byte */
    volatile CPU_INT16U numBytes; /* -- Current number of bytes in the buffer */
    CPU_INT08U *bfr; /* -- Pointer to CircBfr space */
} CircBfr;
#pragma pack()

CPU_VOID BfrInit(CircBfr *bfr, CPU_INT08U *bfrSpace, CPU_INT16U bfrSize);    
CPU_VOID BfrReset(CircBfr *bfr);
CPU_BOOLEAN BfrFull(CircBfr *bfr);
CPU_BOOLEAN BfrEmpty(CircBfr *bfr);
CPU_INT16S BfrAddByte(CircBfr * bfr, CPU_INT16S theByte);
CPU_INT16S BfrRemByte(CircBfr *bfr);
CPU_INT16S BfrNextByte(CircBfr *bfr);
CPU_VOID BfrWrite(CircBfr *bfr, CPU_VOID *rec, CPU_INT08U size);
CPU_VOID BfrRead(CircBfr *bfr, CPU_VOID *rec, CPU_INT08U size);

#endif