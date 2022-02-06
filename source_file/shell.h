#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

/*
 * shell.h
 * author: Doyoung Oh, Ming Shun Lee
 * date: 31/03/2021
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define CMD_LENGTH 1000
#define MAX_VAR_LENGTH 100
//0x20 space ' ' 0x09 tab '\t' 0x00 null '\0'
static const char white_space[3] = {(char)0x20, (char)0x09, (char)0x00};

/* The structure for the commands */
typedef struct Command_struct
{
    char* com_name;
    char** argv;
    char* redirect_in; // <
    char* redirect_out; // >
    int background; // &
    int pipe_to; // |
	int sequential; // ;
} command;

/* Function prototypes */
//==================================================================
/*Command execution methods*/
//==================================================================
/**
 * @brief Backbone method that handles the array of command struct
 * @param array of command struct after tokenisation
 * @return none
 */
void process_cmd(command** cmd);

/**
 * @brief Executes the ouput redirected commands
 * @param a command struct
 * @return none
 */
void execute_redir_out_cmd(command* c);

/**
 * @brief Executes the input redirected commands
 * @param a command struct
 * @return none
 */
void execute_redir_in_cmd(command* c);

/**
 * @brief Executes the piped commands
 * @param an array of command struct
 * @return none
 */
void execute_piped_cmd(command** cmd);

/**
 * @brief Executes the background commands
 * @param an array of command struct
 * @return none
 */
void execute_background_cmd(command** cmd);

/**
 * @brief if the command is internally implemented the method executes
			the command then returns 1, if not it will return 0
 * @param a command struct
 * @return boolean value (0 or 1)
 */
int is_internal_cmd(command* c);

//==================================================================
/*Unitities*/
//==================================================================
/**
 * @brief Get user command line input
 * @param line of string
 * @return none
 */
void getInput(char* line);

/**
 * @brief removes leading and trailing spaces of a string
 * @param string
 * @return none
 */
char* trimspace(char *str);
/**
 * @brief free dynamically allocated memory
 * @param cmd, array of ptr to command structure to be freed
 * @return none
 */
void clean_up(command** cmd);

//==================================================================
/*Parsing methods*/
//==================================================================
/**
 * @brief searches for &, |, and ; delimiters and divide strings into
 *			each command tokens
 * @param command token and and integer that indicate wheter the function
			has called recursively.
 * @return array of command struct
 */
command** process_cmd_line(char* cmd, int );

/**
 * @brief searches for input, output redirection character
 *          then store the information in the Command_struct
 * @param command token and a command struct to store the result of this function
 */
void process_redirection(char* cmd, command* result);

/**
 * @brief simple tokenising function. no special character interpretation.
 * @param cmd - the string to be processed
 * @param result - the command struct to store the results in
 */
void process_simple_cmd(char* cmd, command* result);


#endif // PARSER_H_INCLUDED
