/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 2   -   Jesse Whitworth
-----------------------------------------------------------------------
			              Error.c
-----------------------------------------------------------------------*/
#include "Error.h"
#include "includes.h"

/*-------------------- S h o w E r r o r ( ) -------------------------------------
	Purpose:	Display error messages in a standardized way.
*/
void ShowError(const CPU_CHAR *message){
	BSP_Ser_Printf("\a*** ERROR: %s\n\n", message);
}
