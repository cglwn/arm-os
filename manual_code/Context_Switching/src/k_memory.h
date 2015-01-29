/**
 * @file:   k_memory.h
 * @brief:  kernel memory managment header file
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */
 
#ifndef K_MEM_H_
#define K_MEM_H_

#include "k_rtx.h"

/* ----- Definitions ----- */
#define RAM_END_ADDR 0x10008000
#define HEAP_BLOCK_SIZE 0x100
/* ----- Types ----- */

/* ----- Variables ----- */
extern PCB **gp_pcbs;
extern PCB *gp_current_process;

typedef struct mem_block {
	struct mem_block *mbNext;
	U32 *uMemory;
}MEM_BLOCK;

/* This symbol is defined in the scatter file (see RVCT Linker User Guide) */  
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit; 
extern PCB **gp_pcbs;
extern PROC_INIT g_proc_table[NUM_TEST_PROCS];

/* ----- Functions ------ */
void memory_init(void);
U32 *alloc_stack(U32 size_b);
void *k_request_memory_block(void);
int k_release_memory_block(void *);


/* ----- Helper Functions ------ */
void enableInterrupts( BOOLEAN nEnable ); 
void initializeMemBlock(MEM_BLOCK *mbBlock, U32* uMemory);

#endif /* ! K_MEM_H_ */
