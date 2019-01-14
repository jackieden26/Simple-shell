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
		char cmd[512];
		fgets(cmd, 512, stdin);
		char *pos;
		if ((pos = strchr(cmd, '\n')) != NULL) {
			*pos = '\0';
		}
		else {
			perror("something goes wrong");
		}
		// Prase line
			// Phase 4

		// Fork to handle test
		pid_t pid;
		int status;

		pid = fork();

		if (pid == 0) {
			// child
			// char *test[2] = {"date", NULL};
			// execvp(test[0], test);
			// perror("execvp");
			char *fileArgPtr[2] = {cmd, NULL};
			execvp(fileArgPtr[0], fileArgPtr);
			perror("execvp");
		} else if (pid > 0) {
			// parent
			waitpid(-1, &status, 0);
			fprintf(stderr, "+ completed '%s' [%d]\n", cmd, WEXITSTATUS(status));
		} else {
			perror("fork");
			exit(-1);

		}
	}
}
