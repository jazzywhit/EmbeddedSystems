 /*--------------- R e p l y . h ---------------
  
  by: George Cheney
      ECE Dept.
      UMASS Lowell
  
  PURPOSE
  This is the interface to the Reply task module.
  
  CHANGES
  02-13-2012 gpc -  Created
  03-06-2012 gpc -  Updated for Program 5 and uC/OS-III
  */

  /*
  ---------------------------------------------
  Modified by: Jesse Whitworth
  04/18/2012 - Changed function names
  */ 

  #ifndef REPLY_H
  #define REPLY_H

  #include "BfrQ.h"
  
  #define ShortReplies
  
  /*----- f u n c t i o n    p r o t o t y p e s -----*/
  
  CPU_VOID CreateReplyTask(CPU_VOID *replyBfrQ); //Create semaphores
  CPU_VOID Reply(CPU_VOID *data);
  CPU_VOID ReplyPutMsg(BfrQ *replyBfrQ, const CPU_CHAR *msg);
  CPU_VOID ReplyError(BfrQ *replyBfrQ, const CPU_CHAR *msg);
  CPU_VOID BfrQPendRead(BfrQ *bfrQ);
  #endif