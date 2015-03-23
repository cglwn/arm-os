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
	int time = 0;
	msg = (MSG_BUF *) request_memory_block();
	while(1) {
		//crt to uart0 to free
		delayed_send(PID_CLOCK, msg, 1000);
		msg = receive_message(NULL);
		sprintf(s, "%02d:%02d:%02d\n", (time/60/60) % 24, (time/60)%60, (time %60));
		for (i = 0; i < 9; i++) {
			msg = (MSG_BUF *) request_memory_block();
			msg->mtext[0] = s[i];
			send_message(PID_CRT, msg);
		}
		time++;
	}
}
