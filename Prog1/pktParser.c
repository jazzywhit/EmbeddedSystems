/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 1   -   Jesse Whitworth
-----------------------------------------------------------------------
			              pktParser.c
-----------------------------------------------------------------------*/
#include "pktParser.h"
#include "pktReader.h"
#include "Error.h"

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
typedef enum {P1, P2, P3, C, K, R, S, T, D, ER } ParserState;

/*-------------------- E r r o r ( ) -------------------------------------
	Purpose:	Set parser state to ER and call ShowError(). 
*/
void Error(CPU_CHAR *message, ParserState *parseState){

	*parseState = ER;
	ShowError(message);

}

/*-------------------- P a r s e P k t ( ) -------------------------------------
	Purpose:	Read one packet from the indicated binary file, extract and return the payload.
				In the event the packet errors, display the error message.
	Return:		True - A packet was obtained and its payload was extracted; pktBfr points to packet.
				False - End of file was reached; there are no more packets.
*/
CPU_BOOLEAN ParsePkt(FILE *pktFile, void *payloadBfr){

	ParserState parseState = P1;
	CPU_INT08U  nextByte;
	CPU_INT08U  checkSum = 0;
	CPU_INT08U	i = 0;

	PktBfr *pktBfr = (PktBfr *)payloadBfr;

	for (;;){

		if((nextByte = (GetByte(pktFile))) == EOF) break;
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
				return true;
			}
			break;
		case ER:
			if (nextByte == P1Char) parseState = P2;
			checkSum = P1Char;
			break;
		}
	}
	return false;
}
