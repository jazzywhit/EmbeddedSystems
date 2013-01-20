/*--------------- R e p l y . c ---------------

by: George Cheney
    ECE Dept.
    UMASS Lowell

PURPOSE
This module defines the reply task.

CHANGES
02-20-2012 gpc -  Created

Reply task module - same as Program 3 - Do not change.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "includes.h"

#include "Reply.h"
#include "BfrQ.h"
#include "SerIODriver.h"

/*--------------- R e p l y ( ) ---------------

PURPOSE
This is the reply task, which outputs the reply buffer queue read buffer to the
RS232 transmit port.

INPUT PARAMETERS
replyBfrQ - The address of the reply buffer queue.
*/
CPU_VOID Reply(BfrQ *replyBfrQ)
{
  // Block if the reply buffer queue read buffer is not ready.
  if (!BfrQReadClosed(replyBfrQ))
    return;
  
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
    if (PutByte(c) < 0)
      return;
    
    // Now remove the byte.
    BfrQRemByte(replyBfrQ);
    }
  // Mark the reply buffer queue read buffer open.
  BfrQOpenRead(replyBfrQ);
}  
  
/*--------------- R e p l y P u t M s g ( ) ---------------

PURPOSE
Copy a message to the reply buffer queue write buffer.

INPUT PARAMETERS
replyBfrQ - The address of the reply buffer queue.
msg           - The message to copy
*/
CPU_VOID ReplyPutMsg(BfrQ *replyBfrQ, CPU_CHAR *msg)
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
                whether the message is an ERROR message or INFO message.
*/
CPU_VOID ReplyError(BfrQ *replyBfrQ, CPU_CHAR *msg)
{
#define MaxErrorMsg 80  // Maximum length of error message.
#define ErrorFlag 'E'   // ERROR flag character
  
  // Error message buffer
  CPU_CHAR  msgBfr[MaxErrorMsg+1];
  
  // Form the error message prefix.
  if (msg[0] == ErrorFlag)
    strcpy(msgBfr, "\n*** ERR: ");
  else
    strcpy(msgBfr, "\n*** INFO: ");
  
  // Form the rest of the error message.
  strncat(msgBfr, msg+1, MaxErrorMsg-strlen(msgBfr)-1);
  strncat(msgBfr, "\n", MaxErrorMsg-strlen(msgBfr));
   
  // Copy the message to the reply buufer queue write buffer.
  ReplyPutMsg(replyBfrQ, msgBfr);
   
  // Close the reply buufer queue write buffer.
  BfrQCloseWrite(replyBfrQ);
}
