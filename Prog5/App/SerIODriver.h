/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 5   -   Jesse Whitworth
-----------------------------------------------------------------------
			     SerIODriver.h
-----------------------------------------------------------------------
The RS232 driver module, which defines the circular buffers iBfr and oBfr, and I/O
driver functions, including the functions InitSerIO(), GetByte(), PutByte(), and the two
tasks ServiceTx() and ServiceRx() plus ISR(). Also defined in this module are the
two semaphores “bytesAvail” and “spacesAvail.”
*/

#ifndef SERIODRIVER_H
#define SERIODRIVER_H

#include "includes.h"

#ifndef BfrSize
#define BfrSize 4
#endif

CPU_VOID InitIODriver(CPU_VOID);
CPU_INT16S PutByte(CPU_INT16S txChar);
CPU_INT16S GetByte(CPU_VOID);
CPU_VOID ServiceTx(CPU_VOID);
CPU_VOID ServiceRx(CPU_VOID);
CPU_VOID Ser_ISR(CPU_VOID);

#endif