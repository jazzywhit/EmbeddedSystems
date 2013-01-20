/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 1   -   Jesse Whitworth
-----------------------------------------------------------------------
			              pktReader.c
-----------------------------------------------------------------------*/

#include "CPU.h"
#include "stdio.h"
#include "string.h"
#include "pktParser.h"
#include "Error.h"

#define BUFFER_SIZE   60

/*-------------------- O p e n P k t F i l e ( ) -------------------------------------
	Purpose:	Promt the user for a file, and then open the file for reading.
	Return:		FILE - Returns the file that was opened.
				NULL  - Returns NULL if the file could not be opened.*/
FILE* OpenPktFile(){

	CPU_CHAR buffer[BUFFER_SIZE];
	printf("Packet File Name? ");

	if(fgets(buffer, sizeof(buffer), stdin) == NULL) return NULL;
	if (!sscanf(buffer,"%s", buffer)) ShowError("File name too long");
	else{
		if(buffer[0] == '\n') return NULL;
		else{
			FILE *pktFile = fopen(buffer, "rb");
			if (pktFile == NULL){
				ShowError("File not found.");
			}
			return pktFile;
		}
	}

	return NULL;
}

/*-------------------- G e t B y t e ( ) -------------------------------------
	Purpose:	Read the next byte from the file.
	Return:		False - The user did not enter a file.*/
CPU_INT08U	GetByte(FILE *pktFile){
	
	return fgetc(pktFile);

}