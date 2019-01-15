#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>


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

		pid = fork();

		if (pid == 0) {
			// child
			// char *test[2] = {"date", NULL};
			// execvp(test[0], test);
			// perror("execvp");
			// char *fileArgPtr[2] = {userInput, NULL};
			execvp(target[0], target);
			perror("execvp");
		} else if (pid > 0) {
			// parent
			waitpid(-1, &status, 0);
			fprintf(stderr, "+ completed '%s' [%d]\n", userInputCopy, WEXITSTATUS(status));
		} else {
			perror("fork");
			exit(-1);

		}
	}
}
