#include "shell.h"

int main()
{
    
    // prepare struct array
    initializeCmdParser();

    // run shell
    shellLoop();


    return 0;

}

void shellLoop()
{

    int readMore;


    while(readMore != ZERO)
    {

        readCommands(shellInput, MaxCmdLine); // read std in

        readMore = parseCommands(shellInput); // if valid parse

        readMore = processCommands(); // if valid parse

    }
}