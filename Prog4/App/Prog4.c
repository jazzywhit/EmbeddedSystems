/*--------------- P r o g 4 . c ---------------

by: George Cheney
    ECE Dept.
    UMASS Lowell

PURPOSE
Receive wireless sensor network packets from the RS232 port,
interpret and display the messages.

DEMONSTRATES
Cooperative multitasking
Concurrent polled I/O

CHANGES
02-13-2012 gpc -  Created
*/

#include <stdio.h>
#include <stdlib.h>

#include "includes.h"
#include "BfrQ.h"
#include "Reply.h"
#include "Payload.h"
#include "Parser.h"
#include "SerIODriver.h"
#include "Intrpt.h"

//#define TXTEST
//#define RXTEST
#define NOTEST

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

// Define RS232 baud rate.
#define BaudRate 9600

/*----- m o d u l e    g l o b a l    v a r i a b l e s -----*/


/*----- f u n c t i o n    p r o t o t y p e s -----*/

CPU_VOID AppMain(CPU_VOID);

/*--------------- m a i n ( ) -----------------*/

CPU_INT32S main()
{
    //* Disable all interrupts. */
    IntDis();
    
    /* Initialize BSP functions */
    BSP_Init();
    
    /* Initialize the RS232 interface. */
    BSP_Ser_Init(BaudRate);
    
    // Run the application.
    AppMain();
    
    return 0;
}
/*--------------- A p p M a i n ( ) ---------------
PURPOSE
This is the application main program.
*/
#ifdef TXTEST
CPU_VOID AppMain(CPU_VOID)
{
    
    // Initialize the circular buffers iBfr and oBfr. Also unmask the Rx and Tx
    // interrupts and enable IRQ38.
    InitSerIO();
    
    // Turn on global interrupt enable.
    IntEn();
    
    // Multitasking Executive Loop: Tasks are executed round robin.
    for (;;){
        static CPU_INT16S c = 'A';
        
        while(TRUE){
            if (PutByte(c) < 0)
                break;
            
            if(c < 'Z')
                c++;
            else
                c='A';
        } 
    }
}
#endif

#ifdef RXTEST
CPU_VOID AppMain(CPU_VOID)
{
    CPU_BOOLEAN putPending = FALSE;
    
    // Initialize the circular buffers iBfr and oBfr. Also unmask the Rx and Tx
    // interrupts and enable IRQ38.
    InitSerIO();
    
    // Turn on global interrupt enable.
    IntEn();
    
    // Multitasking Executive Loop: Tasks are executed round robin.
    while(TRUE){
        
        CPU_INT16S c;
        
        for(;;){
            if(!putPending)
                c = GetByte();
            if(c < 0)
                break;
            if (PutByte(c) < 0){
                putPending = TRUE;
                break;
            }else
                putPending = FALSE;
        }
    } 
}
#endif

#ifdef NOTEST
CPU_VOID AppMain(CPU_VOID)
{
    // Payload and Reply buffer queue pointers
    BfrQ *payloadBfrQ;
    BfrQ *replyBfrQ;
    
    // Initialize the circular buffers iBfr and oBfr. Also unmask the Rx and Tx
    // interrupts and enable IRQ38.
    InitSerIO();
    
    // Turn on global interrupt enable.
    IntEn();
    
    // Initialize the Payload and Reply buffer queues and return their addresses.
    PayloadInit(&payloadBfrQ, &replyBfrQ);
    
    // Multitasking Executive Loop: Tasks are executed round robin.
    for (;;){
        // Packet Parser Task: Read a packet from iBfr and extract a payload to the
        // payload buffer queue write buffer.
        Parser(payloadBfrQ);
        // Payload Task: Process a payload from the payload buffer queue read buffer and
        // put the reply message in the reply buffer queue write buffer.
        PayloadTask();
        // Reply Task: Read a reply message from the reply buffer queue read buffer and
        // write one byte at a time to obfr.
        Reply(replyBfrQ);
    }
}
#endif
