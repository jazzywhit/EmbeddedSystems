/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 4   -   Jesse Whitworth
-----------------------------------------------------------------------
			         Bfr.h
-----------------------------------------------------------------------
Same as program 3, but with several volatile members in
type Bfr
*/
#ifndef BFR_H
#define BFR_H

#include "includes.h"

typedef struct
{
    volatile CPU_BOOLEAN closed; /* -- TRUE to indicate ready to be read */
    CPU_INT16U size; /* -- Size of buffer space in bytes */
    volatile CPU_INT16U out; /* -- Position of next removed byte */
    volatile CPU_INT16U in; /* -- Position for next added byte */
    volatile CPU_INT16U numBytes; /* -- Current number of bytes in the buffer */
    CPU_INT08U *bfr; /* -- Pointer to CircBfr space */
} CircBfr;

CPU_VOID BfrInit(CircBfr *bfr, CPU_INT08U *bfrSpace, CPU_INT16U bfrSize);    
CPU_VOID BfrReset(CircBfr *bfr);
CPU_BOOLEAN BfrFull(CircBfr *bfr);
CPU_BOOLEAN BfrEmpty(CircBfr *bfr);
CPU_INT16S BfrAddByte(CircBfr * bfr, CPU_INT16S theByte);
CPU_INT16S BfrRemByte(CircBfr *bfr);
CPU_INT16S BfrNextByte(CircBfr *bfr);
CPU_BOOLEAN BfrClosed(CircBfr *bfr);
CPU_VOID BfrClose(CircBfr *bfr);
CPU_VOID BfrOpen(CircBfr *bfr);
CPU_VOID BfrWrite(CircBfr *bfr, CPU_VOID *rec, CPU_INT08U size);
CPU_VOID BfrRead(CircBfr *bfr, CPU_VOID *rec, CPU_INT08U size);

#endif