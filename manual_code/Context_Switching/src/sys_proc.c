#include "k_rtx.h"
#include "k_utilities.h"
#include "message.h"
#include "sys_proc.h"
#include "k_memory.h"
#include "k_process.h"


void kcd_proc(void) {
	char command[10]; //increase stack size if this goes up
	int command_length = 0;
	int registered_procs[2];
	char proc_commands[2];
	int num_commands_registered = 0;
	int *sender_id;
	
	while (1) {
		MSG_BUF *msg = (MSG_BUF *) receive_message(sender_id);
		if (msg->mtype == DEFAULT) {
			char text = msg->mtext[0];
			send_message(PID_CRT, msg);
			
			//start of a command
			if (command_length == 0 && text == '%') {
				command[command_length++] = text;
			} else	if (command_length > 0 && text != '\r' && text != '\0') {
				//append to a command
				command[command_length++] = text;
			} else if (command_length > 0 && text == '\r') {
				int i;
				char command_char = command[1];
				for (i = 0; i < num_commands_registered; i++) {
					if (proc_commands[i] == command_char) {
						msg = request_memory_block();
						string_copy(msg->mtext, command, command_length);
						send_message(registered_procs[i], msg);
					}
				}
				command_length = 0;
			}
		} else if (msg->mtype == KCD_REG) {
			registered_procs[num_commands_registered] = *sender_id;
			proc_commands[num_commands_registered] = msg->mtext[0];
			num_commands_registered++;
			release_memory_block(msg);
		}
	}
}


void crt_proc(void) {
	while(1) {
		MSG_BUF *msg;
		MSG_HEADER* header = (MSG_HEADER *) request_memory_block();
		msg = receive_message(NULL);
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

