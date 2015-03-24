#include "rtx.h"
#include "message.h"
#include "priority.h"
#include "printf.h"
#ifdef DEBUG_0
#endif /* DEBUG_0 */

void priority_proc(void) {
	MSG_BUF *msg;
	char s[9];
	msg = (MSG_BUF *) request_memory_block();
	msg->mtype = KCD_REG;
	msg->mtext[0] = 'C';
	send_message(PID_KCD, msg);
	while(1) {
		char *ch = NULL;
		int pid;
		int priority;
		msg = receive_message(NULL);
		//%C pid priority
		//0123  45
		ch = msg->mtext + 3;
		pid = *ch - 30;
		ch = msg->mtext + 5;
		priority = *ch - 30;
		set_process_priority(pid, priority);
	}
}
