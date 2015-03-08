#ifndef K_UTIL_H_
#define K_UTIL_H_

#include "k_rtx.h"

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
#endif // K_UTIL_H_
