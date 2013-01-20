/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 1   -   Jesse Whitworth
-----------------------------------------------------------------------
			              pktParser.h
-----------------------------------------------------------------------*/

#ifndef PKTPARSER_H
#define PKTPARSER_H

#include "stdio.h"
#include "CPU.h"

CPU_BOOLEAN ParsePkt(FILE *pktFile, void *pktBfr);

#endif