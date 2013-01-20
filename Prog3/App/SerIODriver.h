/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 3   -   Jesse Whitworth
-----------------------------------------------------------------------
			     SerIODriver.h
-----------------------------------------------------------------------*/

#ifndef SERIODRIVER_H
#define SERIODRIVER_H

#include "includes.h"

CPU_VOID InitSerIO(CPU_VOID);
CPU_INT16S PutByte(CPU_INT16S txChar);
CPU_INT16S GetByte(CPU_VOID);
CPU_INT16S NextByte(CPU_VOID);
CPU_VOID ServiceTx(CPU_VOID);
CPU_VOID ServiceRx(CPU_VOID);

#endif