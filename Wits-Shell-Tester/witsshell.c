#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

#define INPUT_MAX 10

char *path[INPUT_MAX] = {"/bin/", "/usr/bin/", NULL};

void error(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void exit_handler(char *args[], int n){
    int i = 0;
    for(int j = 1; j < n; j++){ // count arguments of exit command
        if(args[j] != NULL){
            i++;
        }
    }
    if(i > 0){ // if exit has atleast 1 argument, throw error
        error();
    }
    exit(0);
}

void cd_handler(char *args[], int n){
    int i = 0;
    for(int j = 1; j < n; j++){ // count arguments of cd command
        if(args[j] != NULL){
            i++;
        }
    }
    if(i != 1){ // if cd has less or more than 1 argument, throw error
        error();
    }
    chdir(args[1]); // change directory in main process if cd has 1 command
}

void path_handler(char *args[], int n){
    int i = 0;
    for(int j = 1; j < n; j++){ // count arguments of path command
        if(args[j] != NULL){
            i++;
        }
    }
    if(i == 0){ // if the path is empty
        path[0] = NULL;
        path[1] = NULL;
    }

}

void built_in_command(char *args[], int n){
    int i = 0;
    if(strcmp(args[0], "exit") == 0){ // if built-in command is exit
        exit_handler(args, n);
    }
    else if(strcmp(args[0], "cd") == 0){ // if built-in command is cd
        cd_handler(args, n);
    }
    else if(strcmp(args[0], "path") == 0){ // if built-in command is path
        path_handler(args, n);
    }
}

void execute(char *args[], char *path_variable){
    int pid = fork();
        if(pid == 0){
            execv(path_variable, args);
        }
        else{
            wait(NULL);
        }
}

void execute_command(char *args[]){
    if(path[0] == NULL){
        error();
    }
    else{
        int n = sizeof(path) / sizeof(path[0]);
        for(int j = 0; j < n; j++){ // travers path array, checks for executable and execute if found
            if(path[j] != NULL){
                char path_variable[1024];
                strcpy(path_variable, path[j]);
                strcat(path_variable, args[0]);
                int path_stat = access(path_variable, X_OK);
                if(path_stat == 0){
                    execute(args, path_variable);
                    break;
                }
            }
        }
    }
}

int check_shell_script(char *command){
    const char *extension = ".sh";
    size_t len_filename = strlen(command);
    size_t len_extension = strlen(extension);

    if (len_filename < len_extension) {
        return 1;
    }

    if (strcmp(command + len_filename - len_extension, extension) == 0) {
        return 0;
    }

    return 1;
}

void parse_command(char *line){
    char *args[INPUT_MAX] = {NULL};
    
    int i = 0;
    // Separate command arguments and adds them to args array!
    while((args[i] = strsep(&line, " ")) != NULL){
        if(strlen(args[i]) > 0){
            i++;
        }
    }

    // Handle built-in commands
    if(strcmp(args[0], "exit") == 0 || strcmp(args[0], "cd") == 0 || strcmp(args[0], "path") == 0){
        int n = sizeof(args) / sizeof(args[0]);
        built_in_command(args, n);
    }

    // Check shell command
    if(check_shell_script(args[0]) == 0){
        error();
    }

    // execute command
    execute_command(args);
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