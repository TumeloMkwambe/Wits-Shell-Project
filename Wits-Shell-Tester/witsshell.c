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

void built_in_command(char *args[], int n){
    printf("Built-in command!\n");
}

void parse_command(char *line){
    char *args[INPUT_MAX] = {NULL};
    
    int i = 0;
    // Separate command arguments and adds them to args array!
    while((args[i] = strsep(&line, " ")) != NULL){
        if(strlen(args[i]) > 0){
            printf("args[%d] = %s\n", i, args[i]);
            i++;
        }
    }

    // Handle built-in commands
    if(strcmp(args[0], "exit") == 0 || strcmp(args[0], "cd") == 0 || strcmp(args[0], "path") == 0){
        int n = sizeof(args) / sizeof(args[0]);
        built_in_command(args, n);
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
            parse_command(line);
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
            parse_command(line);
        }
    }

    return(0);
}