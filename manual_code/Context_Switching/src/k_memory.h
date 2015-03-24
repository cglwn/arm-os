/**
 * @file:   k_memory.h
 * @brief:  kernel memory managment header file
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */
 
#ifndef K_MEM_H_
#define K_MEM_H_
#define __SVC_0 __svc_indirect(0)
#include "k_rtx.h"

/* ----- Definitions ----- */
#define RAM_END_ADDR 0x10008000
#ifndef HEAP_BLOCK_SIZE
#define HEAP_BLOCK_SIZE 0x100
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
void *k_request_memory_block_nb(void);
int k_release_memory_block(void *);
int k_release_memory_block_nb(void *);
BOOLEAN is_in_heap(U32* address);

/* ----- Helper Functions ------ */
void initialize_mem_block(MEM_BLOCK *mb_block, U32* u_memory);

/* Memeory Management */
extern void *k_request_memory_block(void);
#define request_memory_block() _request_memory_block((U32)k_request_memory_block)
extern void *_request_memory_block(U32 p_func) __SVC_0;


extern int k_release_memory_block(void *);
#define release_memory_block(p_mem_blk) _release_memory_block((U32)k_release_memory_block, p_mem_blk)
extern int _release_memory_block(U32 p_func, void *p_mem_blk) __SVC_0;
#endif /* ! K_MEM_H_ */
