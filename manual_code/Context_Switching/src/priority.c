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
		int loopCount = 0;
		int pid = 0;
		int i;
		int priority = 0;
		char buffer[8] = "aaaaaaaa";
		msg = receive_message(NULL);
		//%C pid priority
		//0123  45
		ch = msg->mtext + 3;
		// Put PID string into the buffer
		while( *ch != ' ' ) {
			buffer[loopCount++] = *(ch++);
		}
		// Transfer buffer into PID int
		
		for( i = 0; i < loopCount; i++ ) {
			pid *= 10;
			pid += buffer[i] - '0';
		}
		
		// Pass by the space
		ch++;
		
		// Reinitialize loop count
		loopCount = 0;		
		
		// Put priority string into the buffer
		
		while( *ch != '\0' ) {
			buffer[loopCount++] = *(ch++);
		}
		
		// Put the buffer into the priority int
		for( i = 0; i < loopCount; i++ ) {
			priority *= 10;
			priority += buffer[i] - '0';
		}
		
		set_process_priority(pid, priority);
		release_memory_block( msg );
	}
}
