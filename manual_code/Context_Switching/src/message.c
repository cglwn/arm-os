#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "k_rtx.h"
#include "k_utilities.h"
#include "k_memory.h"
#include "k_process.h"

#ifdef DEBUG_0
#include "printf.h"
#endif

extern uint32_t g_timer_count;

int k_send_message(int process_id, void *message_envelope) {
	MSG_HEADER *header;
	PCB* dest_pcb;
#ifdef DEBUG_1
	printf("Process_Id: %d\nMessage Envelope: %d", process_id, (int)message_envelope); 
#endif /*DEBUG_0*/
	dest_pcb = get_process(gp_pcbs, process_id);
	enable_interrupts(false);
	//initialize and set header
	header = (MSG_HEADER *)k_request_memory_block();
	header->source_pid = (U32) gp_current_process->m_pid;
	header->dest_pid = (U32) process_id;
	header->msg_env = (MSG_BUF*) message_envelope;
	header->next=NULL;
	
	//put message in the message queue
	enqueue_message_queue(dest_pcb, header);
	if (dest_pcb->m_state ==  BLOCKED_ON_RECEIVE) {
		dest_pcb->m_state = RDY;
		enqueuePriority(PCBReadyQueue, dest_pcb);
		if (dest_pcb->m_priority < gp_current_process->m_priority) {
			enable_interrupts(true);
			k_release_processor();
		}
	}
	enable_interrupts(true);
	return RTX_OK; //TODO: check what this should return
}


int k_send_message_nb(int process_id, void *message_envelope) {
	MSG_HEADER *header;
	PCB* dest_pcb;
#ifdef DEBUG_1
	printf("Process_Id: %d\nMessage Envelope: %d", process_id, (int)message_envelope); 
#endif /*DEBUG_0*/
	dest_pcb = get_process(gp_pcbs, process_id);
	//initialize and set header
	header = (MSG_HEADER *)k_request_memory_block();
	header->source_pid = (U32) gp_current_process->m_pid;
	header->dest_pid = (U32) process_id;
	header->msg_env = (MSG_BUF*) message_envelope;
	header->next=NULL;

	enqueue_message_queue(dest_pcb, header);
	if (dest_pcb->m_state ==  BLOCKED_ON_RECEIVE) {
		dest_pcb->m_state = RDY;
		enqueuePriority(PCBReadyQueue, dest_pcb);
	}
	
	return RTX_OK; //TODO: check what this should return
}

void *k_receive_message(int *sender_id) {
	MSG_HEADER *msg_envelope;
	MSG_BUF *msg;
	enable_interrupts(false);
	while(gp_current_process->msg_q == NULL) {
		gp_current_process->m_state = BLOCKED_ON_RECEIVE;
		enable_interrupts(true);
		k_release_processor();
	}
	enable_interrupts(false);
	msg_envelope = dequeue_message_queue(gp_current_process);
	msg = msg_envelope->msg_env;
	k_release_memory_block(msg_envelope);
	enable_interrupts(true);
	return msg;
}

void *nb_receive_message(int *sender_id) {
	MSG_HEADER *msg_envelope;
	MSG_BUF *msg = NULL;
	msg_envelope = dequeue_message_queue(gp_current_process);
	if (msg_envelope ){ 
		msg = msg_envelope->msg_env;
		k_release_memory_block(msg_envelope);
	}
	return msg;
}

int k_delayed_send(int process_id, void *message_envelope, int delay) {
	MSG_HEADER *header;
	header = (MSG_HEADER *)k_request_memory_block();
	header->source_pid = (U32) gp_current_process->m_pid;
	header->dest_pid = (U32) process_id;
	header->msg_env = (MSG_BUF*) message_envelope;
	header->expiry = g_timer_count + delay;
	header->next = NULL;
	enqueue_pending_queue(header);
	return RTX_OK;
}
