/*--------------- R e p l y . c ---------------

by: George Cheney
    ECE Dept.
    UMASS Lowell

PURPOSE
This module defines the reply task.

CHANGES
02-13-2012 gpc -  Created
03-06-2012 gpc -  Updated for Program 5 and uC/OS-III
*/

/*
  ---------------------------------------------
  Modified by: Jesse Whitworth
  04/18/2012 - Changed function names
*/ 


#include <string.h>
#include "Assert.h"
#include "includes.h"
#include "Reply.h"
#include "BfrQ.h"
#include "SerIODriver.h"

//----- c o n s t a n t    d e f i n i t  i o n s -----
#define SuspendTimeout 100   // Timeout for semaphore wait
#define REPLY_STK_SIZE 128  // Reply Task stack size
#define ReplyPrio 5         // Reply Task Priority

//----- g l o b a l    v a r i a b l e s -----

static  OS_TCB   replyTCB;                  // Reply Task TCB
static  CPU_STK  replyStk[REPLY_STK_SIZE];  // Space for Reply Task stack

/*----- f u n c t i o n    p r o t o t y p e s -----*/

CPU_VOID Reply(CPU_VOID *data);

/*--------------- C r e a t e R e p l y T a s k( ) ---------------

PURPOSE
Create the Reply Task.

INPUT PARAMETERS
replyBfrQ - The address of the reply buffer queue.
*/
CPU_VOID CreateReplyTask(CPU_VOID *replyBfrQ)
{
  /* O/S error code */
  OS_ERR  osErr;                           
  
  /* Create the Reply Task. */
  OSTaskCreate(  &replyTCB,           // Task Control Block
                 "Reply Task",        // Task name
                 Reply,               // Task entry point
                 replyBfrQ,           // Address of reply buffer queue
                 ReplyPrio,           // Task priority
                 &replyStk[0],        // Base address of task stack space
                 REPLY_STK_SIZE / 10, // Stack water mark limit
                 REPLY_STK_SIZE,      // Task stack size
                 0,                   // This task has no task queue
                 0,                   // Number of clock ticks (defaults to 10)
                 (CPU_VOID      *)0,  // Pointer to TCB extension
                 OS_OPT_TASK_NONE,    // Task options
                 &osErr);             // Address to return O/S error code
    
  /* Verify successful task creation. */
  assert(osErr == OS_ERR_NONE);
}

/*--------------- R e p l y ( ) ---------------

PURPOSE
This is the reply task, which outputs the reply buffer queue read buffer to the
RS232 transmit port.

INPUT PARAMETERS
replyBfrQ - The address of the reply buffer queue.

This is a consumer Task. Should use BfrQPostWrite and BfrQPendRead
*/
CPU_VOID Reply(CPU_VOID *data)
{
  
  BfrQ *replyBfrQ = (BfrQ *) data;
  
  for (;;)
    {
    // Block if the reply buffer queue read buffer is not ready.
    BfrQPendRead(replyBfrQ);
  
    // Repeatedly get the next byte from the reply buffer queue read buffer
    // and output it to oBfr.
    for (;;)
      {
      // Get the next byte, but don't remove it yet.
      CPU_INT16S c = BfrQNextByte(replyBfrQ);
      
      // If no next byte, this reply is complete.
      if (c < 0)
        break;
  
      // Block if oBfr is full.
      PutByte(c);
      
      // Now remove the byte.
      BfrQRemByte(replyBfrQ);
      }
    
    // Post to the Write buffer to signal - done consuming
    BfrQPostWrite(replyBfrQ);
    }
}  

/*--------------- R e p l y P u t M s g ( ) ---------------

PURPOSE
Copy a message to the reply buffer queue write buffer.

INPUT PARAMETERS
replyBfrQ - The address of the reply buffer queue.
msg           - The message to copy
*/
CPU_VOID ReplyPutMsg(BfrQ *replyBfrQ, const CPU_CHAR *msg)
{
  // Copy the message until the end.
  while (*msg != '\0')
    BfrQAddByte(replyBfrQ, *msg++);
}

/*--------------- R e p l y E r r o r ( ) ---------------

PURPOSE
Write an error mesage to the reply buffer queue write buffer.

INPUT PARAMETERS
replyBfrQ - The address of the reply buffer queue.
msg           - The message to copy. The first character is a flag indicating
                whether the message is an ERROR message of INFO message.
*/
#ifdef ShortReplies
CPU_VOID ReplyError(BfrQ *replyBfrQ, const CPU_CHAR *msg)
{
#define MaxErrorMsg 80  // Maximum length of error message.
  
  // Error message buffer
  CPU_CHAR  msgBfr[MaxErrorMsg];
  
  // Form the error message prefix.
  strcpy(msgBfr, "\n*** ERR: ");
  
  // Form the rest of the error message.
  strncat(msgBfr, msg+1, MaxErrorMsg-strlen(msgBfr)-1);
  strncat(msgBfr, "\n", MaxErrorMsg-strlen(msgBfr));
   
  // Copy the message to the reply buufer queue write buffer.
  ReplyPutMsg(replyBfrQ, msgBfr);
   
  // Close the reply buufer queue write buffer.
  //BfrQCloseWrite(replyBfrQ);
}
#else  
CPU_VOID ReplyError(BfrQ *replyBfrQ, const CPU_CHAR *msg)
{
#define MaxErrorMsg 80  // Maximum length of error message.
  
  // Error message buffer
  CPU_CHAR  msgBfr[MaxErrorMsg];
  
  // Form the error message prefix.
  strcpy(msgBfr, "\n******************** ERROR: ");
  
  // Form the rest of the error message.
  strncat(msgBfr, msg+1, MaxErrorMsg-strlen(msgBfr)-1);
  strncat(msgBfr, "\n", MaxErrorMsg-strlen(msgBfr));
   
  // Copy the message to the reply buffer queue write buffer.
  ReplyPutMsg(replyBfrQ, msgBfr);
   
  // Close the reply buffer queue write buffer.
  BfrQCloseWrite(replyBfrQ);
}
#endif
