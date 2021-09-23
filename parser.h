#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <glob.h>
#include <fcntl.h>

// -----------------------------------------------------------READ ONLY

#define ZERO 0
#define BAD_COMMAND 0
#define EXIT_SHELL 0
#define INVALID -3
#define CONT_FLAG 1

#define SEP_TOKEN " \t\n"
#define SEP_PIPE "|"
#define SEP_CON "&"
#define SEP_SEQ ";"

#define MAX_COMMANDS 100
#define MAX_PROMPT 256
#define MAX_TOKENS 1000
#define MAX_ARGS 1000

#define STDOUT_DIR ">"
#define STDIN_DIR "<"

// ----------------------------------------------------------- Command Struct
typedef struct
{
    char* args[MAX_ARGS];
    char* sep;
    char* std_in;
    char* std_out;
    
}Command;

// ----------------------------------------------------------- Global Variables
Command commands[MAX_COMMANDS];
int numOfCmds;
char prompt[MAX_PROMPT];

// ----------------------------------------------------------- initialization
void wipeCommand(Command* c);
void initializeCmdParser();
void SIG_ignore();

// ----------------------------------------------------------- User Input
void displayPrompt();
void getUserInput(char* commandline, int size);
void removeNewline(char* string, int size);
int compareStrings(char* token, char* values[]);
void readCommands(char* commandline, int size);
void updatePrompt(char* newPrompt);

// ----------------------------------------------------------- Parsing Commands
int tokenise(char* commandline, char* token[]);
int parseCommands(char* commandline);
int separateCommands(char* token[], int size);
void appendWildcard(Command* c);
int isWildcard(Command* c);
int doesWildcardExist(Command* c);
int appendIO(Command* c);
void setIO(Command* c, char* direction, char* file);

#endif