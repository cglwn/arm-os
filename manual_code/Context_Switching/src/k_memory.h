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
#ifndef HEAP_BLOCK_SIZE
#define HEAP_BLOCK_SIZE 0x200
#endif //HEAP_BLOCK_SIZE
#ifndef NUM_MEM_BLOCKS
#define NUM_MEM_BLOCKS 0xFF
#endif //NUM_MEM_BLOCKS
/* ----- Types ----- */

/* ----- Variables ----- */
extern PCB **gp_pcbs;
extern PCB *gp_current_process;

typedef struct mem_block {
	struct mem_block *mb_next;
	U32 *u_memory;
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
BOOLEAN is_in_heap(U32* address);

/* ----- Helper Functions ------ */
void initialize_mem_block(MEM_BLOCK *mb_block, U32* u_memory);

#endif /* ! K_MEM_H_ */
