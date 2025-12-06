#define MEM_SIZE 1000

#define MAX_USER_INPUT 1000
#ifndef FRAMESIZE
#define FRAMESIZE 33
#endif
#ifndef VARMEMSIZE
#define VARMEMSIZE 33
#endif
void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
void program_lines_init();
int program_lines_append(char *line[3]);
void disk_append(char *line);
int program_lines_current();
char *program_lines_get(int i);
int next_PID();
