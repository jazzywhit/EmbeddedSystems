#ifndef Intrpt_H
#define Intrpt_H

/*--------------- I n t r p t . h ---------------

By: George Cheney
    ECE Dept.
    UMASS Lowell

PURPOSE
Provides functions to enable and disable interrupts.

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

#include "CPU.h"

// Level of nesting of interrupt disables.
extern CPU_INT16S disableCnt;

/*----- f u n c t i o n     p r o t o t y p e s -----*/
void IntDis(void);
void IntEn(void);
#endif