#ifndef CPU_H
#define CPU_H

/*=============== C P U . h ===============*/

/*
BY:   George Cheney
      16.472 / 16.572 Embedded Real Time Systems
      Electrical and Computer Engineering Dept.
      UMASS Lowell
*/

/*
PURPOSE
Define CPU/Compiler specific data types and constants.

CHANGES
01-24-2012 gpc - Added CPU_CHAR and CPU_BOOLEAN
*/    


/*----- t y p e    d e f i n i t i o n s -----*/

/* Boolean data type constants */
typedef enum
{
   false = 0,
   //False = 0,
   //FALSE = 0,
   true = 1,
   //True = 1,
   //TRUE = 1
} Boolean;

/* CPU specific data types */
typedef signed char     CPU_INT08S;     
typedef unsigned char   CPU_INT08U;   
typedef short           CPU_INT16S;           
typedef unsigned short  CPU_INT16U;  
typedef long            CPU_INT32S;            
typedef unsigned long   CPU_INT32U;   
typedef char            CPU_CHAR;
typedef unsigned char   CPU_BOOLEAN;

#endif