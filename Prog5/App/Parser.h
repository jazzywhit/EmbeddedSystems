/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 5   -   Jesse Whitworth
-----------------------------------------------------------------------
			       Parser.h
-----------------------------------------------------------------------
The packet parser task module – same as Program 3
The packet parser task module. Parser() must be a uC/OS-III style task.
*/

#ifndef PKTPARSER_H
#define PKTPARSER_H

#include "includes.h"
#include "BfrQ.h"

//The Error State. Needed by both Parser and Payload.
typedef enum {E1 = 1, E2, E3, E4, E5} ErrorState;

typedef struct
{
    CPU_INT08S payloadLen;	  // Total number of data bytes
    CPU_INT08U data[1];	          // Remaining data bytes
} PktBfr;

CPU_VOID CreateParserTask(CPU_VOID *payloadBfrQ);
CPU_VOID ParserTask(CPU_VOID *data);
CPU_BOOLEAN ParseByte(CPU_VOID *payloadBfr, CPU_INT08U nextByte);
CPU_VOID LoadPayloadBfrQ(BfrQ *payloadBfrQ, CPU_VOID *payloadBfr);

#endif
