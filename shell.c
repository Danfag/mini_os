#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"
#include "readyqueue.h"

//page_fault handling from shell
void page_fault(pcb * task, int requeue) {
    printf("Page fault! ");
    if (requeue) {
        scheduler_enqueue(task); //reenqueue the task if it was dequeued when popping(readyqueue.pop dequeues the process if predicted to run to termination, assuming no interruption)
    }
    fetch(task);
    printf("\n");
}
int processInput(char *userInput) {
    int errorCode = 0;
    char input[MAX_USER_INPUT]; // one liners inputs stored here
    int i = 0, j = 0;
    while (i < strlen(userInput)) {
        if (userInput[i] == ';') {      //handle oneliners; each input is treated as its own command
            input[j] = '\0';

            errorCode = parseInput(input);
            if (errorCode == -1) {
                exit(99);
            }
            memset(input, 0, sizeof(input));    //reset input 
            j = 0;
            i++;
        }
        input[j] = userInput[i];
        i++;
        j++;

    }
    input[j] = '\0';
    errorCode = parseInput(input);
    if (errorCode == -1)
        exit(99);               // ignore all other errors
    memset(input, 0, sizeof(input));
    return errorCode;
}

int parseInput(char ui[]);
ready_queue queue;              // global queue, directly managed in shell
int recursion_depth = 0;        // track recursion depth for exec calls
int *pids;
// Start of everything
int main(int argc, char *argv[]) {
    pids = malloc(sizeof(int));
    printf("Frame Store Size = %d; Variable Store Size = %d \n", FRAMESIZE,
           VARMEMSIZE);

    char prompt = '$';          // Shell prompt
    int terminal = isatty(0);   //isatty(0) returns true if fd 0(stdin) is the command line, false otherwise
    char userInput[MAX_USER_INPUT];     // user's input stored here
    int errorCode = 0;          // zero means no error, default

    //init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }

    //init shell memory
    mem_init();
    program_lines_init();       //initialize shell memory for program lines


    while (1) {
        if (terminal) {
            printf("%c ", prompt);
        }
        // here you should check the unistd library
        // so that you can find a way to not display $ in the batch mode
        fgets(userInput, MAX_USER_INPUT - 1, stdin);
        if (userInput == NULL) {
            break;
        }
        processInput(userInput);
    }

    return 0;
}

int wordEnding(char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ';
}

int parseInput(char inp[]) {

    char tmp[200], *words[100];
    int ix = 0, w = 0;
    int wordlen;
    int errorCode;
    for (ix = 0; inp[ix] == ' ' && ix < 1000; ix++);    // skip white spaces
    while (inp[ix] != '\n' && inp[ix] != '\0' && ix < 1000) {
        // ext    ract a word
        for (wordlen = 0; !wordEnding(inp[ix]) && ix < 1000; ix++, wordlen++) {
            tmp[wordlen] = inp[ix];
        }
        tmp[wordlen] = '\0';
        words[w] = strdup(tmp);
        w++;
        if (inp[ix] == '\0')
            break;
        ix++;
    }

    errorCode = interpreter(words, w);
    return errorCode;
}




// run logic: the scheduler will run for $instructions iterations, or until end of task process, depending on policy and remaining lines to run
int scheduler_run(int instructions) {
  start:
    int errCode = 0;
    int requeue = 0;
    pcb *task = ready_queue_pop(&queue, instructions);  // retrieve next task to run(policy logic is not handled here)

    int starting_point = task->currentIndex;    // start from last saved 'checkpoint'
    for (int i = starting_point; i < starting_point + instructions; i++) {
        if (task->currentIndex > task->startIndex + task->length - 1) {
            free(task->pagetable);
            free(task);         // free task from memory upon termination
            break;
        }
        if (task->pagetable[(task->currentIndex - task->startIndex) / 3] == -1) { // check for page faults
            page_fault(task,
                       starting_point + instructions >=
                       task->startIndex + task->length - 1);
            goto start;
        }
        char *line =
            program_lines_get(task->
                              pagetable[(task->currentIndex -
                                         task->startIndex) / 3],
                              (task->currentIndex - task->startIndex) % 3); // currentIndex -> page,offset mapping

        errCode = processInput(line);
        free(line);
        task->currentIndex++;
    }

    return errCode;
}

//encapsulation helper fucntions for queue
void scheduler_enqueue(pcb * script) {
    ready_queue_enqueue(script, &queue);
}
void scheduler_init(char *policy) {
    ready_queue_init(NULL, &queue, policy);
}
int scheduler_isEmpty() {
    return ready_queue_isEmpty(&queue);
}
void scheduler_age() {
    ready_queue_age(&queue);
}
int increment_recursion(int depth) {
    recursion_depth += depth;
    return recursion_depth;
}
pcb *get_pcb(int pid) {
    return ready_queue_get_pcb(&queue, pid);
}
