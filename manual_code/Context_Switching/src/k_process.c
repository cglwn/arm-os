/**
 * @file:   k_process.c  
 * @brief:  process management C file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/01/17
 * NOTE: The example code shows one way of implementing context switching.
 *       The code only has minimal sanity check. There is no stack overflow check.
 *       The implementation assumes only two simple user processes and NO HARDWARE INTERRUPTS. 
 *       The purpose is to show how context switch could be done under stated assumptions. 
 *       These assumptions are not true in the required RTX Project!!!
 *       If you decide to use this piece of code, you need to understand the assumptions and
 *       the limitations. 
 */

#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "uart_polling.h"
#include "k_process.h"
#include "k_memory.h"
#include "k_utilities.h"
#include "message.h"
#include "sys_proc.h"
#include "clock.h"
#include "priority.h"
#include "uart_irq.h"
#include "timer.h"
#include "stress.h"

#ifdef DEBUG_0
#include "printf.h"
#include "assert.h"
#endif /* DEBUG_0 */

/* ----- Global Variables ----- */
PCB **gp_pcbs;                  /* array of pcbs */
PCB *gp_current_process = NULL; /* always point to the current RUN process */

PCB *PCBReadyQueue[5];
PCB *PCBBlockedQueue[5];
MSG_HEADER *pending_delayed_messages = NULL;
MSG_HEADER *pending_crt_messages = NULL;

/* process initialization table */
PROC_INIT g_proc_table[NUM_TEST_PROCS];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];
extern volatile uint32_t g_timer_count;

/**
 * @biref: initialize all processes in the system
 * NOTE: We assume there are only two user processes in the system in this example.
 */
void process_init() 
{
	int i;
	U32 *sp;
  
	for ( i = 0; i < NUM_PRIORITIES; i++)  {
		PCBReadyQueue[i] = NULL;
		PCBBlockedQueue[i] = NULL;
	}
       /* fill out the initialization table */
	set_test_procs();
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_proc_table[i].m_pid = g_test_procs[i].m_pid;
		g_proc_table[i].m_stack_size = g_test_procs[i].m_stack_size;
		g_proc_table[i].mpf_start_pc = g_test_procs[i].mpf_start_pc;
		g_proc_table[i].m_priority = g_test_procs[i].m_priority;
	}
  
	/* initilize exception stack frame (i.e. initial context) for each process */
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		int j;
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_state = NEW;
		(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
		(gp_pcbs[i])->mp_next = NULL;
		(gp_pcbs[i])->msg_q = NULL;

		sp = alloc_stack((g_proc_table[i]).m_stack_size);
#ifdef DEBUG_1
		printf("User process %d stack pointer is 0x%x.\n", i+1, sp);
#endif
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;

		enqueuePriority(PCBReadyQueue, gp_pcbs[i]);
	}
	
	/*Initialize Clock process*/
	(gp_pcbs[PID_CLOCK])->m_pid = PID_CLOCK;
	(gp_pcbs[PID_CLOCK])->m_state = NEW;
	(gp_pcbs[PID_CLOCK])->m_priority = 0;
	(gp_pcbs[PID_CLOCK])->mp_next = NULL;
	(gp_pcbs[PID_CLOCK])->msg_q = NULL;
	
	sp = alloc_stack(0x200);
#ifdef DEBUG_1
		printf("Clock process stack pointer is 0x%x.\n", sp);
#endif
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)(&clock_proc); // PC contains the entry point of the process
	for ( i = 0; i < 6; i++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[PID_CLOCK])->mp_sp = sp;
	enqueuePriority(PCBReadyQueue, gp_pcbs[PID_CLOCK]);

	/*Initialize CRT process*/
	(gp_pcbs[PID_KCD])->m_pid = PID_KCD;
	(gp_pcbs[PID_KCD])->m_state = NEW;
	(gp_pcbs[PID_KCD])->m_priority = 0;
	(gp_pcbs[PID_KCD])->mp_next = NULL;
	(gp_pcbs[PID_KCD])->msg_q = NULL;
	sp = alloc_stack(0x200);
#ifdef DEBUG_1
		printf("KCD process stack pointer is 0x%x.\n", sp);
#endif
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)(&kcd_proc); // PC contains the entry point of the process
	for ( i = 0; i < 6; i++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[PID_KCD])->mp_sp = sp;
	enqueuePriority(PCBReadyQueue, gp_pcbs[PID_KCD]);
	
	/*Initialize CRT Process*/
	(gp_pcbs[PID_CRT])->m_pid = PID_CRT;
	(gp_pcbs[PID_CRT])->m_state = NEW;
	(gp_pcbs[PID_CRT])->m_priority = 0;
	(gp_pcbs[PID_CRT])->mp_next = NULL;
	(gp_pcbs[PID_CRT])->msg_q = NULL;
	
	sp = alloc_stack(0x200);
#ifdef DEBUG_1
		printf("CRT process stack pointer is 0x%x.\n", sp);
#endif
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)(&crt_proc); // PC contains the entry point of the process
	for ( i = 0; i < 6; i++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[PID_CRT])->mp_sp = sp;
	enqueuePriority(PCBReadyQueue, gp_pcbs[PID_CRT]);
	
	/*Initialize NULL Process*/
	(gp_pcbs[PID_NULL])->m_pid = 0;
	(gp_pcbs[PID_NULL])->m_state = NEW;
	(gp_pcbs[PID_NULL])->m_priority = NUM_PRIORITIES - 1;
	(gp_pcbs[PID_NULL])->mp_next = NULL;
	(gp_pcbs[PID_NULL])->msg_q = NULL;
	
	sp = alloc_stack(0x100);
#ifdef DEBUG_1
		printf("Null process stack pointer is 0x%x.\n", sp);
#endif
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)(&null_proc); // PC contains the entry point of the process
	for ( i = 0; i < 6; i++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[PID_NULL])->mp_sp = sp;
	enqueuePriority(PCBReadyQueue, gp_pcbs[PID_NULL]);
	
	/*Initialize Priority Process*/
	(gp_pcbs[PID_SET_PRIO])->m_pid = PID_SET_PRIO;
	(gp_pcbs[PID_SET_PRIO])->m_state = NEW;
	(gp_pcbs[PID_SET_PRIO])->m_priority = 0;
	(gp_pcbs[PID_SET_PRIO])->mp_next = NULL;
	(gp_pcbs[PID_SET_PRIO])->msg_q = NULL;
	
	sp = alloc_stack(0x200);
#ifdef DEBUG_1
		printf("Null process stack pointer is 0x%x.\n", sp);
#endif
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)(&priority_proc); // PC contains the entry point of the process
	for ( i = 0; i < 6; i++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[PID_SET_PRIO])->mp_sp = sp;
	enqueuePriority(PCBReadyQueue, gp_pcbs[PID_SET_PRIO]);
	
	/*Initialize STRESS_A Process*/
	(gp_pcbs[PID_A])->m_pid = PID_A;
	(gp_pcbs[PID_A])->m_state = NEW;
	(gp_pcbs[PID_A])->m_priority = 0;
	(gp_pcbs[PID_A])->mp_next = NULL;
	(gp_pcbs[PID_A])->msg_q = NULL;
	
	sp = alloc_stack(0x200);
#ifdef DEBUG_1
		printf("STRESS_A process stack pointer is 0x%x.\n", sp);
#endif
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)(&proc_A); // PC contains the entry point of the process
	for ( i = 0; i < 6; i++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[PID_A])->mp_sp = sp;
	enqueuePriority(PCBReadyQueue, gp_pcbs[PID_A]);
	
		/*Initialize STRESS_B Process*/
	(gp_pcbs[PID_B])->m_pid = PID_B;
	(gp_pcbs[PID_B])->m_state = NEW;
	(gp_pcbs[PID_B])->m_priority = 0;
	(gp_pcbs[PID_B])->mp_next = NULL;
	(gp_pcbs[PID_B])->msg_q = NULL;
	
	sp = alloc_stack(0x100);
#ifdef DEBUG_1
		printf("STRESS_B process stack pointer is 0x%x.\n", sp);
#endif
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)(&proc_B); // PC contains the entry point of the process
	for ( i = 0; i < 6; i++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[PID_B])->mp_sp = sp;
	enqueuePriority(PCBReadyQueue, gp_pcbs[PID_B]);
	
		/*Initialize STRESS_C Process*/
	(gp_pcbs[PID_C])->m_pid = PID_C;
	(gp_pcbs[PID_C])->m_state = NEW;
	(gp_pcbs[PID_C])->m_priority = 0;
	(gp_pcbs[PID_C])->mp_next = NULL;
	(gp_pcbs[PID_C])->msg_q = NULL;
	
	sp = alloc_stack(0x200);
#ifdef DEBUG_1
		printf("STRESS_C process stack pointer is 0x%x.\n", sp);
#endif
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)(&proc_C); // PC contains the entry point of the process
	for ( i = 0; i < 6; i++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[PID_C])->mp_sp = sp;
	enqueuePriority(PCBReadyQueue, gp_pcbs[PID_C]);
			
	/*Initialize TIMER0_IRQHANDLER Process*/
	(gp_pcbs[PID_TIMER_IPROC])->m_pid = PID_TIMER_IPROC;
	(gp_pcbs[PID_TIMER_IPROC])->m_state = NEW;
	(gp_pcbs[PID_TIMER_IPROC])->m_priority = 3;
	(gp_pcbs[PID_TIMER_IPROC])->mp_next = NULL;
	(gp_pcbs[PID_TIMER_IPROC])->msg_q = NULL;
	
	sp = alloc_stack(0x100);
#ifdef DEBUG_1
		printf("STRESS_C process stack pointer is 0x%x.\n", sp);
#endif
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)(&c_TIMER0_IRQHandler); // PC contains the entry point of the process
	for ( i = 0; i < 6; i++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[PID_TIMER_IPROC])->mp_sp = sp;
	
	/*Initialize UART0_IRQHANDLER Process*/
	(gp_pcbs[PID_UART_IPROC])->m_pid = PID_UART_IPROC;
	(gp_pcbs[PID_UART_IPROC])->m_state = NEW;
	(gp_pcbs[PID_UART_IPROC])->m_priority = 0;
	(gp_pcbs[PID_UART_IPROC])->mp_next = NULL;
	(gp_pcbs[PID_UART_IPROC])->msg_q = NULL;
	
	sp = alloc_stack(0x100);
#ifdef DEBUG_1
		printf("STRESS_C process stack pointer is 0x%x.\n", sp);
#endif
	*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
	*(--sp)  = (U32)(&c_UART0_IRQHandler); // PC contains the entry point of the process
	for ( i = 0; i < 6; i++ ) { // R0-R3, R12 are cleared with 0
		*(--sp) = 0x0;
	}
	(gp_pcbs[PID_UART_IPROC])->mp_sp = sp;
	
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */

PCB *scheduler(void)
{
	int highestPriority = peekPriority(PCBReadyQueue);
	if (gp_current_process != NULL && highestPriority > gp_current_process->m_priority && gp_current_process->m_state != BLOCKED && gp_current_process->m_state!=BLOCKED_ON_RECEIVE) { 
		return gp_current_process;
	} else {
		gp_current_process = dequeuePriority(PCBReadyQueue);
	}
	// Returns NULL if no nodes in Ready Queue
	return gp_current_process;
}

/*@brief: switch out old pcb (p_pcb_old), run the new pcb (gp_current_process)
 *@param: p_pcb_old, the old pcb that was in RUN
 *@return: RTX_OK upon success
 *         RTX_ERR upon failure
 *PRE:  p_pcb_old and gp_current_process are pointing to valid PCBs.
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
int process_switch(PCB *p_pcb_old) 
{
	PROC_STATE_E state;
	PROC_STATE_E old_state;
	state = gp_current_process->m_state;
	old_state = p_pcb_old->m_state;
	if (state == NEW) {
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW) {
			if (old_state != BLOCKED && old_state != BLOCKED_ON_RECEIVE) {
				p_pcb_old->m_state = RDY;
			}
			p_pcb_old->mp_sp = (U32 *) __get_MSP();
			if( ! isInQueuePriority(PCBBlockedQueue, p_pcb_old) && !isInQueuePriority(PCBReadyQueue, p_pcb_old) && p_pcb_old->m_state != BLOCKED_ON_RECEIVE) {
				enqueuePriority( PCBReadyQueue, p_pcb_old );
			}
		}
		gp_current_process->m_state = RUN;
#ifdef DEBUG_1
		//printf("MSP is being set to 0x%d.\n", gp_current_process->mp_sp);
#endif
		__set_MSP((U32) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for a new processes
	} 
	
	/* The following will only execute if the if block above is FALSE */

	if (gp_current_process != p_pcb_old) {
		if (state == RDY){ 		
			if (old_state != BLOCKED && old_state != BLOCKED_ON_RECEIVE && old_state != NEW) {
				p_pcb_old->m_state = RDY;
			}
			p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
			if( ! isInQueuePriority(PCBBlockedQueue, p_pcb_old) && !isInQueuePriority(PCBReadyQueue, p_pcb_old) && old_state != BLOCKED_ON_RECEIVE) {
				enqueuePriority( PCBReadyQueue, p_pcb_old );
			}
			gp_current_process->m_state = RUN;
#ifdef DEBUG_1
		//printf("MSP is being set to 0x%d.\n", gp_current_process->mp_sp);
#endif
			__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack    
		} else {
			gp_current_process = p_pcb_old; // revert back to the old proc on error
			return RTX_ERR;
		} 
	}
	
	if (gp_current_process == p_pcb_old) {
		gp_current_process->m_state = RUN;
	}
	
	return RTX_OK;
}
/**
 * @brief release_processor(). 
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_process gets updated to next to run process
 */
int k_release_processor(void)
{
	PCB *p_pcb_old = NULL;
	
	p_pcb_old = gp_current_process;
	gp_current_process = scheduler();
	
	if ( gp_current_process == NULL  ) {
		gp_current_process = p_pcb_old; // revert back to the old process
		return RTX_ERR;
	}
   if ( p_pcb_old == NULL ) {
		p_pcb_old = gp_current_process;
	}

	return process_switch(p_pcb_old);
}

void handle_process_ready(PCB* process) {
	//PCB *p_pcb_old = gp_current_process;
	//gp_current_process = process;
	//process_switch(p_pcb_old);
	if( ! isInQueuePriority(PCBBlockedQueue, process) && !isInQueuePriority(PCBReadyQueue, process)) {
		enqueuePriority(PCBReadyQueue, process);
	}
	k_release_processor();
}

int k_set_process_priority(int process_id, int priority){
	//gp_pcbs
	int i;
	int queueCode = -1;
	if (process_id == 0 || priority >= 4 || priority < 0){
		return RTX_ERR;
	}
	
	for(i = 0; i < 16 /*NUM_TOTAL_PROCS*/; i++) {
		if(gp_pcbs[i]->m_pid == process_id) {
			// retCode = 0 for Ready ; 1 for Blocked ; -1 for fail
			if (isInQueuePriority(PCBReadyQueue, gp_pcbs[i])) {
				rmvFromPriorityQueue( PCBReadyQueue, gp_pcbs[i] );
				queueCode = 0;
			}
			if (isInQueuePriority(PCBBlockedQueue, gp_pcbs[i])) {
				rmvFromPriorityQueue( PCBBlockedQueue, gp_pcbs[i] );
				queueCode = 1;
			}
			
			gp_pcbs[i]->m_priority = priority;
			
			if( queueCode == 0 ) {
				if ( gp_pcbs[i]->m_state != NEW) {
					gp_pcbs[i]->m_state = RDY;
				}
				if( ! isInQueuePriority(PCBBlockedQueue, gp_pcbs[i]) && !isInQueuePriority(PCBReadyQueue, gp_pcbs[i]) && gp_current_process != gp_pcbs[i] && gp_pcbs[i]->m_state != BLOCKED_ON_RECEIVE) {
					enqueuePriority( PCBReadyQueue, gp_pcbs[i] );
				}
			} else if ( queueCode == 1 ) {
				gp_pcbs[i]->m_state = BLOCKED;
				if( ! isInQueuePriority(PCBBlockedQueue, gp_pcbs[i]) && !isInQueuePriority(PCBReadyQueue, gp_pcbs[i]) && gp_current_process != gp_pcbs[i] && gp_pcbs[i]->m_state != BLOCKED_ON_RECEIVE) {
					enqueuePriority( PCBBlockedQueue, gp_pcbs[i] );
				}
			} else if (gp_pcbs[i] == gp_current_process) { 
				if ( gp_pcbs[i]->m_state != NEW && gp_pcbs[i]->m_state != BLOCKED_ON_RECEIVE) {
					gp_pcbs[i]->m_state = RDY;
				}
					if( ! isInQueuePriority(PCBBlockedQueue, gp_pcbs[i]) && !isInQueuePriority(PCBReadyQueue, gp_pcbs[i]) && gp_pcbs[i]->m_state != BLOCKED_ON_RECEIVE) {
						enqueuePriority( PCBReadyQueue, gp_pcbs[i] );
					}
			}	else {
#ifdef DEBUG_1
				assert(0);
#endif /* DEBUG_0 */
			}
			k_release_processor();
			return RTX_OK;
		}
	}
	return RTX_ERR;
}

int k_get_process_priority(int process_id) {
	int i;
	for(i = 0; i < 16/*NUM_TOTAL_PROCS*/; i++) {
		if(gp_pcbs[i]->m_pid == process_id) {
			return gp_pcbs[i]->m_priority;
		}
	}
	return RTX_ERR;
}

int higher_priority_available(void) {
	int highestPriority = peekPriority(PCBReadyQueue);
	return gp_current_process != NULL && highestPriority <= gp_current_process->m_priority;
}
