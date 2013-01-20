/*--------------- P r o g 5 . c ---------------

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
/*
-----------------------------------------------------------------------
	                    Embedded Systems
                        Prog 5   -   Jesse Whitworth
-----------------------------------------------------------------------
			       Prog5.c
-----------------------------------------------------------------------
Create the static buffer queues that will be used to handle the payloads

The payload task module – PayloadTask() must be a uC/OS-III style task
*/

#include <stdio.h>
#include <stdlib.h>
#include "Assert.h"
#include "includes.h"
#include "BfrQ.h"
#include "Reply.h"
#include "Payload.h"
#include "Parser.h"
#include "SerIODriver.h"

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

#define Init_STK_SIZE 128      // Init task Priority
#define Init_PRIO 2             // Init task Priority

// Define RS232 baud rate.
#define BaudRate 9600

/*----- m o d u l e    g l o b a l    v a r i a b l e s -----*/
static  OS_TCB   initTCB;                         // Init task TCB
static  CPU_STK  initStk[Init_STK_SIZE];          // Space for Init task stack

/*----- f u n c t i o n    p r o t o t y p e s -----*/
static  CPU_VOID  Init  (CPU_VOID *p_arg);


/*--------------- I n i t( ) ---------------
PURPOSE
This will initialize tasks and board support functions.
*/
static CPU_VOID Init(CPU_VOID *p_arg)
{
    //--------------------- Necessary intitialization protocols
    CPU_INT32U  cpu_clk_freq;                                     /* CPU Clock frequency */
    CPU_INT32U  cnts;                                             /* CPU clock interval */
    OS_ERR      err;                                              /* OS Error code */
    
    BSP_Init();                                                   /* Initialize BSP functions  */
    CPU_Init();                                                   /* Initialize the uC/CPU services */

    cpu_clk_freq = BSP_CPU_ClkFreq();                             /* Determine SysTick reference freq. */                                                                        
    cnts         = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;  /* Determine nbr SysTick increments */
    OS_CPU_SysTickInit(cnts);                                     /* Init uC/OS periodic time src (SysTick). */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                                 /* Compute CPU capacity with no task running */
#endif

    CPU_IntDisMeasMaxCurReset();
    
    //First Task should enable interrupts according to the uC/OS III book
    //CPU_IntEn();
    
    //--------------------- Start of Init() Task
    // Payload and Reply buffer queue pointers
    BfrQ *payloadBfrQ;
    BfrQ *replyBfrQ;
    
    // Initialize the payload
    PayloadInit(&payloadBfrQ, &replyBfrQ);
    
    // Create Tasks.
    CreateParserTask(payloadBfrQ);
    CreatePayloadTask();
    CreateReplyTask(replyBfrQ);
    
    // Initialize USART2.
    BSP_Ser_Init(BaudRate);

    // Initialize the USART2 I/O driver. 
    InitIODriver();
    
    // Delete the Init task.
    OSTaskDel(&initTCB, &err);
    assert(err == OS_ERR_NONE);
}

/*--------------- m a i n ( ) -----------------*/
CPU_INT32S main()
{
    // OS Error Code
    OS_ERR  err;  
    
    // Disable all interrupts... When are these re-enabled?
    BSP_IntDisAll();
    
    // Init uC/OS-III.
    OSInit(&err);                         
    assert(err == OS_ERR_NONE);
    
    // Create the init task.
    OSTaskCreate(&initTCB,            // Task Control Block                
               "Init Task",         // Task name
               Init,                // Task entry point
               NULL,                // Address of optional task data block
               Init_PRIO,           // Task priority
               &initStk[0],         // Base address of task stack space
               Init_STK_SIZE / 10,  // Stack water mark limit
               Init_STK_SIZE,       // Task stack size
               0,                   // This task has no task queue
               0,                   // Number of clock ticks (defaults to 10)
               0,                   // Pointer to TCB extension
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),  // Task options
               &err);               // Address to return O/S error code
    
    /* Verify successful task creation. */
    assert(err == OS_ERR_NONE);
    
    /* Initialize the RS232 interface. */
    BSP_Ser_Init(BaudRate);
    
    // Start multitasking.
    OSStart(&err);                        
    assert(err == OS_ERR_NONE);
}
