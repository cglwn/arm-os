/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_memory.h"
#include "k_process.h"
#include "k_utilities.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

/* ----- Global Variables ----- */
U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
               /* The first stack starts at the RAM high address */
	       /* stack grows down. Fully decremental stack */
U32 *start_of_heap;
MEM_BLOCK *mb_head;
/**
 * @brief: Initialize RAM as follows:

0x10008000+---------------------------+ High Address
          |    Proc 1 STACK           |
          |---------------------------|
          |    Proc 2 STACK           |
          |---------------------------|<--- gp_stack
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|<--- p_end
          |        PCB 2              |
          |---------------------------|
          |        PCB 1              |
          |---------------------------|
          |        PCB pointers       |
          |---------------------------|<--- gp_pcbs
          |        Padding            |
          |---------------------------|  
          |Image$$RW_IRAM1$$ZI$$Limit |
          |...........................|          
          |       RTX  Image          |
          |                           |
0x10000000+---------------------------+ Low Address

*/

void memory_init(void)
{
	U8 *p_end = (U8 *)&Image$$RW_IRAM1$$ZI$$Limit;
	U32* block_start;
	int i;
	int num_blocks = 1;
  
	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	gp_pcbs = (PCB **)p_end;
	p_end += (NUM_TEST_PROCS + 1) * sizeof(PCB *);
  
	for ( i = 0; i < (NUM_TEST_PROCS + 1); i++ ) {
		gp_pcbs[i] = (PCB *)p_end;
		p_end += sizeof(PCB); 
	}
#ifdef DEBUG_0  
	printf("gp_pcbs[0] = 0x%x \n", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n", gp_pcbs[1]);
#endif
	
	/* prepare for alloc_stack() to allocate memory for stacks */
	
	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack; 
	}
  
	/* allocate memory for heap, not implemented yet*/
	start_of_heap = (U32*) p_end;
	block_start = (U32*) p_end;
	if(block_start < gp_stack) {
		MEM_BLOCK *mb_current;
		mb_head = (MEM_BLOCK *) block_start;
		mb_current = mb_head;
		initialize_mem_block(mb_current, block_start);
		while(block_start < gp_stack && (block_start+HEAP_BLOCK_SIZE) < gp_stack && num_blocks < NUM_MEM_BLOCKS) {
			MEM_BLOCK *pBlock;
			block_start += HEAP_BLOCK_SIZE;
			pBlock = (MEM_BLOCK *) block_start;
			initialize_mem_block( pBlock , block_start);
			mb_current->mb_next = pBlock;
			mb_current = pBlock;
			num_blocks++;
		}
	}
#ifdef DEBUG_0  
	printf("mb_head = 0x%x \n", mb_head->u_memory);
#endif
}

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */

U32 *alloc_stack(U32 size_b) 
{
	U32 *sp;
	sp = gp_stack; /* gp_stack is always 8 bytes aligned */
	
	/* update gp_stack */
	gp_stack = (U32 *)((U8 *)sp - size_b);
	
	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)gp_stack & 0x04) {
		--gp_stack; 
	}
	return sp;
}

void *k_request_memory_block(void) {
	MEM_BLOCK *temp_block = NULL;
#ifdef DEBUG_0 
	printf("k_request_memory_block: entering...\n");
#endif /* ! DEBUG_0 */
	enable_interrupts(false);
	while(!mb_head) {
		gp_current_process->m_state = BLOCKED;
		enqueuePriority(PCBBlockedQueue, gp_current_process);
		enable_interrupts(true);
		k_release_processor();
	}
	enable_interrupts(false);
	temp_block = mb_head;
	mb_head = mb_head->mb_next;
	enable_interrupts(true);
	return temp_block->u_memory;
}

int k_release_memory_block(void *p_mem_blk) {
	MEM_BLOCK *temp_block;
	MEM_BLOCK *temp_next = mb_head;
	PCB* blocked_pcb;
#ifdef DEBUG_0 
	printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
#endif /* ! DEBUG_0 */
	enable_interrupts(false);
	
	//error if memory is not aligned
	if (!(((U32*)p_mem_blk - start_of_heap) % HEAP_BLOCK_SIZE == 0 &&
			(U32*)p_mem_blk < gp_stack &&
			(U32*)p_mem_blk >= start_of_heap) ||
			is_in_heap((U32*)p_mem_blk)){
				
		enable_interrupts(true);
		return RTX_ERR;
	}
	
	//put freed memory block back in heap
	temp_block = p_mem_blk;
	initialize_mem_block(temp_block, p_mem_blk);
	mb_head = temp_block;
	mb_head->mb_next = temp_next;
	
	//put blocked process in ready queue
	blocked_pcb = dequeuePriority(PCBBlockedQueue);
	if(blocked_pcb) {
		blocked_pcb->m_state = RDY;
		handle_process_ready(blocked_pcb);
	}
	enable_interrupts(true);
	return RTX_OK;
}


/* ----- Helper Functions ------ */

BOOLEAN is_in_heap(U32* address) { 
	MEM_BLOCK *temp_block = mb_head;
	while(temp_block != NULL) { 
		if (temp_block->u_memory == address) { 
			return true;
		}
		temp_block = temp_block->mb_next;
	}
	return false;
}
void initialize_mem_block(MEM_BLOCK *mb_block, U32* u_memory) {
#ifdef DEBUG_0  
	printf("%d\n", u_memory);
#endif
	mb_block->mb_next = NULL;
	mb_block->u_memory = u_memory;
}
