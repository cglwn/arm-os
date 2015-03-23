#ifndef K_UTIL_H_
#define K_UTIL_H_

#include "k_rtx.h"
#include "message.h"
/* ----- Functions ----- */

//PCB* dequeue( PCB* pcbQueue );
//void enqueue ( PCB* pcbQueue, PCB* pcbNode );

PCB* dequeuePriority( PCB**  voidQueue );
void enqueuePriority ( PCB*  pcbQueue[], PCB* pcbNode );
int rmvFromPriorityQueue(PCB**  pcbQueue, PCB* pcbNode );
int peekPriority( PCB** pcbQueue);
BOOLEAN isInQueuePriority(PCB** pcbQueue, PCB* pcb);
void enqueue_message_queue( PCB* pcb, MSG_HEADER *msg );
MSG_HEADER* dequeue_message_queue( PCB* pcb );
void enable_interrupts( BOOLEAN n_enable );
PCB* get_process(PCB **pcb, int pid);
void enqueue_pending_queue(MSG_HEADER *msg);
void enqueue_timeout_queue(MSG_HEADER *msg);
void enqueue_crt_queue(MSG_HEADER *msg);
MSG_HEADER* dequeue_pending_queue(void);
MSG_HEADER* dequeue_crt_queue(void);
MSG_HEADER* dequeue_timeout_queue(void);
void print_priority_queue(PCB**  pcbQueue);
void print_queue(PCB *pcbQueue);
void string_copy(char *destination, char *source, int length);
#endif // K_UTIL_H_
