/**
 * @file:   usr_proc.c
 * @brief:  Six user processes: proc1...6 to test memory blocking/unblocking 
 * @author: Yiqing Huang
 * @date:   2014/02/07
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 *       The test requires set_process_priority preemption works properly.
 *   
 * Two possible output unde the assumption that 
 * we have TWO memory blocks in the system.
 *
 * Expected UART output: (assuming memory block has ownership.):
 * ABCDE
 * FGHIJ
 * 01234
 * KLMNO
 * 56789
 * proc2: end of testing
 * proc3: 
 * proc4: 
 * proc5: 
 * proc6: 
 * proc3: 
 * proc4: 
 * proc5: 
 * proc6: 
 *
 * Expected UART output: (assuming shared memory among processes (no ownership))
 * ABCDE
 * FGHIJ
 * 01234
 * KLMNO
 * 56789
 * PQRST
 * 01234
 * UVWXY
 * 56789
 * ZABCD
 * 01234
 * ...... you see P1 and P2 keep alternating between each other, p3-p6 will never run
 * 
 */

#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "message.h"
#include "printf.h"
#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];

void set_test_procs() {
	int i;
	for( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_test_procs[i].m_pid=(U32)(i+1);
		g_test_procs[i].m_stack_size=0x100;
	}
	//g_test_procs[0].m_pid = (U32) PID_CLOCK;
  
	g_test_procs[0].mpf_start_pc = &proc1;
	g_test_procs[0].m_priority   = HIGH;
	
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[1].m_priority   = MEDIUM;
	
	g_test_procs[2].mpf_start_pc = &proc3;
	g_test_procs[2].m_priority   = MEDIUM;
	
	g_test_procs[3].mpf_start_pc = &proc4;
	g_test_procs[3].m_priority   = LOWEST;
	
	g_test_procs[4].mpf_start_pc = &proc5;
	g_test_procs[4].m_priority   = LOWEST;
	
	g_test_procs[5].mpf_start_pc = &proc6;
	g_test_procs[5].m_priority   = LOWEST;
	uart1_put_string("\n\r\n\r");
	uart1_put_string("G015_test: START\r\nG015_test: total 6 tests\r\n");
}

/**
 * @brief: a process that prints five uppercase letters
 *         and request a memory block.
 */
void proc1(void)
{
	MSG_BUF *msg = (MSG_BUF *)request_memory_block();
	msg->mtext[0] = 'a';
	delayed_send(PID_CRT, msg, 1000);
	
	/*
	MSG_BUF *msg = (MSG_BUF *)receive_message(NULL);
	if(msg->mtext[0] == 'a'){
		uart1_put_string("G015_test: Test 1 OK\r\n"); //Test normal send_message
	}
	release_memory_block(msg);
	set_process_priority(PID_P1, LOWEST);
	
	while(1) {
		release_processor();
	}
	*/
}

/**
 * @brief: a process that prints five numbers
 *         and then releases a memory block
 */
void proc2(void)
{
	MSG_BUF *msg;
	msg = (MSG_BUF *)request_memory_block();
	msg->mtext[0] = 'a';
	send_message(PID_P3, msg);
	msg = (MSG_BUF *)request_memory_block();
	msg->mtext[0] = 'b';
	send_message(PID_P3, msg);
	//send_message(PID_P3, msg);
	while ( 1 ) {
		release_processor();
	}
}

void proc3(void)
{
	MSG_BUF *msg = (MSG_BUF *)receive_message((int*)PID_P2);
	if (msg && msg->mtext[0] == 'b') {
		uart1_put_string("G015_test: Test 2 OK\r\n");
	}
	release_memory_block(msg);
	while ( 1 ) {
		release_processor();
	}
}

void proc4(void)
{
	while ( 1 ) {
		release_processor();
	}
}
void proc5(void)
{
	while(1) {
		release_processor();
	}
}
void proc6(void)
{
	int i=0;
	
	while(1) {
		if ( i < 2 )  {
			uart0_put_string("proc6: \n\r");
		}
		release_processor();
		i++;
	}
}

void clock_proc(void) {
	MSG_BUF *msg;
	char s[1];
	int i;
	int time = 0;
	msg = (MSG_BUF *) request_memory_block();
	while(1) {

		//crt to uart0 to free
		delayed_send(PID_CLOCK, msg, 1000);
		msg = receive_message(NULL);
		//sprintf(s, "\033[s\033[1;69H%02d:%02d:%02d\n\033[u", (time/60/60) % 24, (time/60)%60, (time %60));
		for (i = 0; i < 8; i++) {
			sprintf(s, "%d", i);
			msg = (MSG_BUF *) request_memory_block();
			msg->mtext[0] = s[0];
			//crt -> uart0 -> free
			send_message(PID_CRT, msg);
		}
		time++;
	}
}
