#include "pcb.h"
#include "shellmemory.h"
#include "stdlib.h"
void pcb_init(pcb *script,int length, int startIndex,int *pagetable){ //pcb initialization, pcb has nextNode property for Linked List implementation
script->startIndex= startIndex;
script->currentIndex=startIndex;
script->length=length;
script->length_score=length;
script->next=NULL;
script->pid=next_PID(); //assigns pid based on current memory
script->pagetable=pagetable;
}