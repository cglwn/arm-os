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
  g_test_procs[0].m_pid = PID_CLOCK;
	
	g_test_procs[0].mpf_start_pc = &clock_proc;
	g_test_procs[0].m_priority   = HIGH;
	
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[1].m_priority   = MEDIUM;
	
	g_test_procs[2].mpf_start_pc = &proc3;
	g_test_procs[2].m_priority   = MEDIUM;
	
	g_test_procs[3].mpf_start_pc = &proc4;
	g_test_procs[3].m_priority   = LOWEST;
	
	g_test_procs[4].mpf_start_pc = &proc5;
	g_test_procs[4].m_priority   = MEDIUM;
	
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
	/*
	MSG_BUF *msg;
	MSG_BUF *msg2;
	
	msg = (MSG_BUF *)request_memory_block();
	msg->mtext[0] = 'a';
	send_message(PID_P1, msg);
	msg = (MSG_BUF *) receive_message(NULL);
	if(msg->mtext[0] == 'a'){
		uart1_put_string("G015_test: Test 1 OK\r\n"); //Send message to self
	}
	release_memory_block(msg);
	msg = receive_message(NULL);
	msg2 = receive_message(NULL);
	if (msg->mtext[0] == 'b' && msg2->mtext[0] == 'c') {
		uart1_put_string("G015_test: Test 2 OK\r\n");//Test message priority and block
	}
	release_memory_block(msg);
	release_memory_block(msg2);
	msg = request_memory_block();
	msg->mtext[0] = 'd';
	msg2 = request_memory_block();
	msg2->mtext[0] = 'e';
	delayed_send(PID_P2, msg, 1000);
	delayed_send(PID_P3, msg2, 9000);
	msg = receive_message(NULL);
	
	if (msg->mtext[0] == 'f') {
		uart1_put_string("G015_test: Test 6 OK\r\n"); //delayed send test
	}
	uart1_put_string("G015_test: Test 6/6 OK\n\r");
	uart1_put_string("G015_test: Test 0/6 FAIL\n\r");
	uart1_put_string("G015_test: END\n\r");
	
	release_memory_block(msg);
	set_process_priority(PID_P1, LOWEST);
	*/
	while(1) {
		release_processor();
	}
	
}

/**
 * @brief: a process that prints five numbers
 *         and then releases a memory block
 */
void proc2(void)
{
	/*
	MSG_BUF *msg;
	msg = (MSG_BUF *)request_memory_block();
	msg->mtext[0] = 'b';
	send_message(PID_P1, msg);
	msg = (MSG_BUF *)request_memory_block();
	msg->mtext[0] = 'c';
	send_message(PID_P1, msg);
	msg = receive_message(NULL);
		if(msg->mtext[0] == 'd'){
		uart1_put_string("G015_test: Test 3 OK\r\n"); //delayed send test
	}
	release_memory_block(msg);
	msg = receive_message(NULL);
	if (msg->mtext[0] == 'g') {
		uart1_put_string("G015_test: Test 5 OK\r\n"); //delayed send test
	}
	release_memory_block(msg);
	*/
	while ( 1 ) {
		release_processor();
	}
}

void proc3(void)
{	
	/*
	MSG_BUF *msg;
	MSG_BUF *msg2;
	msg = (MSG_BUF *)receive_message(NULL);
	if(msg->mtext[0] == 'e'){
		uart1_put_string("G015_test: Test 4 OK\r\n"); //longer delay test
	}
	release_memory_block(msg);
	msg = (MSG_BUF *) request_memory_block();
	msg->mtext[0] = 'f';
	msg2 = (MSG_BUF *) request_memory_block();
	msg2->mtext[0] = 'g';
	delayed_send(PID_P1, msg, 9000);
	delayed_send(PID_P2, msg2, 5000);
	*/
	
	while(1) {
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
	while(1) {
		release_processor();
	}
}

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
		for (i = 0; i < 5; i++) {
			msg = (MSG_BUF *) request_memory_block();
			msg->mtext[0] = s[i];
			send_message(PID_CRT, msg);
		}
		time++;
	}
}
