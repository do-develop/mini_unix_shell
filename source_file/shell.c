/*
 * shell.c
 * author: Doyoung Oh, Ming Shun Lee
 * date: 31/03/2021
 */
#include "shell.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h> //isspace()
#include <fcntl.h>
#include <glob.h>
#include <errno.h> // errno

static char prompt_var[MAX_VAR_LENGTH] = "myshell> ";
static int cmd_cnt = 0;

//===========================================================================
// Tokenising methods
//===========================================================================
void process_simple_cmd(char* cmd, command* result)
{
    char* token;//dividing character
    int ptr_cnt = 1; 
	
    //if there is no whitespace, there is no argument
    if((token = index(cmd, white_space[0]))==NULL
       && (token = index(cmd, white_space[1]))==NULL)
    {
        result->com_name = strdup(cmd);
        result->argv = realloc((void*)result->argv, sizeof(char*));
        result->argv[0] = strdup(cmd);
    }
    else
    {
        token = strtok(cmd, white_space);
        result->com_name = strdup(token);
        result->argv = realloc((void*)result->argv, sizeof(char *));
        result->argv[0] = strdup(token);

        while((token=strtok(NULL, white_space)) != NULL)
        {
            result->argv = realloc((void*) result->argv, (ptr_cnt+1)*sizeof(char*));
            result->argv[ptr_cnt] = strdup(token);
            ptr_cnt++;
        }
    }
    //set the final array element NULL
    result->argv = realloc((void*) result->argv, (ptr_cnt+1)* sizeof(char*));
    result->argv[ptr_cnt] = NULL;
}

void process_redirection(char* cmd, command* result)
{
    char* token, *missed_tkn;
    char* simple_cmd = NULL;

    //if no redirection found, only simple command present
    if((token=index(cmd, '<'))==NULL)
    {
        if((token=index(cmd, '>'))==NULL)
        {
            process_simple_cmd(cmd, result);
            result->redirect_in = NULL;
            result->redirect_out = NULL;
        }
        else// '>' output redirection exist
        {
            token = strtok(cmd,">");
            simple_cmd = strdup(token);

            token = strtok(NULL, "\0");
            process_simple_cmd(simple_cmd, result);
            result->redirect_out = strdup(token);
        }
    }
    else // '<' input redirection exist
    {
        token = strtok(cmd, "<");
        simple_cmd = strdup(token);
        token = strtok(NULL, "\0");

        //check if output redirection missed because input checked first
        if((missed_tkn=index(simple_cmd,'>'))!=NULL)
            process_redirection(simple_cmd, result); //before '>' delimiter
        if((missed_tkn=index(simple_cmd,'<'))!=NULL)
            process_redirection(token,result); //after '>' delimiter

        process_simple_cmd(simple_cmd,result);
        result->redirect_in = strdup(token);
    }

    free(simple_cmd);
}

command** process_cmd_line(char* cmd, int new)
{
    char* token, *missed_tkn;
    char* token_copy = NULL;
    static command** cmd_line;
    static int tkn_cnt;

    //ensure statics are null when not recursively called
    if(new==1)
    {
        tkn_cnt = 0;
        cmd_line = NULL;
    }

    if((token = index(cmd, ';'))==NULL)
    {
        if((token = index(cmd, '&'))==NULL) 
        {
			if((token = index(cmd, '|'))==NULL) //no '&', '|', ';' inside the cmd
			{
				if(trimspace(cmd)==NULL) return NULL; //prevent segmentation fault from blank input
				
				cmd_line = realloc(cmd_line, (tkn_cnt+1)*sizeof(command*));
				if(cmd_line==NULL) //realloc check error
				{
					perror("memory allocation error");
					exit(-1);
				}

				cmd_line[tkn_cnt] = malloc(sizeof(command));
				if(cmd_line[tkn_cnt]==NULL) //malloc error check
				{
					perror("memory allocation error");
					exit(-1);
				}
				
				//Initialise the new struct
				cmd_line[tkn_cnt]->com_name = NULL;
				cmd_line[tkn_cnt]->argv = NULL;
				cmd_line[tkn_cnt]->redirect_in = NULL;
				cmd_line[tkn_cnt]->redirect_out = NULL;
				cmd_line[tkn_cnt]->background = 0;
				cmd_line[tkn_cnt]->pipe_to = 0;
				cmd_line[tkn_cnt]->sequential = 0;

				process_redirection(cmd, cmd_line[tkn_cnt]);
				tkn_cnt++;
			}
			else
			{
				token = strtok(cmd, "|");
				token = strtok(NULL, ""); // store next token
				
				cmd_line = realloc((void*)cmd_line, (tkn_cnt+1)*sizeof(command*));
				cmd_line[tkn_cnt] = calloc(1, sizeof(command));
				cmd_line[tkn_cnt]->pipe_to = tkn_cnt+1;
				process_redirection(cmd, cmd_line[tkn_cnt]);
				tkn_cnt++;
				if(token != NULL)
					process_cmd_line(token,0);
			}
        }
        else // '&' was found
        {
			token = strtok(cmd, "&");
			token_copy = strdup(token);
			token = strtok(NULL, ""); // store next token
			if((missed_tkn = index(token_copy, '|'))!=NULL) // '|' exists before '&'
			{
				process_cmd_line(token_copy, 0);
			}
			else
			{
				cmd_line = realloc((void*)cmd_line, (tkn_cnt+1)*sizeof(command*));
				cmd_line[tkn_cnt] = calloc(1, sizeof(command));
				cmd_line[tkn_cnt]->background = tkn_cnt+1;
				process_redirection(cmd, cmd_line[tkn_cnt]);
				tkn_cnt++;
			}
            if(token != NULL)
                process_cmd_line(token,0);
        }
    }
    else // ';' found
    {
        token = strtok(cmd, ";");
        token_copy = strdup(token);
        token = strtok(NULL, ""); //get the token after ';'
        if((missed_tkn = index(token_copy, '|'))!=NULL ||
			(missed_tkn = index(token_copy, '&'))!=NULL) // '|' '&' exists before ';'
        {
            process_cmd_line(token_copy, 0);
        }
        else
        {
            cmd_line = realloc((void*)cmd_line, (tkn_cnt+1)*sizeof(command*));
            cmd_line[tkn_cnt] = calloc(1,sizeof(command));
			cmd_line[tkn_cnt]->sequential = tkn_cnt+1;
            process_redirection(cmd, cmd_line[tkn_cnt]);
            tkn_cnt++;
        }
        if(token != NULL) //process next token recursively
            process_cmd_line(token,0);
    }
    cmd_line = realloc((void*)cmd_line, (tkn_cnt+1)*sizeof(command*));
    cmd_line[tkn_cnt] = NULL;

    free(token_copy);

    return cmd_line;
}


//===========================================================================
// Command execution methods
//===========================================================================
void execute(command* c)
{
	//wildcard variables
	int idx=0, globidx=0, listidx=0;
	char* wildcard;
	glob_t globlist;
	char* arglist[CMD_LENGTH];
	
	while(c->argv[idx]!=NULL)
	{
		if((wildcard = index(c->argv[idx],'*'))!=NULL 
		|| (wildcard = index(c->argv[idx],'?'))!=NULL)
		{
			if(glob(c->argv[idx], GLOB_PERIOD, NULL, &globlist)==GLOB_NOSPACE
			|| glob(c->argv[idx], GLOB_PERIOD, NULL, &globlist)==GLOB_NOMATCH
			|| glob(c->argv[idx], GLOB_PERIOD, NULL, &globlist)==GLOB_ABORTED)//error check
			{
				exit(EXIT_FAILURE); 
			}
			while(globidx<globlist.gl_pathc)
			{
				arglist[listidx] = globlist.gl_pathv[globidx];
				++listidx;
				++globidx;
			}
			idx++;
		}
		else
		{
			arglist[listidx] = c->argv[idx];
			listidx++;
			idx++;
		}
	}
	arglist[listidx] = NULL;
	
	if(execvp(c->com_name, arglist) == -1)
	{
		perror("command execution error\n");
		exit(EXIT_FAILURE);
	}
}

void execute_redir_in_cmd(command* c)
{
	pid_t pid; 
	int infile = open(trimspace(c->redirect_in), O_RDONLY); 
	
	if(infile != -1)
	{
		switch(pid=fork())
		{
			case -1:
				perror("fork() error\n");
				break;
			case 0:
				dup2(infile, STDIN_FILENO); // closes previous fd 1
				close(infile);
				execvp(c->com_name, c->argv);
				perror("command execution error");
				break;
			default:
				close(infile);
				waitpid(pid,NULL,0);
				break;
		}
	}
	else
	{
		printf("File \'%s\' open() error\n", trimspace(c->redirect_in));
	}
}

void execute_redir_out_cmd(command* c)
{
	pid_t pid;
	int outfile = open(trimspace(c->redirect_out), O_WRONLY | O_CREAT | O_TRUNC, 0666); 
	
	if(outfile != -1)
	{
		switch(pid=fork())
		{
			case -1:
				perror("fork() error");
				break;
			case 0:
				dup2(outfile, STDOUT_FILENO); 
				close(outfile);
				execvp(c->com_name, c->argv);
				perror("command execution error");
				break;
			default:
				close(outfile);
				waitpid(pid,NULL,0);
				break;
		}
	}
}

void execute_piped_cmd(command** cmd)
{
	//find number of commands to be piped
	int pipe_count = 1; 
	int cnt = cmd_cnt;
	while(cmd[cnt]->pipe_to != 0)
	{
		pipe_count++;
		cnt++;
		if(cmd[cnt]==NULL) //if the command ended with '|' prevent segmentation fault
		{
			pipe_count--;
			break;
		}
	}
	
	int infile, outfile;
	int fd[pipe_count][2];
	pid_t pid;
	
	for(int i=0; i<pipe_count; ++i)
	{
		if(i != pipe_count-1) //not the last command
		{
			if(pipe(fd[i])<0)
				perror("pipe() error");
		}
		switch(pid=fork())
		{
			case -1:
				perror("fork() error");
				break;
			case 0:
				if(i != pipe_count-1)
				{
					dup2(fd[i][1],STDOUT_FILENO);
					close(fd[i][0]);
					close(fd[i][1]);
				}
				if(i != 0)
				{
					dup2(fd[i-1][0],STDIN_FILENO);
					close(fd[i-1][1]);
					close(fd[i-1][0]);
				}
				if (cmd[cmd_cnt]->redirect_in != NULL)
				{
					infile = open(trimspace(cmd[cmd_cnt]->redirect_in), O_RDONLY); 
					if(infile != -1)
					{
						dup2(infile, STDIN_FILENO); // closes previous fd 1							
						close(infile);
					}
				}
				else if (cmd[cmd_cnt]->redirect_out != NULL)
				{
					outfile = open(trimspace(cmd[cmd_cnt]->redirect_out), O_WRONLY | O_CREAT | O_TRUNC, 0666); 
					if(outfile != -1)
					{
						dup2(outfile, STDOUT_FILENO); 
						close(outfile);
					}
				}
				execute(cmd[cmd_cnt]);
				break;
			default:
				if(i != 0)
				{
					close(fd[i-1][0]);
					close(fd[i-1][1]);
				}
				wait(NULL);
		}
		cmd_cnt++;
	}
	
}

void execute_background_cmd(command** cmd)
{
	//find number of commands to be run in background
	int infile, outfile; //in case of redirection
	int ampersand_count=1;//counts number of &
	int cnt = cmd_cnt;//counts number of command
	int i; // for for-loop
	pid_t pid;
	
	while(cmd[cnt]->background != 0)
	{
		ampersand_count++;
		cnt++;
		if(cmd[cnt]==NULL) //if the command ended with '&' prevent segmentation fault
		{
			ampersand_count--;
			break;
		}
	}
	
	for(i=0; i<ampersand_count; i++)
	{
		switch(pid=fork())
		{
			case -1:
				perror("fork() error");
				break;
			case 0:
				if (cmd[cmd_cnt]->redirect_in != NULL)
				{
					infile = open(trimspace(cmd[cmd_cnt]->redirect_in), O_RDONLY); 
					if(infile != -1)
					{
						dup2(infile, STDIN_FILENO); // closes previous fd 1							
						close(infile);
					}
				}
				else if (cmd[cmd_cnt]->redirect_out != NULL)
				{
					outfile = open(trimspace(cmd[cmd_cnt]->redirect_out), O_WRONLY | O_CREAT | O_TRUNC, 0666); 
					if(outfile != -1)
					{
						dup2(outfile, STDOUT_FILENO); 
						close(outfile);
					}
				}
				execute(cmd[cmd_cnt]);
				break;
		}
		cmd_cnt++;
	}
}

int is_internal_cmd(command* c)
{
	if(strcmp("prompt", c->argv[0])==0)
	{
		strcpy(prompt_var, c->argv[1]);
		strcat(prompt_var, " ");
		return 1;
	}
	if(strcmp("pwd", c->argv[0])==0)
	{
		char cwd[256];
		if(getcwd(cwd, sizeof(cwd)) == NULL)
		{
			perror("getcwd() error");
		}
		else
		{
			printf("Currrent working directory: %s\n", cwd);
		}
		return 1;
	}
	if(strcmp("cd", c->argv[0])==0)
	{
		if(c->argv[1] == NULL)
		{
			chdir(getenv("HOME"));
		}
		else
		{
			if(chdir(c->argv[1]) == -1)
			{
				printf("Directory %s not found\n", c->argv[1]);
			}
		}
		return 1;
	}
	
	return 0;
}

//below executes regular command
void execute_cmd(command* c)
{
	int pid;
	int status;
	
	pid = fork();
	if(pid < 0)
	{
		perror("fork fail\n");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0) //in child
	{
		execute(c);
	}
	else //in parent
	{
		waitpid(pid, &status, 0);
	}
}

//BACKBONE METHOD THAT PROCESSES COMMANDS
void process_cmd(command** cmd)
{
	while(cmd[cmd_cnt]!=NULL)
	{
		if (cmd[cmd_cnt]->redirect_in != NULL)
		{
			execute_redir_in_cmd(cmd[cmd_cnt]);
			cmd_cnt++;
		}
		else if (cmd[cmd_cnt]->redirect_out != NULL)
		{
			execute_redir_out_cmd(cmd[cmd_cnt]);
			cmd_cnt++;
		}
		else if (cmd[cmd_cnt]->sequential != 0)
		{
			execute_cmd(cmd[cmd_cnt]);
			cmd_cnt++;
		}
		else if (cmd[cmd_cnt]->background != 0)
			execute_background_cmd(cmd);
		else if (cmd[cmd_cnt]->pipe_to != 0)
			execute_piped_cmd(cmd);
		else if(!is_internal_cmd(cmd[cmd_cnt])) //check if the command is not internally implemented command
		{
			execute_cmd(cmd[cmd_cnt]);
			cmd_cnt++;
		}
		else
		{
			cmd_cnt++;
		}
	}
	cmd_cnt = 0; //static variable set to 0 before next loop
}


//===========================================================================
// Utilities
//===========================================================================

char* trimspace(char *str)
{
	char *end;

	// Trim leading space
	while(isspace(*str)) str++;

	if(*str == 0)  // All spaces?
		return NULL;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char)*end)) end--;
	//place new null terminator character
	end[1] = '\0';
	
	return str;
}

void getInput(char* line)
{
	int again = 1;
	char* eol;
	
	while(again)
	{
		again = 0;
		printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET, prompt_var);
		fgets(line, CMD_LENGTH, stdin);
		
		if(trimspace(line) == NULL)
		{
			if(errno == EINTR) continue;
			again = 1; //upon signal interruption, read again
		}
		else
		{
			if((eol = index(line, '\n')) > 0)
			{
				*eol = '\0';
			}
		}
	}
}

void clean_up(command** cmd)
{
    int ptr_cnt=0;
    int arg_cnt=0;

    while(cmd[ptr_cnt] != NULL)
    {
        arg_cnt = 0;
        if(cmd[ptr_cnt]->com_name != NULL)
            free(cmd[ptr_cnt]->com_name); //free com_name
        if(cmd[ptr_cnt]->argv != NULL)
        {
            while(cmd[ptr_cnt]->argv[arg_cnt] != NULL)
            {
                free(cmd[ptr_cnt]->argv[arg_cnt]); //free argv[]
                arg_cnt++;
            }
            free(cmd[ptr_cnt]->argv); //free argv
        }
        if(cmd[ptr_cnt]->redirect_in != NULL)
            free(cmd[ptr_cnt]->redirect_in);
        if(cmd[ptr_cnt]->redirect_out != NULL)
            free(cmd[ptr_cnt]->redirect_out);
        //free one Command_structure[]
        free(cmd[ptr_cnt]);
        ptr_cnt++;
    }
    free(cmd); // free the command structure
    cmd = NULL;
    return;
}

