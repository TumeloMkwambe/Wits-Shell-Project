#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#define INPUT_MAX 10

void error(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void built_in_commands(char *args[], int n){
    int i = 0;

    if(strcmp(args[0], "cd") == 0){
        for(int j = 1; j < n; j++){
            if(args[j] != NULL){
                i++;
            }
        }
        if(i != 1){
            error();
        }
        chdir(args[1]);
    }
    else if(strcmp(args[0], "exit") == 0){
        error();
    }
}

void parse(char *line){
    char path[INPUT_MAX] = "/bin/";
    char *args[INPUT_MAX] = {NULL};
    int n = sizeof(args) / sizeof(args[0]);
    
    int i = 0;
    // Separate command arguments and adds them to args array!
    while((args[i] = strsep(&line, " ")) != NULL){
        if(strlen(args[i]) > 0){
            i++;
        }
    }
    
    // If command is built-in command then execute handler!
    if(strcmp(args[0], "exit") == 0 || strcmp(args[0], "cd") == 0 || strcmp(args[0], "path") == 0){
        built_in_commands(args, n);
    }
    
    // If command is not built-in command then execute handler!
    else{
        printf("Not built-in command!");
    }
}

void execute(char *command, char *args[]){
    int pid = fork();
    if(pid == 0){
        execv(command, args);
    }
    else{
        wait(NULL);
    }
}

int main(int MainArgc, char *MainArgv[]){
    FILE *file;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    if(MainArgc > 2){ // Invalid argument count
        error();
    }

    else if(MainArgc == 2){ // Batch case
        file = fopen(MainArgv[1], "r");
        while ((nread = getline(&line, &len, file)) != -1) {
            if (line[nread - 1] == '\n') {
                line[nread - 1] = '\0';
            }
            printf("COMMAND: %s\n", line);
        }
        //return(0);
    }

    else{
        while (1) // Interactive case
        {
            printf("witsshell> ");
            nread = getline(&line, &len, stdin);
            if (line[nread - 1] == '\n') {
                line[nread - 1] = '\0';
            }
            printf("COMMAND: %s\n", line);
        }
    }

    return(0);
}