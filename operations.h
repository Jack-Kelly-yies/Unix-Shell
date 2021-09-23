#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "parser.h"

// -----------------------------------------------------------READ ONLY

#define EXIT "exit"
#define CHANGE_DIRECTORY "cd"
#define PWD "pwd"
#define PROMPT "prompt"
#define CUSTOM_COMMAND_EXECUTED 2


// ----------------------------------------------------------- Global Variables

static char* stockShellCommands[] = {EXIT, CHANGE_DIRECTORY, PWD, PROMPT, NULL};

// ----------------------------------------------------------- initialization

void tidy(); 

// ----------------------------------------------------------- Executing commands 

int processCommands();
int processJobs(Command jobs[], int size);
int executeCommand(Command* command);
int shellCommands(Command* command);

// ----------------------------------------------------------- Executing Shell Custom Commands  

void pwd();

// ----------------------------------------------------------- Zombies / slow system calls


#endif