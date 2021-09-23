#include "operations.h"

// ------------------------------------- Nullifies all commands in command struct
void tidy()
{
    for(int i = 0; i < numOfCmds; i++)
    {
        wipeCommand(&(commands[i]));
    }
    numOfCmds = 0;
}

// ------------------------------------- Handles execution of all commands in command struct array 
int processCommands()
{
    Command jobs[MAX_COMMANDS];
    int numOfPipes = 0;
    int status = 1;
    for(int i = 0; i < numOfCmds; i++)
    {
        if(strcmp(commands[i].sep, SEP_PIPE) == 0) 
        {
            jobs[numOfPipes] = commands[i]; 
            numOfPipes++;
        }
        else if(numOfPipes > 0) 
        {
            jobs[numOfPipes] = commands[i]; 
            numOfPipes++; 
            status = processJobs(jobs, numOfPipes);
            numOfPipes = 0;
        }
        else
        {
            status = executeCommand(&(commands[i]));
        }

        if(status == ZERO)
        {
            return ZERO;
        }
    }
    tidy();
    return CONT_FLAG;
}

// ------------------------------------- handles jobs, commands connected by pipes
int processJobs(Command jobs[], int size)
{
    pid_t pid;
    int p[2];
    int pipe_in = 0;
    int status;

    for(int i = 0; i < size; i++)
    {
        pipe(p);
        pid = fork();

        if(pid == 0) // ---- if process is a child
        {
            dup2(pipe_in, 0); // ---- set STDIN as message from previous process 

            if((i+1) < size || jobs[i].std_out != NULL)
            {
                dup2(p[1], 1); 
            }

            close(p[0]);
            execvp(jobs[i].args[0], jobs[i].args);
            printf("failed to execute command\n");

        }
        else if(pid > 0) // ---- if process is a parent
        {

            if(jobs[i].std_out != NULL)
            {

                close(p[1]);
                int size, fileDescriptor;
                char input[MAX_ARGS];

                if((fileDescriptor = open(jobs[i].std_out, O_WRONLY)) == -1) 
                {
                    FILE* fp = fopen(jobs[i].std_out, "w"); 
                    fclose(fp);
                    fileDescriptor = open(jobs[i].std_out, O_WRONLY); 
                }

                size = read(p[0], input, MAX_ARGS);
                write(fileDescriptor, input, size);
                close(fileDescriptor);
                close(p[0]);

            }
            else if(strcmp(jobs[i].sep, SEP_SEQ) == 0)
            {

                wait(NULL);

            }
            else
            {

                close(p[1]); 
                pipe_in = p[0];

            }
            if(strcmp(jobs[i].sep, SEP_SEQ) == 0)
            {

                wait(NULL);

            }
        }
        else if(pid == -1) // ---- failed fork
        {

            exit(-1);

        }
    }
    return CONT_FLAG;
}

// ------------------------------------- Handles execution of a single command
int executeCommand(Command* command)
{

    pid_t pid;
    int p[2]; 
    int pipe_in = 0;

    if(command->std_out != NULL)
    {

        pipe(p);

    }
    pid = fork();
    
    if(pid == 0) //---- if process is a child
    {

        if(!compareStrings(command->args[0], stockShellCommands)) //---- if not built in command
        {

            if(command->std_out != NULL) //---- if we require file output
            {

                close(p[0]);
                dup2(p[1], 1); //---- pipe to standard output
                
                execvp(command->args[0], command->args);
                printf("failed to execute command\n");

            }
            else
            {
                
                execvp(command->args[0], command->args);
                printf("failed to execute command\n");

            }
           
        }

        exit(0);     
    }
    else if(pid > 0) //---- if process is a parent
    {

        if(command->std_out != NULL) //---- if we require file output
        {

            close(p[1]);
            int size, fileDescriptor;
            char input[MAX_ARGS];

            if((fileDescriptor = open(command->std_out, O_WRONLY)) == -1) //---- if file not found 
            {

                FILE* fp = fopen(command->std_out, "w");
                fclose(fp);
                fileDescriptor = open(command->std_out, O_WRONLY);

            }

            size = read(p[0], input, MAX_ARGS);
            write(fileDescriptor, input, size);
            close(fileDescriptor);
            close(p[0]);

        }
        switch(shellCommands(command)) //---- checks if command requires custom execution 
        {
            case EXIT_SHELL: 
            {
                return EXIT_SHELL;
            }break;
            case CUSTOM_COMMAND_EXECUTED:
            {
                return CONT_FLAG;
            }break;
            default:
            {
                if(strcmp(command->sep, SEP_SEQ) == 0)  //------------------------------ sequential seperator
                {

                    wait(NULL);

                }
            }break;
        }
    }
    else if(pid == -1) //--------------------- failed fork
    {

        exit(-1);

    }
    return CONT_FLAG;
}

// ------------------------------------- implements getcwd() and prints current working directory directory 
void pwd()
{
	char path[1024];

	if(getcwd(path, sizeof(path)) == NULL)
	{

		printf("Directory Path Too Large\n");

	}
	else
	{

		printf("%s\n", path);

	}
}

// ------------------------------------- Checks if command is a built in command, if so executes, else continues
int shellCommands(Command* command)
{
    if(!strcmp(command->args[0], EXIT)) //---- If exit command, exit shell
    {
        return EXIT_SHELL;
    }
    else if(!strcmp(command->args[0], PWD)) //---- If PWD command, execute
    {
        pwd();
        return CUSTOM_COMMAND_EXECUTED;
    }
    else if(!strcmp(command->args[0], CHANGE_DIRECTORY)) //---- If CD
    {
        char* file = command->args[1];
        if(file == NULL)
        {
            chdir(getenv("HOME")); 
        }
        else
        {
            chdir(command->args[1]);
        }
        return CUSTOM_COMMAND_EXECUTED;
    }
    else if(!strcmp(command->args[0], PROMPT)) //---- If prompt
    {
        if(command->args[1] != NULL)
        {
            updatePrompt(command->args[1]);
            return CUSTOM_COMMAND_EXECUTED;
        }
    }
    
    return CONT_FLAG; //---- command is not built in
}

