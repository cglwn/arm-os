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
	struct msg_header *next_msg; /*the message queue for this process*/
} PCB;

typedef struct msgbuf {
	int mtype; /* user defined message type */
	char mtext[1]; /* body of the message */
} MSGBUF;

typedef struct msg_header {
	struct msg_header *next;
	U32 source_pid;
	U32 dest_pid;
	struct msgbuf *msg_env;
}MSG_HEADER;

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
