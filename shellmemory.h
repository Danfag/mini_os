#include "pcb.h"
#define MEM_SIZE 1000

#define MAX_USER_INPUT 100

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
void program_lines_init();
int program_lines_append(char lines[3][100], int pid, int page);
void disk_append(char *line);
int disk_current();
char *program_lines_get(int page, int offset);
int next_PID();
void fetch(pcb * task);
