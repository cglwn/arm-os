/** 
 * @file:   k_rtx.h
 * @brief:  kernel deinitiation and data structure header file
 * @auther: Yiqing Huang
 * @date:   2014/01/17
 */
 
#ifndef K_RTX_H_
#define K_RTX_H_

/*----- Definitations -----*/

#define RTX_ERR -1
#define RTX_OK  0

#define NULL 0
#define NUM_TEST_PROCS 6

#define PID_NULL 0
#define PID_A    7
#define PID_B    8
#define PID_C    9
#define PID_SET_PRIO     10
#define PID_CLOCK        11
#define PID_KCD          12
#define PID_CRT          13
#define PID_TIMER_IPROC  14
#define PID_UART_IPROC   15


#ifdef DEBUG_0
#define USR_SZ_STACK 0x200         /* user proc stack size 512B   */
#else
#define USR_SZ_STACK 0x100         /* user proc stack size 218B  */
#endif /* DEBUG_0 */

#define BOOLEAN unsigned char
#define true 1
#define false 0

#define NUM_PRIORITIES 5

#define DEFAULT 0
#define KCD_REG 1
 /*----- Types -----*/
typedef unsigned char U8;
typedef unsigned int U32;

/* process states, note we only assume three states in this example */
typedef enum {NEW = 0, RDY, RUN, BLOCKED, INTERRUPTED, BLOCKED_ON_RECEIVE} PROC_STATE_E;  

/*
  PCB data structure definition.
  You may want to add your own member variables
  in order to finish P1 and the entire project 
*/
typedef struct pcb
{ 
	struct pcb *mp_next;  /* next pcb, not used in this example */  
	U32 *mp_sp;				/* stack pointer of the process */
	U32 m_pid;				/* process id */
	U32 m_priority;  		/* initial priority */
	PROC_STATE_E m_state;   /* state of the process */      
	struct msg_header *msg_q; /*the message queue for this process*/
} PCB;

/* initialization table item */
typedef struct proc_init
{	
	int m_pid;	        /* process id */ 
	int m_priority;         /* initial priority, not used in this example. */ 
	int m_stack_size;       /* size of stack in words */
	void (*mpf_start_pc) ();/* entry point of the process */    
} PROC_INIT;


typedef PCB* PCBPriority;

/*----- Globals -----*/

extern PCB* PCBReadyQueue[5];
extern PCB* PCBBlockedQueue[5];

//PCBPriority PCBReadyQueue[5];
//PCBPriority PCBBlockedQueue[5];


#endif // ! K_RTX_H_
