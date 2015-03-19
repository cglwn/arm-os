#include "k_rtx.h"
#include "k_utilities.h"
#include "message.h"
#include "sys_proc.h"
#include "k_memory.h"
#include "k_process.h"

/*
void kcd_proc(void) {
	MSG_BUF *msg = k_receive_message(NULL);
	
}
*/

void crt_proc(void) {
	while(1) {
		MSG_BUF *msg;
		MSG_HEADER* header = (MSG_HEADER *) request_memory_block();
		msg= receive_message(NULL);
		header->msg_env = msg;
		header->next = NULL;
		enqueue_crt_queue(header);
	}
}

void null_proc(void) { 
	while(1) {
		release_processor();
	}
}
