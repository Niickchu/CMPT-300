#ifndef INTERNALCOMMANDS_H
#define INTERNALCOMMANDS_H

#define MAX_NAME_LENGTH 32
#define MAX_VALUE_LENGTH 32
#define MAX_COMMAND_LENGTH 1024

#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_WHITE   "\x1b[0m"

typedef struct {
    char name[MAX_COMMAND_LENGTH];
    char time[50];
    int returnValue;
} Command;

typedef struct {
    char name[MAX_NAME_LENGTH];
    char value[MAX_VALUE_LENGTH];
} EnvVar;

void printLog();    
void setColour();   
void changeTheme(char* theme);  
int exitShell();    
void addLog(char* command, int returnValue);    
int assignNewEnvVariable(char* token);  
char* getEnvVarValue(char* name);

#endif