#ifndef REPLY_H
#define REPLY_H
/*--------------- R e p l y . h ---------------

by: George Cheney
    ECE Dept.
    UMASS Lowell

PURPOSE
This is the interface to the Reply task module.

CHANGES
02-13-2012 gpc -  Created
*/
#include "BfrQ.h"

/*----- f u n c t i o n    p r o t o t y p e s -----*/

CPU_VOID Reply(BfrQ *replyBfrQ);
CPU_VOID ReplyPutMsg(BfrQ *replyBfrQ, CPU_CHAR *msg);
CPU_VOID ReplyError(BfrQ *replyBfrQ, CPU_CHAR *msg);
#endif