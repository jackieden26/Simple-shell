default: sshell

sshell.o: sshell.c
	gcc -c -Wall -Werror sshell.c -o sshell.o

sshell: sshell.o
	gcc sshell.o -Wall -Werror -o sshell

clean:
	-rm -f sshell.o
	-rm -f sshell
