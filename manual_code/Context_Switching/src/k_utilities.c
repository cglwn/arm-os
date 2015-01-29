#include "k_utilities.h"
#include "assert.h"
PCB* dequeue( PCB *pcbQueue ) 
{
	PCB* tempPCB = pcbQueue;
	assert( pcbQueue );
	pcbQueue = tempPCB->mp_next;
	return tempPCB;
}

void enqueue ( PCB* pcbQueue, PCB* pcbNode )
{
	PCB* tempPCB = pcbQueue;
	assert( pcbQueue && pcbNode );
	while( tempPCB->mp_next != NULL ) {
		tempPCB = tempPCB->mp_next;
	}
	tempPCB->mp_next = pcbNode;
}

PCB* dequeuePriority( void*  voidQueue )
{
	PCB** pcbQueue = (PCB**) voidQueue;
	int i;
	for( i = 0; i < NUM_PRIORITIES; i++ ) {
		if( pcbQueue[i]) {
			return dequeue( pcbQueue[i] );
		}
	}
	return NULL;
}

void enqueuePriority ( PCB**  pcbQueue, PCB* pcbNode )
{
	enqueue( pcbQueue[ pcbNode->m_priority ], pcbNode );
}
