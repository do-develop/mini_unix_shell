#include "shell.h"
#include <signal.h>
#include <sys/wait.h>

void claim_children(int signal);

int main(void)
{
	//Handle signals
	struct sigaction sa;
	sa.sa_handler = &claim_children;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDSTOP;
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL)!=0
	|| sigaction(SIGINT, &sa, NULL)!=0
	|| sigaction(SIGQUIT, &sa, NULL)!=0
	|| sigaction(SIGTSTP, &sa, NULL)!=0)
	{
		perror("Sigaction Error");
		exit(1);
	}
	
	//Variables for command line
    char line[CMD_LENGTH];
    command** cmd;
	
	//Get user input and loop program until user enters exit
	getInput(line);
    while(strcmp(trimspace(line),"exit")!=0)
    {
        cmd = process_cmd_line(line, 1);
		process_cmd(cmd);
        clean_up(cmd);
        getInput(line);
    }
    exit(0);
}

void claim_children(int signal)
{
	if(signal==SIGCHLD)
	{//Claim zombies
		while(waitpid(0,NULL,WNOHANG)>0); 
	}
	else
	{//Ignore SIGINT, SIGQUIT, SIGTSTP
		fflush(stdout);
	}
}