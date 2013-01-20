/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 5   -   Jesse Whitworth
-----------------------------------------------------------------------
			        Payload.c
-----------------------------------------------------------------------
Create the static buffer queues that will be used to handle the payloads

The payload task module – PayloadTask() must be a uC/OS-III style task
*/

#include <string.h>
#include "Assert.h"
#include "Payload.h"
#include "Reply.h"
#include "Parser.h"
#include "Bfr.h"

//----- c o n s t a n t    d e f i n i t  i o n s -----
#define SuspendTimeout 100    // Timeout for semaphore wait
#define PAYLOAD_STK_SIZE 128  // Payload Task stack size
#define PayloadPrio 4         // Payload Task Priority

#define PayloadHeaderDiff 8  //Amount of header before the data starts in the payload.
#define PacketHeaderDiff 5   //Amount of header before the payload starts in the packet.

//Define the message types with real names
#define BarPacket 'B'
#define DatePacket 'D'
#define HumPacket 'H'
#define NodePacket 'I'
#define PrecPacket 'P'
#define SolarPacket 'R'
#define TempPacket 'T'
#define WindPacket 'W'
#define StationAddr 1

//----- g l o b a l    v a r i a b l e s -----
static  OS_TCB   payloadTCB;                  // Reply Task TCB
static  CPU_STK  payloadStk[PAYLOAD_STK_SIZE];  // Space for Reply Task stack

//Allocate the payloadBfrQ
static BfrQ PayloadBfrQ;
static CPU_INT08U PayloadBfrSpace[NumBfrs * BfrQSize];

//Allocate the ReplyBfrQ
static BfrQ ReplyBfrQ;
static CPU_INT08U ReplyBfrSpace[NumBfrs * BfrQSize];

/*--------------- C r e a t e P a y l o a d T a s k( ) ---------------
PURPOSE
Create the Payload Task.

INPUT PARAMETERS
None
*/
CPU_VOID CreatePayloadTask(CPU_VOID){
    /* O/S error code */
    OS_ERR  osErr;                           
    
    /* Create the Reply Task. */
    OSTaskCreate(  &payloadTCB,         // Task Control Block
                 "Payload Task",        // Task name
                 PayloadTask,               // Task entry point
                 NULL,                  // Address of reply buffer queue
                 PayloadPrio,           // Task priority
                 &payloadStk[0],        // Base address of task stack space
                 PAYLOAD_STK_SIZE / 10, // Stack water mark limit
                 PAYLOAD_STK_SIZE,      // Task stack size
                 0,                   // This task has no task queue
                 0,                   // Number of clock ticks (defaults to 10)
                 (CPU_VOID      *)0,  // Pointer to TCB extension
                 OS_OPT_TASK_NONE,    // Task options
                 &osErr);             // Address to return O/S error code
    
    /* Verify successful task creation. */
    assert(osErr == OS_ERR_NONE);
}

/*-------------------- P a r s e W i n d ( ) -------------------------------------
	Purpose:	Parses the Packed Wind packet into it's component parts using bitwise arithmetic.
*/
CPU_VOID ParseWind(CPU_CHAR *message, CPU_INT08U *speed, CPU_INT16U *dir, CPU_INT08U srcAddr){
    #define Mask 0xF0
    #define Shift 4
    #define NumBase 10
    
    sprintf(message, "\nN%u SP = %u.%u  DIR = %u\n", 
                   srcAddr, \
                   (((speed[0] & Mask) >> Shift)*NumBase*NumBase) + ((speed[0] & ~Mask)* NumBase) + ((speed[1] & Mask) >> Shift), (speed[1] & ~Mask), \
                   *dir);
}

/*-------------------- P a r s e P r e c i p ( ) -------------------------------------
	Purpose:	Parses the Packed Precipitation packet into it's component parts using bitwise arithmetic.
*/
CPU_VOID ParsePrecip(CPU_CHAR *message, CPU_INT08U *depth, CPU_INT08U srcAddr){
    #define Mask 0xF0
    #define Shift 4
    #define NumBase 10
    
    sprintf(message, "\nN%u = %u.%u%u\n",
                   srcAddr, \
                   (((depth[0] & Mask) >> Shift)*NumBase) + (depth[0] & ~Mask), \
                     (depth[1] & Mask) >> Shift, (depth[1] & ~Mask));
}

/*-------------------- P a r s e D a t e ( ) -------------------------------------
	Purpose:	Parses the Packed Date/Time packet into it's component parts using bitwise arithmetic.
	Issue:		The date/time packet is packed in big endian.
			The bytes must be reversed before bitwise manipulation.
*/
CPU_VOID ParseDate(CPU_CHAR *message, CPU_INT32U *ts, CPU_INT08U srcAddr){
    CPU_INT32U rBytes = ReverseBytes32(ts);
    
    //Masks for the different packed components
    #define MYear 0xFFF00000
    #define MMonth 0x000F0000
    #define MDay 0xF800
    #define MHour 0x07C0
    #define MMinute 0x003F
    #define MonthOffset 16
    #define DayOffset 11
    #define YearOffset 20
    #define HourOffset 6
    
    sprintf(message, "\nN%u TS = %u/%u/%u %u:%u\n", srcAddr, \
                   (rBytes & MMonth) >> MonthOffset, (rBytes & MDay) >> DayOffset, (rBytes & MYear) >> YearOffset, \
                     (rBytes & MHour) >> HourOffset, (rBytes & MMinute));
}

/*-------------------- C o n s t r u c t M e s s a g e ( ) -------------------------------------
	Purpose:	Create a message that will be sent to the replyQ
        Parameters:     address of payload, character string
        Return:         TRUE -  A payload message was created
                        FALSE - A error message was created
*/
CPU_BOOLEAN ConstructMessage(Payload *payload, CPU_CHAR *message){
    
    //Error Payload
    if(payload->payloadLen < 0){
        ConstructError(payload, message);
        return FALSE;
    }
    
    //Info Message - Wrong Address
    if(payload->dstAddr != StationAddr){
        sprintf(message, "IBad ADR");
        return FALSE;
    }
   
    switch(payload->msgType){
        case BarPacket:
                sprintf(message, "\nN%u P = %u\n", payload->srcAddr, payload->dataPart.pres);
                break;
        case DatePacket:
                ParseDate(message, &payload->dataPart.dateTime, payload->srcAddr);
                break;
        case HumPacket:
                sprintf(message, "\nN%u DP = %u H = %u\n", 
                        payload->srcAddr, payload->dataPart.hum.dewPt, payload->dataPart.hum.hum);
                break;
        case NodePacket:
                payload->dataPart.id[(payload->payloadLen-PayloadHeaderDiff)] = '\0'; //Terminate the info string
                sprintf(message, "\nN%u ID = %s\n", payload->srcAddr, payload->dataPart.id);
                break;
        case PrecPacket:
                ParsePrecip(message, payload->dataPart.depth, payload->srcAddr);
                break;
        case SolarPacket:
                sprintf(message, "\nN%u R = %u\n", payload->srcAddr, payload->dataPart.rad);
                break;
        case TempPacket:
                sprintf(message, "\nN%u T = %i\n", payload->srcAddr, payload->dataPart.temp);
                break;
        case WindPacket:
                ParseWind(message, payload->dataPart.wind.speed, &payload->dataPart.wind.dir, payload->srcAddr);
                break;
        default:
                sprintf(message, "IBad Type"); //Info Message - Bad Type
                return FALSE;
    }
    return TRUE;
}

/*-------------------- P a y l o a d I n i t( ) -------------------------------------
	Purpose:	Initialize the Payload buffer
        Parameters:     buffer queue address
        Return Value:   None
*/
CPU_VOID PayloadInit(BfrQ **payloadBfrQ, BfrQ **replyBfrQ){
    BfrQInit(&PayloadBfrQ, NumBfrs, BfrQSize, PayloadBfrSpace);
    BfrQInit(&ReplyBfrQ, NumBfrs, BfrQSize, ReplyBfrSpace);
    *payloadBfrQ = &PayloadBfrQ;
    *replyBfrQ = &ReplyBfrQ;
}

/*-------------------- S e n d E r r o r P a y l o a d( ) -----------------------------
	Purpose:	Send an error payload to the reply task
        Parameters:     buffer queue address
        Return Value:   None
        Error Values:   Preamble Byte 1 Error       -1
                        Preamble Byte 2 Error       -2
                        Preamble Byte 3 Error       -3
                        Checksum Error              -4
                        Packet Too Short Error      -5
*/
CPU_VOID ConstructError(Payload *payload, CPU_CHAR *message){
  
    //Bring the error state back to a positive number to match the ErrorState
    // 0 - payloadLen
    switch(0 - payload->payloadLen){
    case E1:
        sprintf(message, "EP1");
        break;
    case E2:
        sprintf(message, "EP2");
        break;
    case E3:
        sprintf(message, "EP3");
        break;
    case E4:
        sprintf(message, "ECS");
        break;
    case E5:
        sprintf(message, "EBad Size");
        break;
    }
}

/*-------------------- C o n s t r u c t P a y l o a d( ) -----------------------------
	Purpose:	Construct a Payload from the PayloadBfr Read Q
        Parameters:     buffer queue address
        Return Value:   None
*/
CPU_VOID ConstructPayload(CPU_VOID *payload){
  
    PktBfr *pktBfr = (PktBfr *)payload;
    CPU_VOID *rec = pktBfr->data;
    
    pktBfr->payloadLen = BfrQRemByte(&PayloadBfrQ);
    if(pktBfr->payloadLen > 0)
        BfrQRead(&PayloadBfrQ, rec, pktBfr->payloadLen - PacketHeaderDiff);
}


/*-------------------- P a y l o a d T a s k( ) -------------------------------------
	Purpose:	Process a payload from the payload buffer queue read buffer and
                        put the reply message in the reply buffer queue write buffer.
        Parameters:     buffer queue address
        Return Value:   None

        This is BOTH a producer and consumer task
        Consumer - PayloadBfrQ
            BfrQPendRead, BfrQPostWrite
        Producer - ReplyBfrQ
            BfrQPendWrite, BfrQPostRead
*/
CPU_VOID PayloadTask(CPU_VOID *data){
 
    static Payload payload;
    static CPU_CHAR message[BfrQSize];
    
    for(;;){
        BfrQPendRead(&PayloadBfrQ); //Pend on available readbfrs in PayloadQ
        
        //Consumer
        ConstructPayload(&payload); //Consume Buffer
        BfrQPostWrite(&PayloadBfrQ); //Done Consuming
        
        //Producer
        BfrQPendWrite(&ReplyBfrQ);  //Pend on available writebfrs in ReplyQ
        if(ConstructMessage(&payload, message)){ //Produce Buffer
            ReplyPutMsg(&ReplyBfrQ, message); 
        }else{
            ReplyError(&ReplyBfrQ, message);
        }
        BfrQPostRead(&ReplyBfrQ); //Done Producing
    }
}

/*-------------------- R e v e r s e B y t e s 3 2 ( ) -------------------------------------
	Purpose:	Reverses the bytes in a 32bit unsigned integer.
*/
CPU_INT32U ReverseBytes32(CPU_INT32U *original){
    return ((*original >> 24)&0x000000FF) | ((*original << 8) &0x00FF0000) | 
      ((*original >> 8) &0x0000FF00) | ((*original << 24)&0xFF000000);
}