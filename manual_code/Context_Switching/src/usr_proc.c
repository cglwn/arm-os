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
#include "sys_proc.h"
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
  
	g_test_procs[0].mpf_start_pc = &proc1;
	g_test_procs[0].m_priority   = HIGH;
	
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[1].m_priority   = MEDIUM;
	
	g_test_procs[2].mpf_start_pc = &proc3;
	g_test_procs[2].m_priority   = LOW;
	
	g_test_procs[3].mpf_start_pc = &proc4;
	g_test_procs[3].m_priority   = LOW;
	
	g_test_procs[4].mpf_start_pc = &proc5;
	g_test_procs[4].m_priority   = LOW;
	
	g_test_procs[5].mpf_start_pc = &proc6;
	g_test_procs[5].m_priority   = LOW;
	uart0_put_string("\n\r\n\r");
	uart0_put_string("G015_test: START\r\nG015_test: total 6 tests\r\n");
}

/**
 * @brief: a process that prints five uppercase letters
 *         and request a memory block.
 */
void proc1(void)
{
	int i = 0;
	void *p_mem_blk1;
	void *p_mem_blk2;
	p_mem_blk1 = request_memory_block();
	p_mem_blk2 = request_memory_block();
	release_memory_block(p_mem_blk1);
	release_memory_block(p_mem_blk2);
	uart0_put_string("G015_test: Test 2 OK\r\n"); //Test empty blocked queue on release_memory_block
	set_process_priority(PID_P1, LOWEST);
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
	int i = 0;
	int ret_val = 20;
	void *p_mem_blk;
	
	p_mem_blk = request_memory_block();
	set_process_priority(PID_P2, HIGH);
	uart0_put_string("G015_test: Test 1 OK\r\n"); //Tests no pre-emption if setting self to highest
	set_process_priority(PID_P2, MEDIUM);
	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			ret_val = release_memory_block(p_mem_blk);
			p_mem_blk = NULL;
			if ( ret_val == 0 ) {
				uart0_put_string("G015_test: Test 3 OK\r\n"); //Test blocked queue 
			}
#ifdef DEBUG_0
			printf("proc2: ret_val=%d\n", ret_val);
#endif /* DEBUG_0 */
			if ( ret_val == -1 ) {
				break;
			}
		}
		//uart0_put_char('0' + i%10);
		i++;
	}
	set_process_priority(PID_P2, HIGH);
	set_process_priority(PID_P3, MEDIUM);
	set_process_priority(PID_P4, MEDIUM);
	set_process_priority(PID_P5, MEDIUM);
	uart0_put_string("G015_test: Test 4 OK\r\n"); //Test priority changes of new processes
	set_process_priority(PID_P2, LOWEST);
	while ( 1 ) {
		release_processor();
	}
}

void proc3(void)
{
	void *p_mem_blk1;
	void *p_mem_blk2;
	int i = 0;
	int counter = 0;
	int ret_val = 100;
	p_mem_blk1 = request_memory_block();
	p_mem_blk2 = request_memory_block();
	release_processor();
	ret_val = release_memory_block(p_mem_blk1);
	p_mem_blk1 = request_memory_block();
	uart0_put_string("G015_test: Test 6 OK\r\n"); //Blocked Queue higher priority 
	uart0_put_string("G015_test: Test 6/6 OK\n\r");
	uart0_put_string("G015_test: Test 0/6 FAIL\n\r");
	uart0_put_string("G015_test: END\n\r");
	while ( 1 ) {
		release_processor();
	}
}

void proc4(void)
{
	void *p_mem_blk;
	p_mem_blk = request_memory_block();
	uart0_put_string("G015_test: Test 5 OK\r\n"); //Blocked Queue same priority 
	set_process_priority(PID_P3, HIGH);
	release_memory_block(p_mem_blk);
	while ( 1 ) {
		release_processor();
	}
}
void proc5(void)
{
	void *p_mem_blk;
	p_mem_blk = request_memory_block();
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
