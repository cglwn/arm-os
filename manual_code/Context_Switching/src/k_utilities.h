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
enqueue_message_queue( PCB* pcb, msg_header *msg );
dequeue_message_queue( PCB* pcb );
#endif // K_UTIL_H_
