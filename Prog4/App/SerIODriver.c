/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 4   -   Jesse Whitworth
-----------------------------------------------------------------------
			      SerIODriver.c
-----------------------------------------------------------------------*/

#include "SerIODriver.h"
#include "Bfr.h"

// If not already defined, use the default buffer size of 4.
#ifndef BfrSize
#define BfrSize 4
#endif

// Allocate the input buffer.
static CircBfr iBfr;
static CPU_INT08U iBfrSpace[BfrSize];

// Allocate the output buffer.
static CircBfr oBfr;
static CPU_INT08U oBfrSpace[BfrSize];

//Required for polling the USART
#define USART_TXE 0x80          // Tx Empty Bit
#define USART_TXEIE 0x80        // Unmask Tx Interrupts
#define USART_RXNE 0x20         // Rx Empty Bit
#define USART_RXNEIE 0x20       // Unmask Rx Interrupts

//Setup macros for the mask/unmask of Tx and Rx interrupts
#define MASK_TX() (USART2->CR1 &= ~USART_TXEIE)
#define UNMASK_TX() (USART2->CR1 |= USART_TXEIE)
#define MASK_RX() (USART2->CR1 &= ~USART_RXNEIE)
#define UNMASK_RX() (USART2->CR1 |= USART_TXEIE)

/*-------------------- I n i t S e r I O ( ) -------------------------------------
	Purpose:	Initialize the RS232 I/O driver by initializing both iBfr and oBfr.
                        [Also unmask the Tx and the Rx and enable IRQ38.]
        Parameters:     None
        Return Value:   None
*/
CPU_VOID InitSerIO(CPU_VOID){

    BfrInit(&iBfr, iBfrSpace, BfrSize);
    BfrInit(&oBfr, oBfrSpace, BfrSize);
    
    // Setup the board to use USART2
    // Unmask the Tx and Rx interrupts while setting CR1
    AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;
    USART2->SR  = 0x00C0;
    USART2->BRR = 0x0EA6;
    USART2->CR1 = 0x20AC; // Unmask UE, TXEIE, RXNEIE, TE, RE
    USART2->CR2 = 0x0000;
    USART2->CR3 = 0x0000;
      
    // Enable IRQ38 - Set NVIC Register SETENA1[6]
    #define SETENA1 (*(CPU_INT32U *) 0xE000E104)
    #define USART2ENA 0x00000040
    SETENA1 = USART2ENA;
    
    // Setup Ser_ISR as the interrupt handler for USART2 at irq 38
    // As defined in app_vect:
    // BSP_IntHandlerUSART2,  /* 54, INTISR[ 38]  USART2 Global Interrupt.*/
    BSP_IntVectSet(38, Ser_ISR);
}

/*-------------------- N e x t B y t e ( ) -------------------------------------
	Purpose:	If iBfr is not empty, remove and return the next byte from the buffer; 
                        if the buffer is empty, return -1 indicating failure.
        Parameters:     None
        Return Value:   Success - NextByte() returns the next character in iBfr
                        Failure - If iBfr is empty, return -1
*/
CPU_INT16S NextByte(CPU_VOID){
    return BfrNextByte(&iBfr); //BfrNextByte already checks for empty
}

/*-------------------- P u t B y t e ( ) -------------------------------------
	Purpose:	If oBfr buffer is not full, write one byte into the buffer,
                        [unmask the Tx interrupt], and return txChar as the return value; 
                        if, the buffer is full, return -1 indicating failure.
        Parameters:     The byte to be transmitted
        Return Value:   Success - PutByte() returns the character in 'txChar'
                        Failure - When put buffer already full, return -1
*/
CPU_INT16S PutByte(CPU_INT16S txChar){
    CPU_INT16S byte = BfrAddByte(&oBfr, txChar);
    if(byte >= 0) //Check if BfrAddByte returned -1 or full
        UNMASK_TX();
    return byte; 
}

/*-------------------- G e t B y t e ( ) -------------------------------------
	Purpose:	If iBfr is not empty, [unmask the Rx interrupt], 
                        remove and return the next byte from the buffer; 
                        if the buffer is empty, return -1 indicating failure.
        Parameters:     None
        Return Value:   Success - GetByte() returns character removed from iBfr
                        Failure - If iBfr is empty, return -1
*/
CPU_INT16S GetByte(CPU_VOID){
    CPU_INT16S byte = BfrRemByte(&iBfr);
    if(byte >= 0) //Check if BfrRemByte returned -1 or empty
        UNMASK_RX();
    return byte;
}

/*-------------------- S e r v i c e T x ( ) -------------------------------------
	Purpose:	If TXE = 1 and oBfr is not empty, then output one byte to the UART
                        Tx and return. If TXE = 0, just return.
                        [If the get buffer (iBfr) is empty, mask the Tx and return]
        Parameters:     None
        Return Value:   None
*/
CPU_VOID ServiceTx(CPU_VOID){
    if (USART2->SR & USART_TXE)
        if (!BfrEmpty(&oBfr))
            USART2->DR = (CPU_INT08U)BfrRemByte(&oBfr);
        else
            MASK_TX();
  return;
} 

/*-------------------- S e r v i c e R x ( ) -------------------------------------
	Purpose:	if RXNE = 1 and the iBfr is not full, then read a byte from the UART Rx
                        and add it to the iBfr. If RXNE = 0, return.
                        [If oBfr is full, mask the Rx interrupt and return.]
        Parameters:     None
        Return Value:   None
*/
CPU_VOID ServiceRx(CPU_VOID){
  if (USART2->SR & USART_RXNE)
        if (!BfrFull(&iBfr))
            BfrAddByte(&iBfr, (CPU_INT16S)USART2->DR);
        else
            MASK_RX();
  return;
}

/*-------------------- S e r _ I S R ( ) -------------------------------------
	Purpose:	Call ServiceRx() to handle Rx interrupts and then call
                        ServiceTx() to handle Tx interrupts.
*/
CPU_VOID Ser_ISR(CPU_VOID){
    ServiceRx();
    ServiceTx();
}