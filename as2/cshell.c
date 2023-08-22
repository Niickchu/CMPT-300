#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cshell.h"
#include "commands.h"

regex_t regex;
int isRunning = 1;

static void tokenize(char* command, char** tokens){
    int i = 0;
    char* token = strtok(command, " ");
    while(token != NULL){
        tokens[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    tokens[i] = token;   //set last token to NULL for future use
} 

static void executeCommand(char** tokens){
    //check internal commmands

    if(strcmp(tokens[0], "exit") == 0){ //exit
        isRunning = exitShell();
    }
    else if(strcmp(tokens[0], "log") == 0){ //log
        printLog();
    }
    else if(strcmp(tokens[0], "print") == 0){ //print

        if(tokens[1] == NULL){
            printf("Error: no arguments\n");
            addLog(tokens[0], -1);
            return;
        }

        int i = 1;
        while(tokens[i] != NULL){
            printf("%s ", tokens[i]);
            i++;
        }
        addLog(tokens[0], 0);
        printf("\n");
    }
    else if(strcmp(tokens[0], "theme") == 0){   //theme
        changeTheme(tokens[1]);
    }
    else{
        int fd[2];
        if(pipe(fd) == -1){
            printf("Error: pipe creation failed\n");
            addLog(tokens[0], -1);
            return;
        }

        pid_t pid = fork();
        
        if(pid == 0){ // Child process
            close(fd[0]);

            dup2(fd[1], STDOUT_FILENO);
            dup2(fd[1], STDERR_FILENO);
            
            close(fd[1]);
            
            int x = execvp(tokens[0], tokens);
            if(x == -1){
                
                
                printf("Error: this command is not found");
                printf("\n");

                addLog(tokens[0], -1);
                exit(1);
            }
        }
        else if(pid > 0){ // Parent process
            close(fd[1]);

            char buff[2048];
            ssize_t bytes = read(fd[0], buff, sizeof(buff)-1);

            buff[bytes] = '\0';
            printf("%s", buff);
            close(fd[0]);
            
            int status;
            waitpid(pid, &status, 0);
            if(status == 0){
                addLog(tokens[0], 0);
            }
            else{
                addLog(tokens[0], -1);
            }
        }
        else{ // Fork failed
            printf("Error: fork failed\n");
            addLog(tokens[0], -1);
        }
    }
}

static void parseTokens(char** tokens){
    if (!regexec(&regex, tokens[0], 0, NULL, 0)) { //first check if assigning new env variable
        addLog(tokens[0], assignNewEnvVariable(tokens[0]));
        return;
    }
    
    for(int i = 0; tokens[i] != NULL; i++){ //loop through tokens and check for tokens that start with $
        if(tokens[i][0] == '$'){
            char* value = getEnvVarValue(tokens[i]); //check if variable exists
            if(value == NULL){
                printf("Error: variable does not exist\n");
                addLog(tokens[0], -1);
                return;
            }
            //replace token with value
            tokens[i] = value;
        }
    }
    executeCommand(tokens);
}

static void interactiveMode(){

    char line[MAX_COMMAND_LENGTH];
    char *tokens[MAX_NUM_ARGUMENTS + 1];

    while(isRunning){  //loop until exit command
        
        printf("cshell$ ");
        //get line from terminal
        printf(ANSI_WHITE);
        if (fgets(line, MAX_COMMAND_LENGTH, stdin) == NULL) {
            printf("Error reading user input.\n");
            continue;
        }

        setColour();

        //check if no input
        if (line[0] == '\n') {
            continue;
        }

        line[strcspn(line, "\n")] = '\0'; //remove newline

        tokenize(line, tokens);

        parseTokens(tokens);    //calls executeCommand()
    }

    regfree(&regex);
    exit(0);

}

static void scriptMode(char* fileName){
    FILE* file = fopen(fileName, "r");
    if(file == NULL){
        printf("Error: file does not exist\n");
        exit(1);
    }

    //read file line by line
    char line[MAX_COMMAND_LENGTH];
    char *tokens[MAX_NUM_ARGUMENTS + 1];

    
    while((fgets(line, MAX_COMMAND_LENGTH, file) != NULL) && isRunning){
        //check if no input
        if (line[0] == '\n') {
            continue;
        }

        line[strcspn(line, "\n")] = '\0';
        // printf("line: %s\n", line);

        tokenize(line, tokens);

        parseTokens(tokens); 
    }

    fclose(file);

    regfree(&regex);

    //exit shell
    if(isRunning){
        exitShell();
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    
    if (argc > 2) {
        printf("Error: too many arguments\n");
        exit(1);
    }

    regcomp(&regex, "^\\$[a-zA-Z0-9_$-]+=([a-zA-Z0-9_$-]+)$", REG_EXTENDED);

    if(argc == 2){ //script mode
        scriptMode(argv[1]);
    }
    else{ //interactive mode
        interactiveMode();
    }
}