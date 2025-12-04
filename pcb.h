#ifndef PCB
#define PCB
typedef struct pcb {
int startIndex;
int pid;
int length;
int length_score;
int currentIndex;
struct pcb *next;
} pcb;
void pcb_init(pcb *script,int length,int startIndex);
#endif