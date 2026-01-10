#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "shellmemory.h"
#include "pcb.h"
#include "shell.h"


long lru_clock = 0;

struct disk_struct {
    int size;
    char lines[1000][100];      //backup storage, emulates hard disk
};
struct memory_struct { //Variable in memory
    char *var;
    char *value;
};

struct memory_struct shellmemory[VARMEMSIZE]; //Variable store

//Serves as Frame Store
struct program_lines_struct { 
    int size;
    int occupied[FRAMESIZE / 3]; // flag for frame availability
    int processes[FRAMESIZE / 3][2]; //stores information on the pid and relative page of the process that occupies the frame
    char lines[FRAMESIZE / 3][3][100];  // Actual frame store
    unsigned long lru[FRAMESIZE / 3]; //Tracks usage recency for a given frame
};

struct disk_struct disk;
struct program_lines_struct program_lines;
int next_pid = 1;               // keep track of available and used pids

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
    for (i = 0; i < VARMEMSIZE; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < VARMEMSIZE; i++) {
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

    for (i = 0; i < VARMEMSIZE; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            return strdup(shellmemory[i].value);
        }
    }
    return "Variable does not exist";
}



void program_lines_init() {
    disk.size = 0;
    memset(program_lines.lines, '\0', sizeof(program_lines.lines));
    memset(disk.lines, '\0', sizeof(disk.lines));

}

//Helper for finding lru frame to evict in case of page fault
int find_lru_victim() {                     
    unsigned long min = program_lines.lru[0];
    int victim = 0;

    for (int i = 1; i < FRAMESIZE / 3; i++) {
        if (program_lines.lru[i] < min) {
            min = program_lines.lru[i];
            victim = i;
        }
    }
    return victim;
}
//Upload page to frame store, the function returns the assigned frame slot index for the page
int program_lines_append(char lines[3][100], int pid, int page) {
    for (int f = 0; f < FRAMESIZE / 3; f++) { //Find next available frame
        if (program_lines.occupied[f] == 0) {
            for (int i = 0; i < 3; i++) {
                strcpy(program_lines.lines[f][i], lines[i]);
            }
            program_lines.processes[f][0] = pid;
            program_lines.processes[f][1] = page;
            program_lines.occupied[f] = 1;
            
            lru_clock++;
            program_lines.lru[f] = lru_clock;

            return f;
        }
    }
    int evict = find_lru_victim(); // Frame store is full -> Evict LRU

    pcb *evicted = get_pcb(program_lines.processes[evict][0]); //update occupant pcb's page table
    if (evicted != NULL) {
        evicted->pagetable[program_lines.processes[evict][1]] = -1;
    }
    char victim[3][100];
    for (int i = 0; i < 3; i++) {
        strcpy(victim[i], program_lines.lines[evict][i]);
        strcpy(program_lines.lines[evict][i], lines[i]); //Load page to chosen frame

    }
    program_lines.processes[evict][0] = pid;
    program_lines.processes[evict][1] = page;
    program_lines.occupied[evict] = 1;
    printf("Victim page contents:\n\n%s%s%s \nEnd of victim page contents.",
           victim[0], victim[1], victim[2]);
    lru_clock++;
    program_lines.lru[evict] = lru_clock;
    return evict;

}

//Write data on the hard disk(happens upon reading the file once)
void disk_append(char *line) {
    strcpy(disk.lines[disk.size], line);
    disk.size++;

}

//return next available disk line slot
int disk_current() {
    return disk.size;
}
int next_PID() {
    next_pid++;
    return next_pid - 1;
}

//returns program line at index page+offset
char *program_lines_get(int page, int offset) {
    lru_clock++;
    program_lines.lru[page] = lru_clock;
    char *line = malloc(100 * sizeof(char));
    strcpy(line, program_lines.lines[page][offset]);
    return line;
}
// function for page_fault handling
void fetch(pcb * task) {
    char page[3][100];
    for (int i = 0; i < 3; i++) {
        strcpy(page[i], disk.lines[task->currentIndex + i]);
    }

    task->pagetable[(task->currentIndex - task->startIndex) / 3] = // Update page table with frame index
        program_lines_append(page, task->pid,
                             (task->currentIndex - task->startIndex) % 3);
}
