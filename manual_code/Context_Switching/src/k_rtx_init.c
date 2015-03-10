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

void k_rtx_init(void)
{
        __disable_irq();
        uart0_irq_init(); 
				timer_init(0);
        memory_init();
        process_init();
        __enable_irq();
	
	/* start the first process */
	
        k_release_processor();
}
