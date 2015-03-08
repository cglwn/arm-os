#include "k_utilities.h"
#include "k_memory.h"
#include "assert.h"
#include "printf.h"

BOOLEAN interruptsEnabled = true; 

PCB* dequeue( PCB* pcbQueue[], int priority ) 
{
	PCB* tempPCB = pcbQueue[priority];
	assert( pcbQueue[priority] );
	pcbQueue[priority] = tempPCB->mp_next;
	tempPCB->mp_next = NULL;
	return tempPCB;
}

void enqueue ( PCB** pcbQueue, int priority, PCB* pcbNode )
{
	PCB* tempPCB;
	if(pcbQueue[priority] == NULL) {
		pcbQueue[priority] = pcbNode;
	} else {
		tempPCB = pcbQueue[priority];
		assert( pcbQueue[priority] && pcbNode );
		while( tempPCB->mp_next != NULL ) {
			tempPCB = tempPCB->mp_next;
			if(tempPCB == pcbNode) {
				return;
			}
		}
		tempPCB->mp_next = pcbNode;
	}
}

PCB* dequeuePriority( PCB*  pcbQueue[] )
{
	int i;
	for( i = 0; i < NUM_PRIORITIES; i++ ) {
		if( pcbQueue[i]) {
			return dequeue( pcbQueue, i );
		}
	}
	return NULL;
}

void enqueuePriority ( PCB*  pcbQueue[], PCB* pcbNode )
{
	enqueue( pcbQueue, pcbNode->m_priority,  pcbNode );
}

int rmvFromPriorityQueue(PCB**  pcbQueue, PCB* pcbNode )
{
	PCB* tempPCB = pcbQueue[ pcbNode->m_priority ];
	if( tempPCB == pcbNode ) {
		dequeue( pcbQueue, pcbNode->m_priority);
		return 0;
	} else if (tempPCB) {
		while(tempPCB->mp_next != pcbNode) {
			tempPCB = tempPCB->mp_next;
		}
		if( tempPCB ) {
			tempPCB->mp_next = pcbNode->mp_next;
			pcbNode->mp_next = NULL;
			return 0;
		}
	}
	return 0;
}

int peekPriority( PCB** pcbQueue) {
	int i;
	for( i = 0; i < NUM_PRIORITIES; i++ ) {
		if( pcbQueue[i]) {
			return i;
		}
	}
	return NUM_PRIORITIES + 1;
}

BOOLEAN isInQueue(PCB** pcbQueue, int priority, PCB* pcb) {
	PCB* tempPCB = pcbQueue[priority];
// 	if(tempPCB) {
// 		if (tempPCB == pcb) { 
// 			return true;
// 		}
// 		while(tempPCB->mp_next != NULL) {
// 			tempPCB = tempPCB->mp_next;			
// 			if (tempPCB == pcb) { 
// 				return true;
// 			}
// 		}
// 	}
	while( tempPCB != NULL ){
		if (tempPCB == pcb) { 
			return true;
		}
		tempPCB = tempPCB->mp_next;	
	} 
	
	
	return false;
}

BOOLEAN isInQueuePriority(PCB** pcbQueue, PCB* pcb) {
	int i;
	BOOLEAN inQueue = false;
	for(i = 0; i < NUM_PRIORITIES; i++) { 
		inQueue |= isInQueue(pcbQueue, i, pcb);
	}
	return inQueue;
}

void enqueue_message_queue( PCB* pcb, MSG_HEADER *msg ) {
	MSG_HEADER *tail = pcb->msg_q;
	if ( tail == NULL ) {
		pcb->msg_q = msg;
		return;
	}
	
	while(tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = msg;
}

MSG_HEADER* dequeue_message_queue( PCB* pcb ) {
	MSG_HEADER *msg_queue = pcb->msg_q;
	if (msg_queue == NULL) {
			return NULL; 
	}
	pcb->msg_q = msg_queue->next;
	return msg_queue;
}

void enable_interrupts( BOOLEAN n_enable )
{
	if( nEnable == true  && interruptsEnabled == false ) {
		interruptsEnabled = true;
		__enable_irq();
	} 
	
	if ( nEnable == false  && interruptsEnabled == true) {
		interruptsEnabled = false;
		__disable_irq();
	}
}
