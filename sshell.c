#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	while (1) {
		// Display prompt
		printf("sshell$ ");
		// Read line from input
		// https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
		char userInput[512];
		fgets(userInput, 512, stdin);
		char *pos;
		if ((pos = strchr(userInput, '\n')) != NULL) {
			*pos = '\0';
		}
		else {
			perror("something goes wrong");
		}
		//add a comment
		//add a comment
		// Prase line
		char userInputCopy[512];
		strcpy(userInputCopy, userInput);
		int strLength = strlen(userInput);
		printf("strLength is: %d\n", strLength);
		int index = 0;
		int wordIndex = 0;
		char *target[16];

		// Remove all white space and tab to '\0'
		while(index < strLength) {
			if (userInput[index] == ' ' || userInput[index] == '\t') {
				userInput[index] = '\0';
			}
			index++;
		}

		index = 0;
		while(index < strLength) {
			while (userInput[index] == '\0' && index < strLength) {
				index++;
			}
			if (index >= strLength) {
				break;
			}
			target[wordIndex] = userInput + index;
			wordIndex++;
			while (userInput[index] != '\0' && index < strLength) {
				index++;
			}
		}


		target[wordIndex] = NULL;


		// Fork to handle test
		pid_t pid;
		int status;

		/*
		sshell$ &
		Error: invalid command line

		sshell$ toto
		Error: command not found
		+ completed 'toto' [1]

		sshell$ cd toto
		Error: no such directory

		sshell$ grep toto < tata
		Error: cannot open input file

		sshell$ cat <
		Error: no input file

		sshell$ echo hack > /etc/passwd
		Error: cannot open output file

		sshell$ echo >
		Error: no output file

		sshell$ cat file | grep toto < file
		Error: mislocated input redirection

		sshell$ echo Hello world! > file | cat file
		Error: mislocated output redirection

		sshell$ echo > file & | grep toto
		Error: mislocated background sign

		sshell$ exit
		Error: active jobs still running
		+ completed 'exit' [1]
		*/



		pid = fork();

		if (pid == 0) {
			// child
			execvp(target[0], target);
			//if (execvp(target[0], target) == -1){
			//	printf("ERROR: %s\n", strerror(errno));
			//}
			perror("execvp");
		} else if (pid > 0) {
			// parent
			waitpid(-1, &status, 0);
			fprintf(stderr, "+ completed '%s' [%d]\n", userInputCopy, WEXITSTATUS(status));
		} else {
			perror("fork");
			exit(1);

		}
	}
}
