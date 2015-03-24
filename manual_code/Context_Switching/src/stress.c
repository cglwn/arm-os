#include "rtx.h"
//#include "k_utilities.h"
#include "message.h"
#include "sys_proc.h"
//#include "k_memory.h"
//#include "k_process.h"
#include "stress.h"

// Stress Test Processes

void proc_A(void) {
	int count = 0;
	int* sender_id;
	MSG_BUF* msg;
	
	// This line will need to be released somewhere ?
	sender_id = request_memory_block();

	// NOT SURE IF THIS IS HOW YOU REGISTER:
	msg = (MSG_BUF *) request_memory_block();
	msg->mtype = KCD_REG;
	msg->mtext[0] = 'Z';
	send_message(PID_KCD, msg);
	
	while( 1 ) {
			MSG_BUF *msg = (MSG_BUF *) receive_message(sender_id);	
			// NOT SURE IF THIS IS HOW ITS SET UP TO CHECK FOR THE %Z KCD COMMAND
			if( msg && *sender_id == PID_KCD ) {
				release_memory_block( msg );
				break;
			} else {
				release_memory_block( msg );
			}
	}
	
	while( 1 ) {
		MSG_BUF *msg = (MSG_BUF *) request_memory_block();
		msg->mtype = COUNT_REPORT;
		msg->mtext[0] = count;
		send_message( PID_B, msg );
		count++;
		release_processor();
	}
	
}

void proc_B(void) {
	while(1) {
		MSG_BUF *msg = (MSG_BUF *) receive_message(NULL);	
		send_message(PID_C, msg);
	}
}

// NOTE : For the local msg queue I used the msg_header style way of queue'ing
// wasn't sure if I was to use non-blocking or blocking so I used non-blocking

void proc_C(void) {
	MSG_HEADER *local_msg_queue = NULL;

	while( 1 ) {
		MSG_BUF* msg = NULL;
		
		if( local_msg_queue == NULL ) {
			msg = receive_message(NULL);
		} else { 
			// Dequeue next in local messages
			MSG_HEADER *msgHeader = local_msg_queue;
			msg = msgHeader->msg_env;
			local_msg_queue = local_msg_queue->next;
			release_memory_block( msgHeader ) ;
		}
		
		
		if( msg->mtype == COUNT_REPORT ) {
			int report_num = msg->mtext[0];
			
			// DO WE COUNT 0 % 20 == 0 ??
			if( report_num % 20 == 0 ) {
				char *output = "Process C";
				int i = 0;
				for (i = 0; i < 9 /*strlen(output)*/ ; i++) {
					// Assumption: first time "msg" is sent it should already have memory
					// therefore only after the first iteration does it need to request memory
					// aka we're recycling the report_num msg
					if( i != 0 ) {
						msg = (MSG_BUF *) request_memory_block();
					}
					
					msg->mtext[0] = output[i];
					send_message(PID_CRT, msg);
				}
				// Hibernate for 10 seconds
				{
					MSG_BUF	*delay_msg = (MSG_BUF *) request_memory_block();
					delay_msg->mtype = WAKE_UP_10;
					delayed_send( PID_C, delay_msg, 10000 );
					while( 1 ) {
						msg = receive_message(NULL);
						if( msg->mtype == WAKE_UP_10 ) {
							break;
						} else {
							// Enqueue the msg for later processing
								if(local_msg_queue == NULL) {
									local_msg_queue = (MSG_HEADER *)request_memory_block();
									local_msg_queue->msg_env = msg;
									local_msg_queue->next = NULL;
								} else {
									MSG_HEADER *header = local_msg_queue;
									MSG_HEADER *newHeader = (MSG_HEADER *)request_memory_block();
									while (header->next != NULL) {
										header = header->next;
									}
									header->next = newHeader;
									newHeader->msg_env = msg;
									newHeader->next = NULL;
								}
						}
					}
				}
				
			}
		}
		release_memory_block( msg );
		release_processor();
	}
}
