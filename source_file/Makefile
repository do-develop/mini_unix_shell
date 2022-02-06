minishell: shell.o main.o
	gcc -Wall -o minishell shell.o main.o

shell.o : shell.c shell.h
	gcc -Wall -c -o shell.o shell.c

main.o : main.c shell.h
	gcc -Wall -c -o main.o main.c

clean:
	rm -f *.o
	rm -f minishell

