/*
-----------------------------------------------------------------------
	                    Embedded Systems
                   Prog 5   -   Jesse Whitworth
-----------------------------------------------------------------------
			      SerIODriver.c
-----------------------------------------------------------------------
The RS232 driver module, which defines the circular buffers iBfr and oBfr, and I/O
driver functions, including the functions InitSerIO(), GetByte(), PutByte(), and the two
tasks ServiceTx() and ServiceRx() plus Ser_ISR(). Also defined in this module are the
two semaphores “bytesAvail” and “spacesAvail.”
*/
#include "Assert.h"
#include "SerIODriver.h"
#include "Bfr.h"

//----- c o n s t a n t    d e f i n i t  i o n s -----
//USART Bit Masks
#define USART_TXE 0x80          // Tx Empty Bit
#define USART_TXEIE 0x80        // Unmask Tx Interrupts
#define USART_RXNE 0x20         // Rx Empty Bit
#define USART_RXNEIE 0x20       // Unmask Rx Interrupts
//IRQ 38 Definitions
#define SETENA1 (*(CPU_INT32U *) 0xE000E104)
#define USART2ENA 0x00000040

//Setup macros for the mask/unmask of Tx and Rx interrupts
#define MASK_TX() (USART2->CR1 &= ~USART_TXEIE)
#define UNMASK_TX() (USART2->CR1 |= USART_TXEIE)
#define MASK_RX() (USART2->CR1 &= ~USART_RXNEIE)
#define UNMASK_RX() (USART2->CR1 |= USART_TXEIE)



//----- g l o b a l    v a r i a b l e s -----
// Timeout for semaphore wait
#define SuspendTimeout 100
OS_SEM	spacesAvail;	  /* Upon removing a byte from oBfr, ServiceTx() posts to this
                                     semaphore to signal PutByte() that a new data byte is available.*/
OS_SEM	bytesAvail;	  /* Upon adding a byte to iBfr, ServiceRx() posts to this semaphore to
                                     signal GetByte() that a space has been made available.*/

// Allocate the input buffer.
static CircBfr iBfr;
static CPU_INT08U iBfrSpace[BfrSize];

// Allocate the output buffer.
static CircBfr oBfr;
static CPU_INT08U oBfrSpace[BfrSize];


/*-------------------- I n i t S e r I O ( ) -------------------------------------
	Purpose:	[Initialize the RS232 I/O driver by initializing both iBfr and oBfr.
                        Unmask the Tx and the Rx, and enable IRQ38. Also initialize the
                        two semaphores bytesAvail and spacesAvail.]
        Parameters:     None
        Return Value:   None
*/
CPU_VOID InitIODriver(CPU_VOID){
    OS_ERR osErr; /* -- Semaphore error code */
    
    /* Initialize buffers. */
    BfrInit(&iBfr, iBfrSpace, BfrSize);
    BfrInit(&oBfr, oBfrSpace, BfrSize);
    
    /* Create and initialize semaphores. */
    OSSemCreate(&spacesAvail, "Buffer Spaces Avail", BfrSize, &osErr);
    assert(osErr == OS_ERR_NONE);
    OSSemCreate(&bytesAvail, "Bytes Avail", 0, &osErr);
    assert(osErr == OS_ERR_NONE);
    
    // Setup the board to use USART2
    // Unmask the Tx and Rx interrupts while setting CR1
    AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;
    USART2->SR  = 0x00C0;
    USART2->BRR = 0x0EA6;
    USART2->CR1 = 0x20AC; // Unmask UE, TXEIE, RXNEIE, TE, RE
    USART2->CR2 = 0x0000;
    USART2->CR3 = 0x0000;
      
    // Enable IRQ38 - Set NVIC Register SETENA1[6]
    BSP_IntVectSet(38, Ser_ISR); // Setup Ser_ISR as the interrupt handler for USART2 at irq 38
    SETENA1 = USART2ENA;
    
    /*
    // Unmask Tx/Rx interrupts.
    USART2->CR1 |= USART_TXEIE;
    USART2 ->CR1 |= USART_RXNEIE;
    
    // Enable IRQ38.
    BSP_IntVectSet(38, Ser_ISR); // Setup Ser_ISR as the interrupt handler for USART2 at irq 38
    SETENA1 = USART2ENA;
    */
}

/*-------------------- P u t B y t e ( ) -------------------------------------
	Purpose:	[Pend on the semaphore “spacesAvail” and then write one
                        byte into oBfr, unmask the Tx interrupt, and return txChar]
        Parameters:     The byte to be transmitted
        Return Value:   Success - PutByte() returns the character in 'txChar'
                        Failure - When put buffer already full, return -1
*/
CPU_INT16S PutByte(CPU_INT16S txChar){
    OS_ERR osErr; /* -- Semaphore error code */
    
    OSSemPend(&spacesAvail, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr);
    BfrAddByte(&oBfr, txChar);
    UNMASK_TX();
    
    return txChar; 
}

/*-------------------- G e t B y t e ( ) -------------------------------------
	Purpose:	[Pend on the semaphore "bytesAvail" and then remove one byte from
                        iBfr, unmask Rx interrup, and return result of BfrRemByte.]
        Parameters:     None
        Return Value:   Success - GetByte() returns character removed from iBfr
                        Failure - If iBfr is empty, return -1
*/
CPU_INT16S GetByte(CPU_VOID){
    OS_ERR osErr; /* -- Semaphore error code */
    
    OSSemPend(&bytesAvail, 0, OS_OPT_PEND_BLOCKING, NULL, &osErr); 
    CPU_INT16S byte = BfrRemByte(&iBfr);
    UNMASK_RX();
    
    return byte;
}

/*-------------------- S e r v i c e T x ( ) -------------------------------------
	Purpose:	[If TXE = 0, just return.
                        Otherwise, if oBfr is empty, mask the Tx interrupt and return.
                        If TXE = 1 and the oBfr is not empty, remove the next byte
                        from oBfr and output it to the Tx then post to the semaphore “spacesAvail.”]
        Parameters:     None
        Return Value:   None
*/
CPU_VOID ServiceTx(CPU_VOID){
    OS_ERR osErr; /* -- Semaphore error code */
    
    //if (USART2->SR & USART_TXE) -- Original
    if ((USART2->SR & USART_TXE)){
        if (!BfrEmpty(&oBfr)){
            USART2->DR = (CPU_INT08U)BfrRemByte(&oBfr);
            OSSemPost(&spacesAvail, OS_OPT_POST_1, &osErr);
            assert(osErr==OS_ERR_NONE);
        }else{
            MASK_TX();
        }
    }
} 

/*-------------------- S e r v i c e R x ( ) -------------------------------------
	Purpose:	[if RXNE = 1 and the iBfr is not full, then read a byte from the UART Rx
                        and add it to the iBfr then post to the semaphore "bytesAvail." 
                        If RXNE = 0, return. If oBfr is full, mask the Rx interrupt and return.]
        Parameters:     None
        Return Value:   None
*/
CPU_VOID ServiceRx(CPU_VOID){
    OS_ERR osErr; /* -- Semaphore error code */
    
    if (USART2->SR & USART_RXNE){
        if (!BfrFull(&iBfr)){
            BfrAddByte(&iBfr, (CPU_INT16S)USART2->DR);
            OSSemPost(&bytesAvail, OS_OPT_POST_1, &osErr);
            assert(osErr==OS_ERR_NONE);
        }else{
            MASK_RX();
        }
    }
}

/*-------------------- S e r _ I S R ( ) -------------------------------------
	Purpose:	Call ServiceRx() to handle Rx interrupts and then call
                        ServiceTx() to handle Tx interrupts.
                        [Added the prologue and epilogue to make this ISR compatible
                        with uC/OS-III.]
*/
CPU_VOID Ser_ISR(CPU_VOID){
    //OS_ERR osErr; /* O/S Error code */
  
    /*---------------------- Prologue ----------------*/   
    /*Disable Interrupts*/
    CPU_SR_ALLOC();
    OS_CRITICAL_ENTER();
    
        /*Tell the kernel we are in an ISR*/
        OSIntEnter();
    
    /* Enable Interrupts */
    OS_CRITICAL_EXIT();
    /*----------------------  ISR  -------------------*/
    
    ServiceRx();
    ServiceTx();
    
    /*---------------------- Epilogue ----------------*/
    /*Give the O/S a chance to swap tasks. */
    OSIntExit();
}