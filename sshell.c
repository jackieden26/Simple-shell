#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_ARG 16
#define MAX_INPUT 512

int findMax(int x, int y) {
    if (x > y) {
        return x;
    }
    else {
        return y;
    }
}

int findMin(int x, int y) {
    if (x > y) {
        return y;
    }
    else {
        return x;
    }
}

char* deblank(char* input) {
    int i,j;
    char *output = input;
    for (i = 0, j = 0; i < strlen(input); i++, j++) {
        if (input[i]!=' ')
            output[j]=input[i];
        else
            j--;
    }
    output[j]=0;
    return output;
}

typedef struct {
    char *exec;
    char *args[MAX_ARG + 1];
    char *fileIn;
    char *fileOut;
} Command;

typedef struct {
    bool background;
    int cmdCount;
    Command cmds[MAX_INPUT];
} Jobs;

char *ptrToFree[MAX_ARG];
int ptrToFreeCount = 0;

// This function modifies cmdStr, by replacing whitespace and tab with '\0'
void commandConstructor(char *cmdStr, int count, Command* cmd) {
    // printf("in commandConstructor, comming cmdStr is: %s\n", cmdStr);
    // printf("coming cmd address is: %p\n", cmd);
    int strLength = strlen(cmdStr);

    char *cmdStrC = malloc((strLength+1) * sizeof(char));
    ptrToFree[ptrToFreeCount++] = cmdStrC;

    // Initialize every property to be NULL.
    cmd->exec = NULL;
    cmd->fileIn = NULL;
    cmd->fileOut = NULL;
    for (int i = 0; i < MAX_ARG + 1; i++) {
        cmd->args[i] = NULL;
    }

    // Formatting strings add one whitespace if there is none.
    // printf("before copy userinput is %s\n",cmdStrC);
    int j = 0;
    for (int i = 0; i < strLength; i++) {
        if ((i == 0) && (cmdStr[i] == ' ' || cmdStr[i] == '\t')) {
            while (cmdStr[i+1] == ' ' || cmdStr[i+1] == '\t') {
                i++;
            }
        }
    	else if (cmdStr[i] == ' ' || cmdStr[i] == '\t') {
    		cmdStrC[j] = ' ';
            j++;
            while (cmdStr[i+1] == ' ' || cmdStr[i+1] == '\t') {
                i++;
            }
    	}

        else if (cmdStr[i] == '<' || cmdStr[i] == '>' || cmdStr[i] == '&') {
            if (cmdStr[i-1] != ' ') {
                cmdStrC[j] = ' ';
                j++;
            }
            cmdStrC[j] = cmdStr[i];
            j++;
            if (cmdStr[i+1] != ' ') {
                cmdStrC[j] = ' ';
                j++;
            }
        }
        else {
            cmdStrC[j] = cmdStr[i];
            j++;
        }
    }

    // printf("after copy userinput is %s\n",cmdStrC);
    //if only one element in userInput = if no whitespace in strings

    // if (strstr(cmdStrC," ") == NULL) {
    //     cmd->exec = cmdStrC;
    //     cmd->args[0] = cmdStrC;
    //     return;
    // }
    cmdStrC[j] = '\0';

    // Stroke input by whitespace.
    int i = 0;
    int length = 0;
    // printf("in line 123 cmdStrC is: %s\n", cmdStrC);
    char *split = strtok(cmdStrC, " ");
    char *cmdArray[MAX_INPUT];

    while (split != NULL)
    {
        cmdArray[i++] = split;
        split = strtok(NULL, " ");
        length++;
        // printf("split is: %s\n", split);
    }

    // Store all userinput to its specified position.
    j = 0;
    for (int i = 0; i < length; i++) {
        if (i == 0) {
            cmd->exec = cmdArray[i];
            cmd->args[j] = cmdArray[i];
            j++;
        }
        else if (strcmp(cmdArray[i],"<") == 0) {
            i += 1;
            cmd->fileIn = cmdArray[i];
        }
        else if (strcmp(cmdArray[i],">") == 0) {
            i += 1;
            cmd->fileOut = cmdArray[i];
        }
        else {
            cmd->args[j] = cmdArray[i];
            // printf("cmdArray[i] is: %s\n", cmdArray[i]);
            j++;
        }
    }
    // printf("prints in commandConstructor function\n");
    // printf("first exec are: %s\n", cmd->exec);
    // printf("filein are: %s\n", cmd->fileIn);
    // printf("fileout are: %s\n", cmd->fileOut);
    // for (int i = 0; i < 5;i++) {
    //     printf("args: %s\n", cmd->args[i]);
    // }
    // free(cmdStrC);
}

void jobsConstructor(char* userInput, int userInputLength, Jobs* job) {

    job->background = false;

    // P points to the first character of each command string.
    // printf("userInput is: %s\n", userInput);
    // printf("userInputLength is: %d\n", userInputLength);
    // printf("userInput[0]is: %s\n", userInput[0]);

    char *startPtr[MAX_ARG];
    startPtr[0] = userInput;
    int ptrCount = 1;

    for (int i = 0; i < userInputLength; i++) {
        if (userInput[i] == '|') {
            userInput[i] = '\0';
            startPtr[ptrCount] = userInput + i + 1;
            ptrCount++;
        }
        if (userInput[i] == '&') {
            job->background = true;
        }
    }


    Command myCommandArray[ptrCount];


    for (int i = 0; i < ptrCount; i++) {
        // printf("startPtr storing value is: %s\n", startPtr[i]);
        commandConstructor(startPtr[i], ptrCount, &myCommandArray[i]);
        job->cmds[i] = myCommandArray[i];
    }

    job->cmdCount = ptrCount;



    // printf("In jobsConstructor count is: %d\n", ptrCount);

}



int main(int argc, char *argv[])
{
	while (1) {
		//struct Command cmd[MAX_INPUT];
		// https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
		char userInput[MAX_INPUT];
		char userInputCopy[MAX_INPUT];
		// int index = 0;
		// int wordIndex = 0;
		// char *target[16];
		pid_t pid;
		int status;
		// int count;
		char buf[256];
		char pre_cd[256];
		// char *array[100];

		// struct Input user_input;
		// struct Cmd command;


		//char *path;
		//int fd;

		printf("sshell$ ");

		// Read user input, and append \0 at the end.
		fgets(userInput, MAX_INPUT, stdin);
		char *lastCharPos;
        int userInputLength;
		if ((lastCharPos = strchr(userInput, '\n')) != NULL) {
			*lastCharPos = '\0';
            userInputLength = lastCharPos - userInput;
		}
		else {
			perror("something goes wrong");
		}
		strcpy(userInputCopy, userInput);
		// int strLength = strlen(userInput);

		// char *includeFileIn = strstr(userInput, "<");
		// char *includeFileOut = strstr(userInput, ">");
		// char *includePipe = strstr(userInput, "|");

		// // Simple case: no file redirection or pipe.
		// if (includeFileIn == NULL || includeFileOut == NULL || includePipe == NULL) {
		// 	//print exit
		// 	if (strcmp(userInput, "exit") == 0) {
		// 		fprintf(stderr, "Bye...\n");
		// 		exit(0);
		// 	}
		//
		// 	// Print pwd.
		// 	// Todo: WEXITSTATUS[status] is always 127
		// 	else if ((strcmp(userInput, "pwd") == 0)) {
		//
		// 		getcwd(buf,sizeof(buf));
		// 		fprintf(stderr,"%s\n",buf);
		// 		fprintf(stderr, "+ completed '%s' [0]\n", userInputCopy);
		// 		continue;
		// 	}
		// }

        // Jobs *myjobPtr, myjob;
        // myjobPtr = &myjob;
        Jobs *myjobPtr = malloc(sizeof(Jobs));


        // userInput is modified in this function.
        jobsConstructor(userInput, userInputLength, myjobPtr);
        // printf("In main function, background: %d\n", myjobPtr->background); //1->false; 0->true
        // printf("In main first exec are: %s\n", myjobPtr->cmds[0].exec);
        // printf("In main frist filein are: %s\n", myjobPtr->cmds[0].fileIn);
        // printf("In main first fileout are: %s\n", myjobPtr->cmds[0].fileOut);
        // for (int i = 0; i < 5;i++) {
        //     printf("In main first args: %s\n", myjobPtr->cmds[0].args[i]);
        // }
        //
        // printf("In main second exec are: %s\n", myjobPtr->cmds[1].exec);
        // printf("In main second filein are: %s\n", myjobPtr->cmds[1].fileIn);
        // printf("In main second fileout are: %s\n", myjobPtr->cmds[1].fileOut);
        // for (int i = 0; i < 5;i++) {
        //     printf("In main second args: %s\n", myjobPtr->cmds[1].args[i]);
        // }

        //error management in phase 5.3.1
        // sshell$ &
        // Error: invalid command line

        int args_size = 0;
        for (int i = 0; i < MAX_ARG;i++) {
            if (myjobPtr->cmds[0].args[i] == NULL) {
                break;
            }
            else {
                args_size ++;
            }
        }

        if ((myjobPtr->background == 1) && (args_size < 2)) {
            fprintf(stderr,"Error: invalid command line\n");
            continue;
        }
        //print exit
        if (strcmp(myjobPtr->cmds[0].exec, "exit") == 0) {
            fprintf(stderr, "Bye...\n");
            exit(0);
        }

        // Print pwd.
        else if ((strcmp(myjobPtr->cmds[0].exec, "pwd") == 0)) {
            getcwd(buf,sizeof(buf));
            fprintf(stderr,"%s\n",buf);
            fprintf(stderr, "+ completed '%s' [0]\n", userInputCopy);
            continue;
        }

        //print cd
		else if ((strcmp(myjobPtr->cmds[0].exec, "cd") == 0)) {
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

			if (chdir(myjobPtr->cmds[0].args[1]) != 0){
				fprintf(stderr,"Error: no such directory\n");
			}

			//waitpid(-1, &status, 0);
			fprintf(stderr, "+ completed '%s' [%d]\n", userInputCopy, WEXITSTATUS(status));
			continue;
		}

        pid = fork();
        if (pid == 0) {
        	// child
            printf("exec are: %s\n",myjobPtr->cmds[0].exec);
            execvp(myjobPtr->cmds[0].exec, myjobPtr->cmds[0].args);

            fprintf(stderr,"Error: command not found\n");
            exit(1);

        } else if (pid > 0) {
        	// parent
        	waitpid(-1, &status, 0);
        	fprintf(stderr, "+ completed '%s' [%d]\n", userInputCopy, WEXITSTATUS(status));
        } else {
        	perror("fork");
        	exit(1);

        }

        for (int i = 0; i < ptrToFreeCount; i++) {
            free(ptrToFree[i]);
            ptrToFree[i] = NULL;
        }
        free(myjobPtr);



















		//
		// while(index < strLength) {
		// 	if (userInput[index] == ' ' || userInput[index] == '\t') {
		// 		userInput[index] = '\0';
		// 	}
		// 	index++;
		// }
		//
		// index = 0;
		// while(index < strLength) {
		// 	while (userInput[index] == '\0' && index < strLength) {
		// 		index++;
		// 	}
		// 	if (index >= strLength) {
		// 		break;
		// 	}
		// 	target[wordIndex] = userInput + index;
		// 	wordIndex++;
		// 	while (userInput[index] != '\0' && index < strLength) {
		// 		index++;
		// 	}
		// }
		// target[wordIndex] = NULL;
		//
		// //put userinput into struct
		// for (i = 0; i < strlen(userInputCopy); i++) {
		// 	if (strcmp(userInputCopy[i],"|")==0) {
		//
		// 	}
		// }
		//
		// //fprintf(stderr,"target %c\n",userInput[strlen(userInput)-1]);
		//
		// //tokenize userinput and store in array (an array of strings)
		//
		// if (strstr(userInputCopy,"<") != NULL) {
		// 	string = "<";
		// }
		// else if (strstr(userInputCopy,">") != NULL) {
		// 	string = ">";
		// }
		//
		// i = 0;
		// count= 0;
		// char *p = strtok (userInputCopy, string);
		//
		// while (p != NULL) {
		// 	array[i++] = p;
		// 	p = strtok (NULL, string);
		// 	count++;
		// }
		//
		//
		// //printf("count %d\n",count);
		// /*
		// //input redirection
		// if ((count == 1) && (strcmp("<",string) == 0)) {
		// 	fprintf(stderr,"Error: no input file\n");
		// 	continue;
		// }
		//
		//
		// else if (strcmp("<",string) == 0) {
		// 	for (i = 0;i<count;i++) {
		// 		if (i==(count-1)) {
		// 			getcwd(buf,sizeof(buf));
		// 			path = malloc(strlen(buf)+strlen(array[i])+2);
		// 			path[0] = '\0';
		// 			strcat(path,buf);
		// 			strcat(path,"/");
		// 			strcat(path,array[i]);
		// 			//printf("%s \n",path);
		// 			fd = open(path,O_RDWR);
		// 			if (fd<0) {
		// 				fprintf(stderr,"Error: cannot open input file\n");
		// 				break;
		// 			}
		// 			dup2(fd,STDIN_FILENO);
		// 			close(fd);
		// 			waitpid(-1, &status, 0);
		// 			fprintf(stderr, "+ completed '%s' [%d]\n", userInputCopy, WEXITSTATUS(status));
		// 			//if array[i] exist in current directory
		//
		// 		}
		// 	}
		// 	continue;
		// }
		// */
		//
		// //printf("%s\n",string);
		//
		// //print cd
		// else if ((strcmp(*target, "cd") == 0)) {
		// 	getcwd(buf,sizeof(buf));
		// 	strcpy(pre_cd,buf);
		// 	//cd .. or cd .
		// 	if ((userInputCopy[strlen(userInputCopy)-1]) == '.') {
		// 		if ((userInputCopy[strlen(userInputCopy)-2]) == '.') {
		// 			chdir("..");
		// 		}
		// 		continue;
		// 	}
		// 	//cd filename
		// 	for (i = 0;i<count;i++) {
		// 		if (i==(count-1)) {
		// 			if (chdir( array[i] ) == 0){
		// 				break;
		// 			}
		// 			else {
		// 				fprintf(stderr,"Error: no such directory\n");
		// 			}
		// 		}
		// 	}
		// 	waitpid(-1, &status, 0);
		// 	fprintf(stderr, "+ completed '%s' [%d]\n", userInputCopy, WEXITSTATUS(status));
		// 	continue;
		// }
		//
		//
		// pid = fork();
		// if (pid == 0) {
		// 	// child
		// 	execvp(target[0], target);
		// 	perror("execvp");
		//
		// } else if (pid > 0) {
		// 	// parent
		// 	waitpid(-1, &status, 0);
		// 	fprintf(stderr, "+ completed '%s' [%d]\n", userInputCopy, WEXITSTATUS(status));
		// } else {
		// 	perror("fork");
		// 	exit(1);
		//
		// }
	}
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
Set flag in jobsConstructor

sshell$ toto
Error: command not found
+ completed 'toto' [1]
After execvp

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
