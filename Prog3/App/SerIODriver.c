/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 3   -   Jesse Whitworth
-----------------------------------------------------------------------
			      SerIODriver.c
-----------------------------------------------------------------------*/

#include "SerIODriver.h"
#include "Bfr.h"

// If not already defined, use the default buffer size of 4.
#ifndef BfrSize
#define BfrSize 4
#endif

//Required for polling the USART
#define USART_TXE 0x80
#define USART_RXNE 0x20

//Allocate iBfr
static CircBfr iBfr;
static CPU_INT08U iBfrSpace[BfrSize];

//Allocate oBfr
static CircBfr oBfr;
static CPU_INT08U oBfrSpace[BfrSize];


/*-------------------- I n i t S e r I O ( ) -------------------------------------
	Purpose:	Initialize the RS232 I/O driver by initializing both iBfr and oBfr
        Parameters:     None
        Return Value:   None
*/
CPU_VOID InitSerIO(CPU_VOID){
    BfrInit(&iBfr, iBfrSpace, BfrSize);
    BfrInit(&oBfr, oBfrSpace, BfrSize);
    
    //Setup the board to use USART2 and reset all relevant registers
    AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;
    USART2->SR  = 0x00C0;
    USART2->BRR = 0x0EA6;
    USART2->CR1 = 0x200C;
    USART2->CR2 = 0x0000;
    USART2->CR3 = 0x0000;
}

/*-------------------- P u t B y t e ( ) -------------------------------------
	Purpose:	If oBfr buffer is not full, write one byte into the buffer, and return
                        txChar as the return value; if, the buffer is full, return -1 indicating failure.
        Parameters:     The byte to be transmitted
        Return Value:   Success - PutByte() returns the character in 'txChar'
                        Failure - When put buffer already full, return -1
*/
CPU_INT16S PutByte(CPU_INT16S txChar){
    return BfrAddByte(&oBfr, txChar); //BfrAddByte already checks for full
}

/*-------------------- G e t B y t e ( ) -------------------------------------
	Purpose:	If iBfr is not empty, remove and return the next byte from the buffer; 
                        if the buffer is empty, return -1 indicating failure.
        Parameters:     None
        Return Value:   Success - GetByte() returns character removed from iBfr
                        Failure - If iBfr is empty, return -1
*/
CPU_INT16S GetByte(CPU_VOID){
    return BfrRemByte(&iBfr); //BfrRemByte already checks for empty
}

/*-------------------- N e x t B y t e ( ) -------------------------------------
	Purpose:	If iBfr is not empty, remove and return the next byte from the buffer; 
                        if the buffer is empty, return -1 indicating failure.
        Parameters:     None
        Return Value:   Success - GetByte() returns character removed from iBfr
                        Failure - If iBfr is empty, return -1
*/
CPU_INT16S NextByte(CPU_VOID){
    return BfrNextByte(&iBfr); //BfrNextByte already checks for empty
}

/*-------------------- S e r v i c e T x ( ) -------------------------------------
	Purpose:	If TXE = 1 and oBfr is not empty, then output one byte to the UART
                        Tx and return. If TXE = 0 or if oBfr is empty, just return
        Parameters:     None
        Return Value:   None
*/
CPU_VOID ServiceTx(CPU_VOID){
  if (USART2->SR & USART_TXE)
    if (!BfrEmpty(&oBfr))
      USART2->DR = (CPU_INT08U)BfrRemByte(&oBfr);
  return;
} 

/*-------------------- S e r v i c e R x ( ) -------------------------------------
	Purpose:	if RXNE = 1 and the iBfr is not full, then read a byte from the UART Rx
                        and add it to the iBfr. If RXNE = 0 or iBfr is full, return.
        Parameters:     None
        Return Value:   None
*/
CPU_VOID ServiceRx(CPU_VOID){
  if (USART2->SR & USART_RXNE)
    if (!BfrFull(&iBfr))
      BfrAddByte(&iBfr, (CPU_INT16S)USART2->DR);
  return;
}