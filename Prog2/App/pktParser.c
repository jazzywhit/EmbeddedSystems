/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 2   -   Jesse Whitworth
-----------------------------------------------------------------------
			      pktParser.c
-----------------------------------------------------------------------*/
#include "pktParser.h"
#include "Error.h"
#include "includes.h"


#define PacketHeaderDiff 5  //Number of bytes of header before the payload starts.

typedef struct
{
	CPU_INT08U payloadLen;	// Total number of data bytes
	CPU_INT08U data[1];		// Remaining data bytes

} PktBfr;

//Preamble bytes as defined in guidelines.
#define P1Char 0x03
#define P2Char 0xAF
#define P3Char 0xEF

//Set the error states to a numerical value through enumeration.
typedef enum {P1, P2, P3, C, K, D, ER } ParserState;

/*-------------------- Function Prototypes -------------------------------------*/
void Error(CPU_CHAR *message, ParserState *parseState);


/*-------------------- E r r o r ( ) -------------------------------------
	Purpose:	Set parser state to ER and call ShowError(). 
*/
void Error(CPU_CHAR *message, ParserState *parseState){

	*parseState = ER;
	ShowError(message);

}

/*-------------------- P a r s e P k t ( ) -------------------------------------
	Purpose:	Obtain a single packet from the byte stream and put the bytes into the payload.
			Verify that the packet has 3 preamble bytes and verify the checksum.
*/
void ParsePkt(void *payloadBfr){

	ParserState parseState = P1;
	CPU_INT08U  nextByte;
	CPU_INT08U  checkSum = 0;
	CPU_INT08S  i = 0;

	PktBfr *pktBfr = (PktBfr *)payloadBfr;

	while(i > -1){

		nextByte = GetByte();
		checkSum ^= nextByte;

		switch(parseState){
		case P1: 
			if (nextByte == P1Char) parseState = P2;
			else Error("Bad Preamble Byte 1.", &parseState);
			break;
		case P2:
			if (nextByte == P2Char) parseState = P3;
			else Error("Bad Preamble Byte 2.", &parseState);
			break;
		case P3:
			if (nextByte == P3Char) parseState = C;
			else Error("Bad Preamble Byte 3.", &parseState);
			break;
		case C:
			parseState = K;
			break;
		case K:
			if (nextByte-PacketHeaderDiff < 1){
				Error("Bad Packet Size", &parseState);
				break;
			}
			pktBfr->payloadLen = nextByte;
			parseState = D;
			i = 0;
			break;
		case D:
			pktBfr->data[i++] = nextByte;
			if (i >= pktBfr->payloadLen-PacketHeaderDiff){ //Finished collecting data
				parseState = P1;

				if (checkSum != 0){
					Error("Checksum error", &parseState);
					break;
				}
				i = -1;
			}
			break;
		case ER:
			if (nextByte == P1Char) parseState = P2;
			checkSum = P1Char;
			break;
		}
	}
}
