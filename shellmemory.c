#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[MEM_SIZE];
struct program_lines_struct{
    int size;
    char lines[1000][100]; //program_lines has capacity of 1000 lines of 100 characters.
};
struct program_lines_struct program_lines;
int next_pid=1;// keep track of available and used pids

// Helper functions
int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i])
            matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else
        return 0;
}

// Shell memory functions

void mem_init() {
    int i;
    for (i = 0; i < MEM_SIZE; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, "none") == 0) {
            shellmemory[i].var = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    return;
}

//get value based on input key
char *mem_get_value(char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            return strdup(shellmemory[i].value);
        }
    }
    return "Variable does not exist";
}

struct memory_struct shellmemory[MEM_SIZE];

void program_lines_init(){
    program_lines.size=0;
    memset(program_lines.lines, '\0', sizeof(program_lines.lines));
}
//Add new line to memory and augment current size
void program_lines_append(char* line){
    strcpy(program_lines.lines[program_lines.size],line); 
    program_lines.size++;
    
}
//return next available line slot
int program_lines_current(){
    return program_lines.size;
}
int next_PID(){
    next_pid++;
    return next_pid-1;
}
//returns program line at index i and frees the slot
char *program_lines_get(int i){
    char *line= malloc(100*sizeof(char));
    strcpy(line,program_lines.lines[i]);
    memset(program_lines.lines[i], '\0', 100);
    return line;
}
