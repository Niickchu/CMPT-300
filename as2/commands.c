#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "commands.h"
#include "cshell.h"

Command* logHistory = NULL;
int numCommands = 0;

EnvVar* envVars = NULL;
int numEnvVars = 0;
int currentTheme = 0; //0 = white, 1 = red, 2 = green, 3 = blue

void printLog(){
    for(int i = 0; i < numCommands; i++){
        printf("%s %s %d\n", logHistory[i].time, logHistory[i].name, logHistory[i].returnValue);
    }
    addLog("log", 0);
}

void setColour(){
    if(currentTheme == 0){
        printf(ANSI_WHITE);
    }
    else if(currentTheme == 1){
        printf(ANSI_RED);
    }
    else if(currentTheme == 2){
        printf(ANSI_GREEN);
    }
    else{
        printf(ANSI_BLUE);
    }
}

void changeTheme(char* theme){
    if(strcmp(theme, "red") == 0){
        currentTheme = 1;
        printf(ANSI_RED);
    }
    else if(strcmp(theme, "green") == 0){
        currentTheme = 2;
        printf(ANSI_GREEN);
    }
    else if(strcmp(theme, "blue") == 0){
        currentTheme = 3;
        printf(ANSI_BLUE);
    }
    else if(strcmp(theme, "white") == 0){
        currentTheme = 0;
        printf(ANSI_WHITE);
    }
    else{
        printf("unsupported theme\n");
        addLog("theme", -1);
        return;
    }

    addLog("theme", 0);
}

void freeEnvVars(){
    free(envVars);
}

void freeLogHistory(){
    free(logHistory);
}

int exitShell(){
    printf("Bye!\n");
    addLog("exit", 0);
    freeEnvVars();
    freeLogHistory();
    return 0;
}

void addLog(char* command, int returnValue){
    Command newCommand;
    strcpy(newCommand.name, command);
    newCommand.returnValue = returnValue;

    time_t t;
    time(&t);
    strcpy(newCommand.time, ctime(&t));

    numCommands++;
    logHistory = realloc(logHistory, numCommands * sizeof(Command));
    logHistory[numCommands - 1] = newCommand;
}

int assignNewEnvVariable(char* token) {

    char* tokenName = strtok(token, "=");   //break token into name and value
    char* tokenValue = strtok(NULL, "=");

    // First check if variable already exists
    for (int i = 0; i < numEnvVars; i++) {
        if (strcmp(envVars[i].name, tokenName) == 0) {  //if variable exists, update value
            strcpy(envVars[i].value, tokenValue);
            return 0;
        }
    }

    // Variable does not exist, create new variable
    EnvVar newEnvVar;
    strcpy(newEnvVar.name, tokenName);
    strcpy(newEnvVar.value, tokenValue);

    numEnvVars++;
    envVars = realloc(envVars, numEnvVars * sizeof(EnvVar));
    envVars[numEnvVars - 1] = newEnvVar;
    
    return 0;
}

char* getEnvVarValue(char* name){
    for(int i = 0; i < numEnvVars; i++){
        if(strcmp(envVars[i].name, name) == 0){
            return envVars[i].value;
        }
    }
    return NULL;
}