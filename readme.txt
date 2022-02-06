==================================================================================
=============================== Mini Unix Shell ==================================
==================================================================================
Author: Doyoung Oh
Files included:
- main.c – execution of program
- Makefile – to automate compilation using command ‘make’
- minishell – executable of this program
- shell.c – parsing input, processing command, and utilities
- shell.h - header file for declaration of function prototypes
==================================================================================
============================ What this program does ==============================
==================================================================================
• The program uses heap memory to store user line input and command line arguments. 
  Thus, it can allow for more than 100 commands in a command line and more than 1000 
  arguments in each command as long as the memory allocation does not fail.
• The program takes in user input and parses the input line into separate commands 
  using ‘|’, ‘;’, and ‘&’ as delimiters
• The redirection delimiters ‘<’, ‘>’ tokenize the command lines even further and 
  stores the file names of where the redirected input or output will be placed.
• Shell program shell prompt (default%) is reconfigurable. Users can change the prompt 
  of their choice
• Command pwd will prints out the current working directory of the shell process
• It will perform directory walk with command cd and will move to a designated directory 
  if a path is provided by the user. If a path is not provided, the shell program will 
  move to the home directory of the user
• It processes commands containing wildcard characters (such as *  or ?) by treating 
  these wildcard characters as a filename and process files with the matching formats 
  (i.e., if ls *.c is entered, the shell will locate all .c format files and execute ls 
  on these matching files)
• Shell pipeline | 
• It can perform synchronous background jobs when the ‘&’ is provided in a command
• It is able to perform sequential jobs when ‘;’ is provided in a command
• It is able to inherit its environment from the parent process
• It will terminate only when the built-in command ‘exit’ is entered by the user
• The program ignores the SIGINT, SIGQUIT, SIGTSTP as per the assignment requirement
