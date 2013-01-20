/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 1   -   Jesse Whitworth
-----------------------------------------------------------------------
			              pktReader.h
-----------------------------------------------------------------------*/
#ifndef PKTREADER_H
#define PKETREADER_H

#include "CPU.h"

FILE* OpenPktFile();
CPU_INT08U	GetByte(FILE *pktFile);

#endif