/** 
 * @file:   k_rtx_init.c
 * @brief:  Kernel initialization C file
 * @auther: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_rtx_init.h"
#include "k_memory.h"
#include "k_process.h"
#include "uart.h"
#include "timer.h"
#include "uart_polling.h"
#include "printf.h"

#include "LPC17xx.h"
void k_rtx_init(void)
{
        __disable_irq();
        uart0_irq_init(); 
	uart1_init();
	timer_init(0);
#ifdef DEBUG_0  
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
#endif /*DEBUG_0*/
        memory_init();
        __enable_irq();
	/* start the first process */
        k_release_processor();
}
