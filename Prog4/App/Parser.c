/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 4   -   Jesse Whitworth
-----------------------------------------------------------------------
			       Parser.c
-----------------------------------------------------------------------
The packet parser task module – same as Program 3
*/
#include "Parser.h"
#include "Reply.h"
#include "SerIODriver.h"
#include "Bfr.h"
#include "Payload.h"

#define PacketHeaderDiff 5  //Number of bytes of header before the payload starts.

//Preamble bytes as defined in guidelines.
#define P1Char 0x03
#define P2Char 0xAF
#define P3Char 0xEF

//Set the parser states to a numerical value through enumeration.
typedef enum {P1, P2, P3, C, K, D, ER } ParserState;

/*-------------------- Local Function Prototypes -----------------------------*/
CPU_VOID Error(PktBfr *pktBfr, ParserState *parserState, ErrorState errState);

/*-------------------- E r r o r ( ) -------------------------------------
	Purpose:	Set parser state to ER and call Reply(). 
Update: To accommodate packet error reporting, the Parser task will output a special 
Error Message Payload containing the error code to be reported. The Payload task will 
then interpret this error code and forward the appropriate error message string to 
the Reply task. An Error Message Payload is identified by having a negative packet 
length and no data bytes. The new packet structure is shown below
*/
CPU_VOID Error(PktBfr *pktBfr, ParserState *parserState, ErrorState errState){
    *parserState = ER;
    pktBfr->payloadLen = (0 - errState); //Make the error state negative
}


/*-------------------- L o a d P a y l o a d B f r Q ( ) -----------------------
    Packet Parser Task: Fill the writeBfr of the PayloadBfrQ with the bytes from the pktBfr
typedef struct
*/
CPU_VOID LoadPayloadBfrQ(BfrQ *payloadBfrQ, void *payloadBfr){
     PktBfr *pktBfr = (PktBfr *)payloadBfr;
     CPU_VOID *rec = pktBfr->data;
     
     BfrQAddByte(payloadBfrQ, pktBfr->payloadLen);
     BfrQWrite(payloadBfrQ, rec, pktBfr->payloadLen - PacketHeaderDiff);
     BfrQCloseWrite(payloadBfrQ);
}

/*-------------------- P a r s e r ( ) -------------------------------------
    Packet Parser Task: Read a packet from iBfr and extract a payload to the
                        payload buffer queue write buffer.
    There must be a valid byte available and the PayloadBfrQ write buffer must not be closed.
*/
void Parser(BfrQ *payloadBfrQ){
    static Payload parserPayload;
    CPU_INT16S nextByte = GetByte();
    
    if((nextByte >= 0)){
        if(!BfrQWriteClosed(payloadBfrQ)){
            if(ParseByte(&parserPayload, nextByte)){
                LoadPayloadBfrQ(payloadBfrQ, &parserPayload);
            }
        }
    }
}

/*-------------------- P a r s e B y t e ( ) -------------------------------------
    Packet Parser Task: Parse a single byte and progress through the states
*/
CPU_BOOLEAN ParseByte(CPU_VOID *payloadBfr, CPU_INT08U nextByte){
    
    static ParserState parseState = P1;
    static CPU_INT08U checkSum = 0;
    static CPU_INT08U  i = 0;
    
    PktBfr *pktBfr = (PktBfr *)payloadBfr;

    checkSum ^= nextByte;

    switch(parseState){
        case P1: 
            if (nextByte == P1Char) parseState = P2;
            else{
                Error(pktBfr, &parseState, E1);
                return TRUE;
            }
            break;
        case P2:
            if (nextByte == P2Char) parseState = P3;
            else{
                Error(pktBfr, &parseState, E2);
                return TRUE;
            }
            break;
        case P3:
            if (nextByte == P3Char) parseState = C;
            else{
                Error(pktBfr, &parseState, E3);
                return TRUE;
            }
            break;
        case C:
            parseState = K;
            break;
        case K:
            if (nextByte - PacketHeaderDiff < 1){
                Error(pktBfr, &parseState, E5);
                return TRUE;
            }
            pktBfr->payloadLen = nextByte;
            parseState = D;
            i = 0;
            break;
        case D: //Go through each data part after the header
            pktBfr->data[i++] = nextByte;
            if (i >= pktBfr->payloadLen - PacketHeaderDiff){
                parseState = P1;
                
                //Checksum Error if all packets XOR'd != 0
                if (checkSum != 0){ 
                    Error(pktBfr, &parseState, E4);
                }
                return TRUE; //Payload is finished
            }
            break;
        case ER:
            if (nextByte == P1Char) parseState = P2;
                checkSum = P1Char;
            break;
        }
    return FALSE; //Payload is not finished
}
