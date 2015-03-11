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
		MSG_HEADER* header;
		msg= k_receive_message(NULL);
		header = (MSG_HEADER *) k_request_memory_block();
		header->msg_env = msg;
		header->next = NULL;
		enqueue_crt_queue(header);
	}
}

void null_proc(void) { 
	while(1) {
		k_release_processor();
	}
}

void temp_proc(void)
{
	int i = 0;
	void *p_mem_blk1;
	void *p_mem_blk2;
	p_mem_blk1 = k_request_memory_block();
	p_mem_blk2 = k_request_memory_block();
	k_release_memory_block(p_mem_blk1);
	k_release_memory_block(p_mem_blk2);
	//uart0_put_string("G015_test: Test 2 OK\r\n"); //Test empty blocked queue on release_memory_block
	k_set_process_priority(1, 4);
	while(1) {
		k_release_processor();
	}
}
