#ifndef K_UTIL_H_
#define K_UTIL_H_

#include "k_rtx.h"

/* ----- Functions ----- */

PCB* dequeue( PCB* pcbQueue );
void enqueue ( PCB* pcbQueue, PCB* pcbNode );

PCB* dequeuePriority( void*  voidQueue );
void enqueuePriority ( PCB**  pcbQueue, PCB* pcbNode );


#endif // K_UTIL_H_
