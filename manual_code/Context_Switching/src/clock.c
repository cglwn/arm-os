#include "rtx.h"
#include "message.h"
#include "printf.h"
#include "clock.h"
#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

void clock_proc(void) {
	MSG_BUF *msg;
	char s[9];
	int i;
	int *sender_id;
	int clock_running = 1;
	int time = 0;
	sender_id = request_memory_block();
	msg = (MSG_BUF *) request_memory_block();
	msg->mtype = KCD_REG;
	msg->mtext[0] = 'W';
	send_message(PID_KCD, msg);
	
	msg = (MSG_BUF *) request_memory_block();
	while(1) {
		MSG_BUF *time_msg;
		delayed_send(PID_CLOCK, msg, 1000);
		msg = receive_message(sender_id);
		if (/* *sender_id == PID_CLOCK &&*/ clock_running == 1) {
			sprintf(s, "%02d:%02d:%02d\n", (time/60/60) % 24, (time/60)%60, (time %60));
			for (i = 0; i < 9; i++) {
				time_msg = (MSG_BUF *) request_memory_block();
				time_msg->mtext[0] = s[i];
				send_message(PID_CRT, time_msg);
			}
			time++;
		}
	}
}
