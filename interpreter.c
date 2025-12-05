#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "shellmemory.h"
#include "shell.h"
#include <sys/wait.h>
#include "pcb.h"
#include "readyqueue.h"


int MAX_ARGS_SIZE = 7;

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}

int help();

int quit();

int set(char *var, char *value);

int print(char *var);

int source(char *script);

int echo(char message[]);

int my_ls();

int my_mkdir(char *dirname);

int my_touch(char *filename);

int my_cd(char *dirname);

int run(char *command[]);

int exec(char *files[],char *policy, int file_nb, int background);

int badcommandFileDoesNotExist();


// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand();
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);

    } else if (strcmp(command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand();
        
        return exec(&command_args[1],"FCFS",1,0);// source is now a helper for exec
      
        
    } else if (strcmp(command_args[0], "echo") == 0) {
        if (args_size != 2)
            return badcommand();
        return echo(command_args[1]);

    } else if (strcmp(command_args[0], "my_ls") == 0) {
        if (args_size != 1)
            return badcommand();
        return my_ls();

    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
        if (args_size != 2)
            return badcommand();
        return my_mkdir(command_args[1]);

    } else if (strcmp(command_args[0], "my_touch") == 0) {
        if (args_size != 2)
            return badcommand();
        return my_touch(command_args[1]);

    } else if (strcmp(command_args[0], "my_cd") == 0) {
        if (args_size != 2)
            return badcommand();
        return my_cd(command_args[1]);

    } else if (strcmp(command_args[0], "run") == 0) {
        command_args[args_size] = NULL;// execvp expects a null-terminated array
        return run(&command_args[1]);

    } else if (strcmp(command_args[0], "exec") == 0) {
        
        if(args_size<3 || args_size>6)
            return badcommand();
        if(*command_args[args_size-1]=='#'){ // with and without background are handled differently(indices,arg_size)
            char *files[args_size-3];
         for(int i =1;i<args_size-2;i++){
            files[i-1]=strdup(command_args[i]);
        }
        return exec(files,command_args[args_size-2],args_size-3,1);
        }
        else {
        char *files[args_size-2];
        for(int i =1;i<args_size-1;i++){
            files[i-1]=strdup(command_args[i]);
        }
        return exec(files,command_args[args_size-1],args_size-2,0);}

    }else
        return badcommand();
}

int help() {
    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
    // Challenge: allow setting VAR to the rest of the input line,
    // possibly including spaces.

    // Hint: Since "value" might contain multiple tokens, you'll need to loop
    // through them, concatenate each token to the buffer, and handle spacing
    // appropriately. Investigate how `strcat` works and how you can use it
    // effectively here.

    mem_set_value(var, value);
    return 0;
}


int print(char *var) {
    printf("%s\n", mem_get_value(var));
    return 0;
}
// Source now abstracts the creation and enqueueing of every process, actual execution happens in exec()
int source(char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");      // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }
    pcb *script_pcb =malloc(sizeof(pcb)); // allocate memory for pcb, will be freed at termination
    
    int startIndex=program_lines_current(); //program_lines_current() returns the next free line in program_lines
    
    int length=0;
       
    while (1) {
  
        memset(line, 0, sizeof(line));

        if (feof(p)) {

            break;
        }
        fgets(line, MAX_USER_INPUT - 1, p);
        
        length++;
        
        program_lines_append(line);     

    }

    fclose(p);
    pcb_init(script_pcb,length,startIndex); // build pcb with computed length and startindex
    scheduler_enqueue(script_pcb);
    return script_pcb->pid;
 
}

int echo(char message[]) {
    if (message[0] == '$') {
        char *val = mem_get_value(&message[1]);//exclude '$' sign
        if (strcmp(val, "Variable does not exist") == 0) {
            printf("\n");
            return 0;
        } else {
            printf("%s\n", val);
            return 0;
        }
    } else {
        printf("%s\n", message);
        return 0;
    }
}
// comparison function to emulate passing strcmp as an argument to qsort
int comp(const void *string1, const void *string2) {
    return strcmp(*(const char **) string1, *(const char **) string2);
}

int my_ls() {
    DIR *dir = opendir(".");
    struct dirent *entry;
    char **files;
    files = malloc(16 * sizeof(char *));
    int capacity = 16;
    int i = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (i >= capacity) {
            capacity = capacity * 2;
            files = realloc(files, capacity * sizeof(char *));// We don't know how many files there are in the directory
        }
        files[i] = strdup(entry->d_name);// dynamic allocation for filename
        i++;
    }
    closedir(dir);
    qsort(files, i, sizeof(char *), comp);
    for (int j = 0; j < i; j++) {
        printf("%s\n", files[j]);
        free(files[j]);
    }
    free(files);
    return 0;
}

int my_mkdir(char *dirname) {
    if (dirname[0] == '$') {
        char *val = mem_get_value(&dirname[1]);
        if (strcmp(val, "Variable does not exist") == 0) {
            printf("Bad command: my_mkdir\n");
            return 0;
        } else {
            mkdir(val, 0777);
        }
    } else {
        for(int i=0;i<strlen(dirname);i++){
            if((dirname[i]<'a'||dirname[i]>'z')&&(dirname[i]<'A'||dirname[i]>'Z')){
            printf("Bad command: my_mkdir\n");
            return 0;
            }
        }
        mkdir(dirname, 0777);
    }
}

int my_touch(char *filename) {
    fclose(fopen(filename, "w+"));
}

int my_cd(char *dirname) {
    if (chdir(dirname) == 0) {
        return 0;
    } else {
        printf("Bad command: my_cd\n");
        return 0;
    }
}

int run(char *command[]) {
    int wstatus;
    fflush(stdout);
    pid_t pid = fork();
    if (pid == 0) {
        execvp(command[0], command); // execvp takes the command as first argument, and the arguments(including the command) as the second
    } else if (pid < 0) {
        perror("forking failed");
        return 1;
    } else {
        wait(&wstatus);// retrieve child process exit code
        return 0;
    }
}
    void readShell() { // helper for background handling. Emulates source(), but reads and converts stdin instead of a script file.
    pcb *script_pcb =malloc(sizeof(pcb));
    int startIndex=program_lines_current();
    int length=0;
        while(1){
        char userInput[MAX_USER_INPUT];
        fgets(userInput, MAX_USER_INPUT - 1, stdin);
        length++; 
        program_lines_append(userInput);
        if (userInput == NULL|| feof(stdin)) {
            break;
        }
                // ignore all other errors
        memset(userInput, 0, sizeof(userInput));
        }
        pcb_init(script_pcb,length,startIndex);
        script_pcb->pid=0; // bash script has special pid 0, which the scheduler will use as a flag.
        scheduler_enqueue(script_pcb);
    }

    int exec(char *files[],char *policy, int files_nb, int background) { 
        int* pids = malloc(sizeof(int)* files_nb);
        int errCode=0;
        int isFirstCall=increment_recursion(1)==1; // Vanguard against recursive exec calls for initializing and running the scheduler
        if(isFirstCall){
        scheduler_init(policy);}
        if(background){
            readShell();
        }
        for(int i=0;i<files_nb;i++){
            for(int j =0;j<i;j++){
                if(strcmp(files[i],files[j])==0){
                    pcb *script_pcb = malloc(sizeof(pcb));
                    pcb *clone = get_pcb(pids[j]);
                    pcb_init(script_pcb,clone->length,clone->startIndex);
                    scheduler_enqueue(script_pcb);
                    
                }
            }
         
            pids[i]= source(files[i]); 
        
            if(errCode!=0){
                return errCode;
            }
        }
            if(isFirstCall){
            while(!scheduler_isEmpty()){ // Single loop runs the scheduler for n instructions(n depends on scheduling policy) until each process is terminated. n is 1000 for non preemptive policies, so scheduler runs until termination

                
                if(strcmp("RR",policy)==0){
                   errCode= scheduler_run(2);
                }
                else if (strcmp("AGING",policy)==0){
                    errCode=scheduler_run(1);
                    scheduler_age();
                }
                else if (strcmp("RR30",policy)==0){
                    errCode= scheduler_run(30);
                }
                else{
                    errCode = scheduler_run(1000);
                }
            }}
            increment_recursion(-1);// return from recursive call
        return errCode;
    }
    

