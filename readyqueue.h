#ifndef RQ
#define RQ

#include "pcb.h"
typedef struct {
char *policy;
pcb *head;
int first;
} ready_queue;
void ready_queue_init(pcb *head, ready_queue *queue, char *policy);
void ready_queue_enqueue(pcb *new_block, ready_queue *queue);
pcb *ready_queue_pop(ready_queue *queue, int instructions);
int ready_queue_isEmpty(ready_queue *queue);
void ready_queue_age(ready_queue *queue);
pcb *ready_queue_get_pcb(ready_queue *queue, int pid);


#endif