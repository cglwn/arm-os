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
MEM_BLOCK *mbHead;
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
	U32* blockStart;
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
	blockStart = (U32*) p_end;
	if(blockStart < gp_stack) {
		MEM_BLOCK *mbCurrent;
		mbHead = (MEM_BLOCK *) blockStart;
		mbCurrent = mbHead;
		initializeMemBlock(mbCurrent, blockStart);
		while(blockStart < gp_stack && (blockStart+HEAP_BLOCK_SIZE) < gp_stack && num_blocks < NUM_MEM_BLOCKS) {
			MEM_BLOCK *pBlock;
			blockStart += HEAP_BLOCK_SIZE;
			pBlock = (MEM_BLOCK *) blockStart;
			initializeMemBlock( pBlock , blockStart);
			mbCurrent->mbNext = pBlock;
			mbCurrent = pBlock;
			num_blocks++;
		}
	}
#ifdef DEBUG_0  
	printf("mbHead = 0x%x \n", mbHead->uMemory);
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
	MEM_BLOCK *tempBlock = NULL;
#ifdef DEBUG_0 
	printf("k_request_memory_block: entering...\n");
#endif /* ! DEBUG_0 */
	enableInterrupts(false);
	while(!mbHead) {
		gp_current_process->m_state = BLOCKED;
		enqueuePriority(PCBBlockedQueue, gp_current_process);
		enableInterrupts(true);
		k_release_processor();
	}
	enableInterrupts(false);
	tempBlock = mbHead;
	mbHead = mbHead->mbNext;
	enableInterrupts(true);
	return tempBlock->uMemory;
}

int k_release_memory_block(void *p_mem_blk) {
	MEM_BLOCK *tempBlock;
	MEM_BLOCK *tempNext = mbHead;
	PCB* blockedPCB;
#ifdef DEBUG_0 
	printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
#endif /* ! DEBUG_0 */
	enableInterrupts(false);
	
	//error if memory is not aligned
	if (!(((U32*)p_mem_blk - start_of_heap) % HEAP_BLOCK_SIZE == 0 &&
			(U32*)p_mem_blk < gp_stack &&
			(U32*)p_mem_blk >= start_of_heap) ||
			isInHeap((U32*)p_mem_blk)){
		return RTX_ERR;
	}
	
	//put freed memory block back in heap
	tempBlock = p_mem_blk;
	initializeMemBlock(tempBlock, p_mem_blk);
	mbHead = tempBlock;
	mbHead->mbNext = tempNext;
	
	//put blocked process in ready queue
	blockedPCB = dequeuePriority(PCBBlockedQueue);
	if(blockedPCB) {
		blockedPCB->m_state = RDY;
		handle_process_ready(blockedPCB);
	}
	enableInterrupts(true);
	return RTX_OK;
}


/* ----- Helper Functions ------ */
void enableInterrupts( BOOLEAN nEnable )
{
	if( nEnable == true ) {
		__enable_irq();
	} else {
		__disable_irq();
	}
}

BOOLEAN isInHeap(U32* address) { 
	MEM_BLOCK *tempBlock = mbHead;
	while(tempBlock != NULL) { 
		if (tempBlock->uMemory == address) { 
			return true;
		}
		tempBlock = tempBlock->mbNext;
	}
	return false;
}
void initializeMemBlock(MEM_BLOCK *mbBlock, U32* uMemory) {
#ifdef DEBUG_0  
	printf("%d\n", uMemory);
#endif
	mbBlock->mbNext = NULL;
	mbBlock->uMemory = uMemory;
}
