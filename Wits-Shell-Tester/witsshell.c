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

void exitHandler(char *args[], int n){
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

void cdHandler(char *args[], int n){
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

void pathHandler(char *args[], int n){
    int i = 0;
    
    for(int j = 1; j < n; j++){ // count arguments of path command
        if(args[j] != NULL){
            i++;
        }
    }
    if(i == 0){ // if path has no arguments then reset path
        path[0] = NULL;
        path[1] = NULL;
    }
    else{
        for(int j = 0; j < i; j++){
            char new_arg[1024];
            strcpy(new_arg, args[j+1]); // copy don't assign; strings are pointers remember!
            if(new_arg[strlen(new_arg) - 1] != '/'){
                strcat(new_arg, "/");
            }
            path[j] = strdup(new_arg);
        }
    }
}

void builtInCommand(char *args[], int n){
    int i = 0;
    if(strcmp(args[0], "exit") == 0){ // if built-in command is exit
        exitHandler(args, n);
    }
    else if(strcmp(args[0], "cd") == 0){ // if built-in command is cd
        cdHandler(args, n);
    }
    else if(strcmp(args[0], "path") == 0){ // if built-in command is path
        pathHandler(args, n);
    }
}

int redirectionParser(char *args[], int argc){
    for(int i = 0; i < argc; i++){ // traverse arguments array
        if(i == 0 && strlen(args[i]) == 1){ // if there is no command before >
            error();
            return -1;
        }
        else if(strstr(args[i], ">") != NULL && strlen(args[i]) != 1){ // if args element contains >
            char* token = strtok(args[i], ">");
            while (token != NULL){
                args[i] = token;
                //printf("args[%d] = %s\n", i, args[i]);
                if(strlen(args[i]) > 0){
                    i++;
                }
                token = strtok(NULL, ">");
            }
            break;
        }
        else if(strstr(args[i], ">") != NULL){
            if(args[i+1] == NULL || args[i+2] != NULL){ // if there is not output file or more than 1 output file
                error();
                return -1;
            }
            strcpy(args[i], args[i+1]);
            args[i+1] = NULL;
            break;
        }
    }

    int count = 0;
    while(args[count] != NULL){
        count++;
    }
    if(count < 2){
        error();
        return -1;
    }
    return 0;
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

void executeCommand(char *args[]){
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

void redirectionHandler(char *args[]){
    char filename[1024];
    //char *updated_args[INPUT_MAX] = {NULL};
    int i = 0;
    while(args[i] != NULL){
        //printf("args[%d] = %s\n", i, args[i]);
        i++;
    }

    int saved_stdout = dup(STDOUT_FILENO);
    strcpy(filename, args[i-1]);
    args[i-1] = NULL;
    FILE *file = freopen(filename, "w", stdout); // reroutes stdout for main process
    dup2(fileno(stdout), fileno(stderr));
    executeCommand(args);
    fclose(file);
    fflush(stdout);
    dup2(saved_stdout, STDOUT_FILENO);
}

int checkShellScript(char *command){
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

void parseCommand(char line[]){
    char *args[INPUT_MAX] = {NULL};
    char *parallel_commands[INPUT_MAX] = {NULL};
    int i = 0;

    if(strstr(line, "&") != NULL){
        char* token = strtok(line, "&");
        while(token != NULL){
            parallel_commands[i] = token;
            //printf("parallel_command[%d] = %s\n", i, parallel_commands[i]);
            if(strlen(parallel_commands[i]) > 0){
                i++;
            }
            token = strtok(NULL, "&");
        }
        int j = 0;
        while(parallel_commands[j] != NULL){
            parseCommand(parallel_commands[j]);
            j++;
        }
    }
    else if(strstr(line, ">") != NULL){ // if > is present in input
        while((args[i] = strsep(&line, " ")) != NULL){
            if(strlen(args[i]) > 0){
                i++;
            }
        }
        if(redirectionParser(args, i) == 0){
            redirectionHandler(args);
        }
        
    }
    else{
        // Separate command arguments and adds them to args array!
        while((args[i] = strsep(&line, " ")) != NULL){
            if(strlen(args[i]) > 0){
                i++;
            }
        }

        if(i == 0){
            // whitespaces
        }

        // Handle built-in commands
        else if(strcmp(args[0], "exit") == 0 || strcmp(args[0], "cd") == 0 || strcmp(args[0], "path") == 0){
            int n = sizeof(args) / sizeof(args[0]);
            builtInCommand(args, n);
        }

        // Check shell command
        else if(checkShellScript(args[0]) == 0 && (path[0] == NULL || strcmp(path[0], "/bin/") == 0)){
            error();
        }

        // execute command
        else{
            executeCommand(args);
        }
    }
}

int main(int MainArgc, char *MainArgv[]){ 
    FILE *file;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    if(MainArgc > 2){ // Invalid argument count
        error();
        exit(1);
    }

    else if(MainArgc == 2){ // Batch case
        file = fopen(MainArgv[1], "r");
        if(file != NULL){
            while ((nread = getline(&line, &len, file)) != -1) {
                if(strlen(line) > 1){
                    if (line[nread - 1] == '\n') {
                        line[nread - 1] = '\0';
                    }
                    parseCommand(line);
                }
            }
        }
        else{
            error();
            exit(1);
        }
        //return(0);
    }

    else{
        while (1) // Interactive case
        {
            printf("witsshell> ");
            nread = getline(&line, &len, stdin);
            if(strlen(line) > 1){
                if (line[nread - 1] == '\n') {
                    line[nread - 1] = '\0';
                }
                parseCommand(line);
            }
        }
    }

    return(0);
}