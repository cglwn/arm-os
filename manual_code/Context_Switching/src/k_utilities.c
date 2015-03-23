#include "k_utilities.h"
#include "k_memory.h"
#include "assert.h"
#include "printf.h"

BOOLEAN interruptsEnabled = true; 
extern MSG_HEADER *pending_delayed_messages;
extern MSG_HEADER *timeout_queue;
extern MSG_HEADER *pending_crt_messages;

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

void enqueue_message_queue(PCB* pcb, MSG_HEADER *msg) {
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

MSG_HEADER* dequeue_message_queue(PCB* pcb ) {
	MSG_HEADER *msg_queue = pcb->msg_q;
	if (msg_queue == NULL) {
			return NULL; 
	}
	pcb->msg_q = msg_queue->next;
	return msg_queue;
}

void enqueue_pending_queue(MSG_HEADER *msg) {
	if(pending_delayed_messages == NULL) {
		pending_delayed_messages = msg;
	} else {
		MSG_HEADER *header = pending_delayed_messages;
		while (header->next != NULL) {
			header = header->next;
		}
		header->next = msg;
	}
}

void enqueue_timeout_queue(MSG_HEADER *msg) {
	MSG_HEADER *current = timeout_queue;
	if(timeout_queue == NULL) {
		timeout_queue = msg;
	} else if (timeout_queue->next == NULL && msg->expiry < timeout_queue->expiry){
		msg->next = timeout_queue;
		timeout_queue = msg;
	} else {
		MSG_HEADER *next_msg = timeout_queue->next;
		while (next_msg != NULL && next_msg->expiry < msg->expiry) {
			current = current->next;
			next_msg = next_msg->next;
		}
		current->next = msg;
		msg->next = next_msg;
	}
}

void enqueue_crt_queue(MSG_HEADER *msg) {
	if(pending_crt_messages == NULL) {
		pending_crt_messages = msg;
	} else {
		MSG_HEADER *header = pending_crt_messages;
		while (header->next != NULL) {
			header = header->next;
		}
		header->next = msg;
	}
}

MSG_HEADER* dequeue_crt_queue() {
	MSG_HEADER *head = pending_crt_messages;
	if (pending_crt_messages != NULL) {
		pending_crt_messages = head->next;
	}
	if( head ) {
		head->next = NULL;
	}
	return head;
}

MSG_HEADER* dequeue_pending_queue() {
	MSG_HEADER *head = pending_delayed_messages;
	if (pending_delayed_messages != NULL) {
		pending_delayed_messages = head->next;
	}
	if( head ) {
		head->next = NULL;
	}
	return head;
}

MSG_HEADER* dequeue_timeout_queue() {
	MSG_HEADER *head = timeout_queue;
	if (timeout_queue != NULL) {
		timeout_queue = head->next;
	}
	if( head ) {
		head->next = NULL;
	}
	return head;
}

void enable_interrupts( BOOLEAN n_enable )
{
	if( n_enable == true  && interruptsEnabled == false ) {
#ifdef DEBUG_0
			//printf("Enabling interrupts\n");
#endif // DEBUG_0
		interruptsEnabled = true;
		__enable_irq();
	} 
	
	if ( n_enable == false  && interruptsEnabled == true) {
#ifdef DEBUG_0
			//printf("Disabling interrupts\n");
#endif // DEBUG_0
		interruptsEnabled = false;
		__disable_irq();
	}
}

PCB* get_process(PCB **pcbs, int pid) { 
	int i;
	for( i = 0; i < 10/*NUM_TOTAL_PROCS*/; i++ ) {
		if(pcbs[i]->m_pid == pid) {
			return pcbs[i];
		}
	}
	return NULL;
}


void print_priority_queue(PCB**  pcbQueue) {
	int i;
	for (i = 0; i < NUM_PRIORITIES; i++) {
		print_queue(pcbQueue[i]);
	}
}

void print_queue(PCB *pcbQueue) {
	PCB *current = pcbQueue;
	MSG_BUF *msg;
	while (current != NULL) {
		msg = (MSG_BUF *)k_request_memory_block_nb();
		sprintf(msg->mtext, "%1d", current->m_pid);
		k_send_message_nb(PID_CRT, msg);
		current = current->mp_next;
	}
	msg = (MSG_BUF *)k_request_memory_block_nb();
	msg->mtext[0] = '\n';
	k_send_message_nb(PID_CRT, msg);
}

void string_copy(char *destination, char *source, int length) {
	int i;
	for (i = 0; i < length; i++)
	*destination++ = *source++;
}
