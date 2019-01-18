#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>

struct Input {
	struct Cmd command;
	bool background;
};

struct Cmd {
	char *exec;
	char *args[16];
	char *inputFile;
	char *outputFile;
};

int main(int argc, char *argv[])
{
	while (1) {
		//struct Command cmd[512];
		// https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
		char userInput[512];
		char *pos;
		char userInputCopy[512];
		int index = 0;
		int wordIndex = 0;
		char *target[16];
		pid_t pid;
		int status,i,count;
		char buf[256],pre_cd[256];
		char *array[100];
		char *string = " ";
		struct Input user_input;
		struct Cmd command;
		char *

		//char *path;
		//int fd;

		printf("sshell$ ");

		//read userinput
		fgets(userInput, 512, stdin);
		if ((pos = strchr(userInput, '\n')) != NULL) {
			*pos = '\0';
		}
		else {
			perror("something goes wrong");
		}
		strcpy(userInputCopy, userInput);
		int strLength = strlen(userInput);

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

		//put userinput into struct
		for (i = 0; i < strlen(userInputCopy); i++) {
			if (strcmp(userInputCopy[i],"|")==0) {

			}
		}

		//fprintf(stderr,"target %c\n",userInput[strlen(userInput)-1]);

		//tokenize userinput and store in array (an array of strings)

		if (strstr(userInputCopy,"<") != NULL) {
			string = "<";
		}
		else if (strstr(userInputCopy,">") != NULL) {
			string = ">";
		}

		i = 0;
		count= 0;
		char *p = strtok (userInputCopy, string);

		while (p != NULL) {
			array[i++] = p;
			p = strtok (NULL, string);
			count++;
		}


		//printf("count %d\n",count);
		/*
		//input redirection
		if ((count == 1) && (strcmp("<",string) == 0)) {
			fprintf(stderr,"Error: no input file\n");
			continue;
		}


		else if (strcmp("<",string) == 0) {
			for (i = 0;i<count;i++) {
				if (i==(count-1)) {
					getcwd(buf,sizeof(buf));
					path = malloc(strlen(buf)+strlen(array[i])+2);
					path[0] = '\0';
					strcat(path,buf);
					strcat(path,"/");
					strcat(path,array[i]);
					//printf("%s \n",path);
					fd = open(path,O_RDWR);
					if (fd<0) {
						fprintf(stderr,"Error: cannot open input file\n");
						break;
					}
					dup2(fd,STDIN_FILENO);
					close(fd);
					waitpid(-1, &status, 0);
					fprintf(stderr, "+ completed '%s' [%d]\n", userInputCopy, WEXITSTATUS(status));
					//if array[i] exist in current directory

				}
			}
			continue;
		}
		*/

		//printf("%s\n",string);
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
		//print cd
		else if ((strcmp(*target, "cd") == 0)) {
			getcwd(buf,sizeof(buf));
			strcpy(pre_cd,buf);
			//cd .. or cd .
			if ((userInputCopy[strlen(userInputCopy)-1]) == '.') {
				if ((userInputCopy[strlen(userInputCopy)-2]) == '.') {
					chdir("..");
				}
				continue;
			}
			//cd filename
			for (i = 0;i<count;i++) {
				if (i==(count-1)) {
					if (chdir( array[i] ) == 0){
						break;
					}
					else {
						fprintf(stderr,"Error: no such directory\n");
					}
				}
			}
			waitpid(-1, &status, 0);
			fprintf(stderr, "+ completed '%s' [%d]\n", userInputCopy, WEXITSTATUS(status));
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
char *tokenize(int *count, char userInputCopy[]) {
	int i = 0;
	count= 0;
	char *p = strtok (userInputCopy, " ");

	while (p != NULL) {
		array[i++] = p;
		p = strtok (NULL, " ");
		count++;
	}
	return
}
*/
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
