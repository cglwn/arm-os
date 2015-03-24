#include "rtx.h"
#include "message.h"
#include "clock.h"
//#ifdef DEBUG_0
#include "printf.h"
//#endif /* DEBUG_0 */

void clock_proc(void) {
	MSG_BUF *msg;
	char s[9];
	int i;
	int tempTime;
	int *sender_id;
	int clock_running = 0;
	int time = 0;
	char *buffer = "aaaaaaaaaaaaaaaa";
	int loopCount = 0;
	char *ch;
	
	sender_id = request_memory_block();
	msg = (MSG_BUF *) request_memory_block();
	msg->mtype = KCD_REG;
	msg->mtext[0] = 'W';
	send_message(PID_KCD, msg);
	
	while(1) {
		MSG_BUF *time_msg;
		msg = receive_message(sender_id);
		
		if( *(msg->mtext + 2) == 'R' && *sender_id == PID_KCD ) {
			time = 0;
			clock_running = 1;
			delayed_send(PID_CLOCK, msg, 1000);
		} else if( *(msg->mtext + 2) == 'S' && *sender_id == PID_KCD ) {
			// %WX XX:XX:XX	
			tempTime = time = 0;
			ch = msg->mtext + 4;
			// Put time string into the buffer
			while( *ch != ':' ) {
				buffer[loopCount++] = *(ch++);
			}
			// Transfer buffer into time int
			
			for( i = 0; i < loopCount; i++ ) {
				tempTime *= 10;
				tempTime += buffer[i] - '0';
			}
			time += tempTime * 3600;
			tempTime = loopCount = 0;
			ch++;
			
			// Put time string into the buffer
			while( *ch != ':' ) {
				buffer[loopCount++] = *(ch++);
			}
			// Transfer buffer into time int
			
			for( i = 0; i < loopCount; i++ ) {
				tempTime *= 10;
				tempTime += buffer[i] - '0';
			}
			time += tempTime * 60;
			tempTime = loopCount = 0;
			ch++;
			
			// Put time string into the buffer
			while( *ch != '\0' ) {
				buffer[loopCount++] = *(ch++);
			}
			// Transfer buffer into time int
			
			for( i = 0; i < loopCount; i++ ) {
				tempTime *= 10;
				tempTime += buffer[i] - '0';
			}
			time += tempTime;
			tempTime = loopCount = 0;
			
			clock_running = 1;
			delayed_send(PID_CLOCK, msg, 1000);
		} else if( *(msg->mtext + 2) == 'T' && *sender_id == PID_KCD ) {
			time = 0;
			clock_running = 0;
			release_memory_block(msg);
		}
		
		if (*sender_id == PID_CLOCK && clock_running == 1) {
			sprintf(s, "%02d:%02d:%02d\n", (time/60/60) % 24, (time/60)%60, (time %60));
			for (i = 0; i < 9; i++) {
				time_msg = (MSG_BUF *) request_memory_block();
				time_msg->mtext[0] = s[i];
				send_message(PID_CRT, time_msg);
			}
			time++;
			delayed_send(PID_CLOCK, msg, 1000);
		}
	}
}
