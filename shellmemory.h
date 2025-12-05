#define MEM_SIZE 1000

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
void program_lines_init();
int program_lines_append(char *line);
int program_lines_current();
char *program_lines_get(int i);
int next_PID();
