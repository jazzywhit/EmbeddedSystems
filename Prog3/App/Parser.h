/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 3   -   Jesse Whitworth
-----------------------------------------------------------------------
			       Parser.h
-----------------------------------------------------------------------*/

#ifndef PKTPARSER_H
#define PKTPARSER_H

#include "includes.h"
#include "BfrQ.h"

typedef enum {E1 = 1, E2, E3, E4, E5} ErrorState;

typedef struct
{
    CPU_INT08S payloadLen;	    // Total number of data bytes
    CPU_INT08U data[1];	            // Remaining data bytes
} PktBfr;

void Parser(BfrQ *payloadBfrQ);
CPU_BOOLEAN ParseByte(CPU_VOID *payloadBfr, CPU_INT08U nextByte);
void LoadPayloadBfrQ(BfrQ *payloadBfrQ, CPU_VOID *payloadBfr);

#endif
