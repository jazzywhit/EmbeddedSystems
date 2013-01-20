/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 1   -   Jesse Whitworth
-----------------------------------------------------------------------
			              Error.c
-----------------------------------------------------------------------*/
#include "Error.h"
#include "stdio.h"

/*-------------------- S h o w E r r o r ( ) -------------------------------------
	Purpose:	Display error messages in a standardized way.
*/
void ShowError(const CPU_CHAR *message){
	fprintf(stderr, "*** ERROR: %s\n\n", message);
}