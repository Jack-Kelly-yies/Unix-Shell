#include "parser.h"


// ------------------------------------- Takes command struct and Nullifies all variables
void wipeCommand(Command* c)
{
    for(int i =0; i<MAX_ARGS; i++)
    {
        c->args[i] = NULL;
    }

    c->sep = NULL;
    c->std_in = NULL;
    c->std_out = NULL;
}

// ------------------------------------- Prints currently stored prompt
void displayPrompt()
{
    printf("%s ", prompt);
	fflush(stdout); 
}

// ------------------------------------- Takes command line string for standard input, removes newline 
void getUserInput(char* shellInput, int size)
{

	int again = 1;
	char *linept;        // pointer to the line buffer

      while (again) {
           again = 0;
           linept = fgets(shellInput, size, stdin);
           if (linept == NULL) 
               if (errno == EINTR)
                    again = 1;        // signal interruption, read again
      }

}

// ------------------------------------- Takes string and removes first newline character
void removeNewline(char* string, int size)
{
    int i = 0;
	while(i <= size)
	{
		if(string[i] == '\n')
		{
			string[i] = '\0';
			break;
		}
		i++;
	}
}

// ------------------------------------- Takes new prompt string, if valid stores as global prompt
void updatePrompt(char* temp)
{
	int i = 0;

	while(temp[i] != '\0')
	{
		i++;
	}
	temp[i+1] = '\0';

	if(i > MAX_PROMPT)
	{
		printf("Prompt too big!\n");
		return;
	}
	else if(i <= 0)
	{
		printf("Prompt too small!\n");
		return;
	}

	for(int j = 0; prompt[j] != '\0'; j++)
	{
		prompt[j] = '\0';
	}

	for(int j = 0; j < i+1; j++)
	{
		prompt[j] = temp[j];
	}

}

// ------------------------------------- Tokenizes command line arguments
int tokenise(char* inputLine, char* token[])
{
	char* tk;
	int i = 0;

	tk = strtok(inputLine, SEP_TOKEN);
	token[i] = tk;

	while(tk != NULL)
	{
		i++;
		if(i >= MAX_TOKENS)
		{
			i = -1;
			break;
		}

		tk = strtok(NULL, SEP_TOKEN);
		token[i] = tk;
	}
	return i;
}

// ------------------------------------- Takes string and tests if exists in string array
int compareStrings(char* string, char* strings[])
{
    int i=0;
    while (strings[i] != NULL)
    {
        if (strcmp(strings[i], string) == 0)
        {
            return 1;
        }
        ++i;
    }
    return 0;
}

// ------------------------------------- initalizes prompt, prepares command struct array
void initializeCmdParser()
{
    prompt[0] = '$';
	prompt[1] = '\0';
	SIG_ignore();
    numOfCmds = 0;
    
    Command* c;
    for(int i = 0; i < MAX_COMMANDS; i++)
    {
        c = &(commands[i]);
        wipeCommand(c);
    }
}

// ------------------------------------- Ignores several signals
void SIG_ignore()
{
    signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
}



// ------------------------------------- displays prompt, reads users inputs
void readCommands(char* shellInput, int size)
{
    displayPrompt();
    getUserInput(shellInput, size);

}

// ------------------------------------- Takes command line string -> tokenizes -> populates command struct array
int parseCommands(char* shellInput)
{
    char* tmpCmdLine = strdup(shellInput);
    char* tokens[MAX_TOKENS];

    int numOfTokens = tokenise(tmpCmdLine, tokens); //---- splits into tokens 
    if(numOfTokens == ZERO)
    {
        //printf("No Tokens\n");
        return BAD_COMMAND;
    }

    int numOfCommands = separateCommands(tokens, numOfTokens); //---- splits into commands
    if(numOfCommands == ZERO)
    {
        //printf("Could not seperate commands\n");
        return BAD_COMMAND;
    }
    return CONT_FLAG;
}

// ------------------------------------- Takes tokenized input, combines and stores commands in command struct array
int separateCommands(char* token[], int size)
{
    int numOfTokens = size;
    char *CMD_Sep[] = 
	{
		SEP_PIPE,
		SEP_CON,
		SEP_SEQ,
		NULL
	};

    char *IO_Sep[] = 
	{
		STDIN_DIR,
		STDOUT_DIR,
		NULL
	};

    if(compareStrings(token[0], CMD_Sep)) // ----------- if first token & or ;, reject
    {
        return INVALID;
    }
    if(!compareStrings(token[numOfTokens-1], CMD_Sep)) // -------------- if last token isn't seperator, assume sequential
    {
        token[size] = SEP_SEQ;
        numOfTokens++;
    }

    int first = 0;
    int last = 0;
    char *sep = NULL;
    char *command = NULL;
    int argIndex = 0;
    Command* currentCommand;

    for(int i = 0; i < numOfTokens; i++)
    {

        last = i; 

        if(!compareStrings(token[i], CMD_Sep)) 
        {
            continue; 
        }

        currentCommand = &(commands[numOfCmds]);
        wipeCommand(currentCommand); 
        sep = token[i]; 

        if(first == last) 
        {
            return BAD_COMMAND;
        }

        currentCommand->sep = sep;
        argIndex = 0;

        for(int j = first; j <= last; j++) 
        {

            if(compareStrings(token[j], IO_Sep)) 
            {

                if(j == last || ((j+2) != last)) 
                {

                    printf("Invalid IO\n");
                    return BAD_COMMAND; 

                }

                setIO(currentCommand, token[j], token[j+1]);
                j = last;

				if(currentCommand->std_in != NULL)
				{
					appendIO(currentCommand);
				}
                break;


            }
            else if(!compareStrings(token[j], CMD_Sep)) 
            {
                currentCommand->args[argIndex] = token[j];
                argIndex++;
            }
			
        }
        if(isWildcard(currentCommand) == 1 && doesWildcardExist(currentCommand) == 1)
        {
            appendWildcard(currentCommand);
        }

        first = last + 1; 
        numOfCmds++;
    }
}

// ------------------------------------- implements glob for appending wildcard arguments
void appendWildcard(Command* c)
{

	glob_t wildcard;
	int i = 0;
	int indexWhereWildcardExists = 0;
	char* buffer[MAX_ARGS];
	int bufferLen;
	int wildcardArrayLen;
	int m;
	int f;

	while(c->args[i] != NULL)
	{
		bufferLen = 0;
		m = 0;
		f = 0;
		
		for(int k = 0; k < strlen(c->args[i]); k++)
		{
			if(!(c->args[i][k] == '?' || c->args[i][k] == '*'))
			{
				continue;
            }

			if(i<=0) 
			{
                continue;
            }

			int Len = 0; 
			while(c->args[Len] != NULL)
			{
				Len++;
			}
			
			m=i+1;
			int counter = m;
			while(m<Len)
			{
				
				buffer[m-counter] = c->args[m];
				m++;
				bufferLen++;
				
			}
			int check;
			check= glob(c->args[i], GLOB_ERR, NULL, &wildcard);
			while(wildcard.gl_pathv[check] != NULL)
			{
				
				check++;
				
			}
			for(int n = i; n<i+check; n++)
			{
				
				c->args[n] = wildcard.gl_pathv[n-i];
			}
			
			f = i + check;
			for(int z = f; z < i + check + bufferLen; z++)
			{
				c->args[z] = buffer[z-f];
			}
			
		}

		i++;
	}

}

// ------------------------------------- checks all command arguments for wildcard symbols
int isWildcard(Command* c)
{
	int i = 0;
	while(c->args[i] != NULL)
	{		
		for(int k = 0; k < strlen(c->args[i]); k++)
		{		
			if(c->args[i][k] == '?' || c->args[i][k] == '*')
			{		
				return 1;	
            }
        }
		i++;
	}
	return 0;
}

// ------------------------------------- checks all command arguments for wildcard symbols
int doesWildcardExist(Command* c)
{
	int i = 0;
	int check = 0;
	glob_t wildcard;
	while(c->args[i] != NULL)
	{		
		for(int k = 0; k < strlen(c->args[i]); k++)
		{		
			if(c->args[i][k] == '?' || c->args[i][k] == '*')
			{		
				check = glob(c->args[i], GLOB_ERR, NULL, &wildcard);

				if(check==0)
				{
					
					return 1;
					
					
				}

            }
        }
		i++;
	}
	return 0;
}

// ------------------------------------- appends filename as final command argument
int appendIO(Command* c)
{
	
	int argsSizeBeforeInput = 0;
	while(c->args[argsSizeBeforeInput] != NULL)
	{
		
		argsSizeBeforeInput++;
		
	}

    c->args[argsSizeBeforeInput] = c->std_in;

	return 0;

}

// ------------------------------------- depending on which I/O stores filename in command struct  
void setIO(Command* c, char* direction, char* file)
{
    if(strcmp(direction, STDOUT_DIR) == 0) 
    {
        c->std_out = file; 
    }
    else 
    {
        c->std_in = file; 
    }
}