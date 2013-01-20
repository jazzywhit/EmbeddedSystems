/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 5   -   Jesse Whitworth
-----------------------------------------------------------------------
			       Assert.h
-----------------------------------------------------------------------
Assert module defined by Professor Cheney
*/

/*----------------------- a s s e r t () ----------------------
PURPOSE
On failed assertion, execute a BPKT instruction, trapping the debugger
allowing inspection of variable at the point of failure.
MACRO PARAMETERS
cond - A boolean which if false indicates a failed assertion
*/
#define assert(cond) \
  if(!(cond)) \
    asm(" BKPT 0xFF");