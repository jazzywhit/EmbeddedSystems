#include "includes.h"
#include "Intrpt.h"

/*--------------- I n t r p t . c ---------------

By: George Cheney
    ECE Dept.
    UMASS Lowell

PURPOSE
Defines global variable "disableCnt" to track nested
interrupt disables. 

IntDis() and IntEn() will be treated as inline functions
if the symbol "INLINE_INT_FCNS" has been defined. 
Otherwise, they will be treated as ordinary functions.

CHANGES
03-03-2012  gpc - Created for 16.572 Program 4
*/

/*
Provides IntEn() and IntDis(), interrupt enable/disable
functions accommodating nested interrupt disables –
provided.
*/

// Level of nesting of interrupt disables.
static CPU_INT16S disableCnt = 0;

/*--------------- I n t D i s ( ) ----------

PURPOSE
Set PRIMASK = 1 to disable interrupts.
*/

void IntDis(void)
{
 	asm(" cpsid i");
	++disableCnt;
}

/*--------------- I n t E n ( ) ----------

PURPOSE
Clear PRIMASK = 0 to enable interrupts.
*/

void IntEn(void)
{
	if (disableCnt)
		--disableCnt;
 	if (disableCnt == 0)
		asm(" cpsie i");
}
