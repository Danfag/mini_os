
#define MAX_USER_INPUT 1000
int parseInput(char inp[]);
#ifndef RQS
#define RQS
#include "readyqueue.h"
void scheduler_enqueue(pcb *script);
void scheduler_init(char *policy);
int scheduler_run();
int scheduler_isEmpty();
void scheduler_age();
int increment_recursion();
int decrement_recursion();
pcb* get_pcb(int pid);

#endif


