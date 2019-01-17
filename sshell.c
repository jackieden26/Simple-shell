#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
/*
struct Command {
	char *args[16];
};
*/
int main(int argc, char *argv[])
{
	while (1) {
		// Display prompt
		//struct Command cmd[512];
		// Read line from input
		// https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
		char userInput[512];
		char *pos;
		// Prase line
		char userInputCopy[512];
		int index = 0;
		int wordIndex = 0;
		char *target[16];
		pid_t pid;
		int status;
		char buf[256];
		//char pre_cd[256];
		//int i;

		printf("sshell$ ");
		fgets(userInput, 512, stdin);
		if ((pos = strchr(userInput, '\n')) != NULL) {
			*pos = '\0';
		}
		else {
			perror("something goes wrong");
		}
		//printf("strLength is: %d\n", strLength);
		strcpy(userInputCopy, userInput);
		int strLength = strlen(userInput);
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
		fprintf(stderr,"target %s\n",*target);
		//print exit
		if (strcmp(*target, "exit") == 0) {
			fprintf(stderr, "Bye...\n");
			exit(0);
		}
		//print pwd
		else if ((strcmp(*target, "pwd") == 0)) {
			getcwd(buf,sizeof(buf));
			fprintf(stderr,"%s\n",buf);
			waitpid(-1, &status, 0);
			fprintf(stderr, "+ completed '%s' [%d]\n", userInputCopy, WEXITSTATUS(status));
			continue;
		}
		else if ((strcmp(*target, "cd") == 0)) {
			//problems with this part
			//char arg[2] = '..';
			//chdir('.');
			continue;
		}


		pid = fork();
		if (pid == 0) {
			// child
			execvp(target[0], target);
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

void parseArg(char *target[]) {

}

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
