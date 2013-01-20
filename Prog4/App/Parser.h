/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 4   -   Jesse Whitworth
-----------------------------------------------------------------------
			       Parser.h
-----------------------------------------------------------------------
The packet parser task module – same as Program 3
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

void Parser(BfrQ *payloadBfrQ);
CPU_BOOLEAN ParseByte(void *payloadBfr, CPU_INT08U nextByte);
void LoadPayloadBfrQ(BfrQ *payloadBfrQ, void *payloadBfr);

#endif
