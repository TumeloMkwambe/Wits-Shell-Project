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

void execute(char *line, char *path[]){
    char default_path[INPUT_MAX] = "/bin/";
    char *args[INPUT_MAX] = {NULL};
    
    int i = 0;
    while((args[i] = strsep(&line, " ")) != NULL){ // parses line and adds arguments to args
        if(strlen(args[i]) > 0){
            i++;
        }
        args[i] = NULL;
    }

    int k = 0;
    int n = sizeof(args) / sizeof(args[0]);
    if(strcmp(args[0], "cd") == 0){ // checks if cd command and then checks argument count
        for(int j = 1; j < n; j++){
            if(args[j] != NULL){
                k++;
            }
        }
        if(k != 1){
            error();
        }
        chdir(args[1]); // if argument count = 1, then change directory
    }
    else if(strcmp(args[0], "exit") == 0){ // if exit command with argument, then throw error
        error();
    }
    else if(strcmp(args[0], "path") == 0){
        if(args[1] == NULL){
            path[0] = "/bin/";
        }
        else{
            int i = 0;
            while(args[i] != NULL){
                path[i] = args[i+1];
                i++;
            }
        }
    }
    else{
        printf("Path[0] = %s\n", path[0]); // BUG: Arguments to command erase value of path[0]!

        strcat(default_path, args[0]);
        int pid = fork();
        if(pid == 0){
            int status = execv(default_path, args);
            if(status == -1){
                error();
            }
        }
        else{
            wait(NULL);
        }
    }
}

int main(int MainArgc, char *MainArgv[]){
    char *path[INPUT_MAX] = {NULL};
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
            if(strcmp(line, "exit") == 0){
                exit(0);
            }
            execute(line, path);
        }

        return(0);
    }

    while (1) // Interactive case
    {
        printf("witsshell> ");
        nread = getline(&line, &len, stdin);
        if (line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }
        if(strcmp(line, "exit") == 0){
            exit(0);
        }

        execute(line, path);
    }

    return(0);
}