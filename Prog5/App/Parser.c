/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 5   -   Jesse Whitworth
-----------------------------------------------------------------------
			       Parser.c
-----------------------------------------------------------------------
The packet parser task module – same as Program 3
The packet parser task module. Parser() must be a uC/OS-III style task.
*/
#include "Assert.h"
#include "Parser.h"
#include "Reply.h"
#include "SerIODriver.h"
#include "Bfr.h"
#include "Payload.h"

//Set the parser states to a numerical value through enumeration.
typedef enum {P1, P2, P3, C, K, D, ER } ParserState;

//----- c o n s t a n t    d e f i n i t  i o n s -----
#define SuspendTimeout 100   // Timeout for semaphore wait
#define PARSER_STK_SIZE 128  // Parser Task stack size
#define ParserPrio 3         // Parser Task Priority

//Preamble bytes as defined in guidelines.
#define P1Char 0x03
#define P2Char 0xAF
#define P3Char 0xEF

//Number of bytes of header before the payload starts.
#define PacketHeaderDiff 5  

//----- g l o b a l    v a r i a b l e s -----
static  OS_TCB   parserTCB;                  // Reply Task TCB
static  CPU_STK  parserStk[PARSER_STK_SIZE];  // Space for Reply Task stack

/*-------------------- Local Function Prototypes -----------------------------*/
CPU_VOID Error(PktBfr *pktBfr, ParserState *parserState, ErrorState errState);

/*--------------- C r e a t e P a r s e r T a s k( ) ---------------
PURPOSE
Create the Parser Task.

INPUT PARAMETERS
payloadBfrQ - The address of the reply buffer queue.
*/
CPU_VOID CreateParserTask(CPU_VOID *payloadBfrQ){
    OS_ERR  osErr;     /* O/S error code */                       
    
    /* Create the Reply Task. */
    OSTaskCreate(  &parserTCB,         // Task Control Block
                 "Parser Task",        // Task name
                 ParserTask,               // Task entry point
                 payloadBfrQ,          // Address of payload buffer queue
                 ParserPrio,           // Task priority
                 &parserStk[0],        // Base address of task stack space
                 PARSER_STK_SIZE / 10, // Stack water mark limit
                 PARSER_STK_SIZE,      // Task stack size
                 0,                   // This task has no task queue
                 0,                   // Number of clock ticks (defaults to 10)
                 (CPU_VOID      *)0,  // Pointer to TCB extension
                 OS_OPT_TASK_NONE,    // Task options
                 &osErr);             // Address to return O/S error code
    
    /* Verify successful task creation. */
    assert(osErr == OS_ERR_NONE);
}

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
 
}

/*-------------------- P a r s e r T a s k ( ) -------------------------------------
    Packet Parser Task: Read a packet from iBfr and extract a payload to the
                        payload buffer queue write buffer.
    There must be a valid byte available and the PayloadBfrQ write buffer must not be closed.
    
    This is a producer Task. BfrQPostRead and BfrQPendWrite
*/
CPU_VOID ParserTask(CPU_VOID *data){
  
    BfrQ *payloadBfrQ = (BfrQ *) data;
    static Payload parserPayload;
    
    for(;;){
      
        BfrQPendWrite(payloadBfrQ); //Pend on Write buffer - Start Producing
        
        for (;;){    
            CPU_INT16S nextByte = GetByte();  //Pend on bytesAvail 
            
            if((nextByte >= 0)){
                if(ParseByte(&parserPayload, nextByte)){
                    LoadPayloadBfrQ(payloadBfrQ, &parserPayload);
                    BfrQPostRead(payloadBfrQ); // Post to Read buffer - Done producing
                    break;
                }
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
