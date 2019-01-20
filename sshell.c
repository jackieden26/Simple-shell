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

// struct Input {
// 	struct Cmd command;
// 	bool background;
// };
//
// struct Cmd {
// 	char *exec;
// 	char *args[16];
// 	char *inputFile;
// 	char *outputFile;
// };

// https://stackoverflow.com/questions/13084236/function-to-remove-spaces-from-string-char-array-in-c

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
    for (i = 0, j = 0; i < strlen(input); i++, j++)
    {
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
    Command cmds[MAX_INPUT]; //*cmds
} Jobs;


// cmdStr is p in jobsConstructor.
// This function modifies cmdStr, by replacing whitespace and tab with '\0'
void commandConstructor(char *cmdStr, int count, Command* cmd) {
    char cmdStrC[MAX_INPUT];
    //printf("cmdStr are: %s\n", cmdStr);

    // Initialize every property to be NULL.
    cmd->exec = NULL;
    cmd->fileIn = NULL;
    cmd->fileOut = NULL;
    for (int i = 0; i < MAX_ARG + 1; i++) {
        cmd->args[i] = NULL;
    }

    // Replace every whitespace and tab to NULL character '\0'
    //formatting strings add one whitespace if there is none
    int j = 0;
    int strLength = strlen(cmdStr);
    for (int i = 0; i < strLength; i++) {
    	if (cmdStr[i] == ' ' || cmdStr[i] == '\t') {
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

    //stroke input by whitespace
    int i = 0;
    int length = 0;
    char *split = strtok (cmdStrC, " ");
    char *cmdArray[MAX_INPUT];

    while (split != NULL)
    {
        cmdArray[i++] = split;
        split = strtok (NULL, " ");
        length ++;
    }

    //store all userinput to its specified position
    j = 0;
    for (int i = 0; i < length; i++) {
        if (i == 0) {
            cmd->exec = cmdArray[i];
            cmd->args[j] = cmdArray[i];
            j++;
        }
        else if (strcmp(cmdArray[i],"<") == 0) {
            // printf("this line %s\n",&cmdStrC[i]);
            i += 1;
            cmd->fileIn = cmdArray[i];
        }
        else if (strcmp(cmdArray[i],">") == 0) {
            i += 1;
            cmd->fileOut = cmdArray[i];
        }
        else {
            cmd->args[j] = cmdArray[i];
            j++;
        }
    }
    printf("prints in commandConstructor function\n");
    printf("first exec are: %s\n", cmd->exec);
    printf("filein are: %s\n", cmd->fileIn);
    printf("fileout are: %s\n", cmd->fileOut);
    for (int i = 0; i < 10;i++) {
        printf("args: %s\n", cmd->args[i]);
    }


    // exit(0);

    // Create copy of cmdStr with null character for less, large, argument pointer modification.
    // char cmdStrLe[MAX_INPUT];
    // char cmdStrLa[MAX_INPUT];
    // char cmdStrAr[MAX_INPUT];
    //strcpy(cmdStrLe, cmdStr);
    // strcpy(cmdStrLa, cmdStr);
    // strcpy(cmdStrAr, cmdStr);


    // int lessStart;
    // int lessEnd;
    // // int largerStart;
    // // int largerEnd;
    //
    // char* less = strchr(cmdStrC, '<');
    // int lessIndex = strLength;
    // if (less != NULL) {
    //     lessIndex = (int)(less - cmdStrC);
    // }
    // char* larger = strchr(cmdStrC, '>');
    // int largerIndex = strLength;
    // if (larger != NULL) {
    //     largerIndex = (int)(larger - cmdStrC);
    // }
    //
    // // // argStop is either the index of <, > or strLength.
    // // int argStop = findMin(lessIndex, largerIndex);
    // //
    // //
    // // int wordIndex = 0;
    // // int index = 0;
    // // while(index < argStop) {
    // // 	while (cmdStr[index] == '\0' && index < argStop) {
    // // 		index++;
    // // 	}
    // // 	if (index >= argStop) {
    // // 		break;
    // // 	}
    // // 	cmd->args[wordIndex] = cmdStr + index;
    // // 	wordIndex++;
    // // 	while (cmdStr[index] != '\0' && index < argStop) {
    // // 		index++;
    // // 	}
    // // }
    // //
    // // cmd->exec = cmd->args[0];
    //
    // // Find input file redirection.
    // if (lessIndex != strLength) {
    //     for (int i = lessIndex + 1; i < strLength; i++) {
    //         // Skip \0 right after <.
    //         if (cmdStrLe[i] == '\0') {
    //             continue;
    //         }
    //         // Set input file redirection pointer.
    //         else {
    //             // lessStart to lessEnd is the redirected file.
    //             lessStart = i;
    //             for (int j = i; j < strLength; j++) {
    //                 if (j != '\0' && j != '>') {
    //                     continue;
    //                 }
    //                 else {
    //                     cmdStrLe[j] = '\0';
    //                     lessEnd = j-1;
    //                     break;
    //                 }
    //             }
    //             cmd->fileIn = cmdStrLe + i;
    //             printf("lessStart and lessEnd is: %d %d\n", lessStart, lessEnd);
    //             break;
    //         }
    //     }
    // }
    // cmdStr[lessIndex] = '\0';
    //
    // // Find output file redirection.
    // if (largerIndex != strLength && lessIndex == strLength) {
    //     for (int i = largerIndex + 1; i < strLength; i++){
    //         // Skip \0 right after <.
    //         if (cmdStr[i] == '\0') {
    //             continue;
    //         }
    //         // Set input file redirection pointer.
    //         else {
    //             cmd->fileOut = cmdStr + i;
    //             break;
    //         }
    //     }
    // }
    //
    // // If both < and > is in the command string:
    // if (largerIndex != strLength && lessIndex != strLength) {
    //     // < is before >.
    //     if (lessIndex < largerIndex) {
    //
    //     }
    // }
    //
    //
    //
    //
    //
    // printf("fileIn is: %s\n", cmd->fileIn);
    // printf("exec is: %s\n", cmd->exec);
    // for (int i = 0; i < 13 ;i++) {
    //     printf("args[i] is: %s\n", cmd->args[i]);
    // }
}

void jobsConstructor(char* userInput, Jobs* job) {

    job->background = false;

    int count= 0;

    // P points to the first character of each command string.
    char *p = strtok(userInput, "|");

    while (p != NULL) {
        Command *cmdPtr, cmd;
        cmdPtr = &cmd;
        commandConstructor(p, count, cmdPtr);
        job->cmds[count] = *cmdPtr;
    	p = strtok(NULL, "|");
    	count++;
        //printf("exec are: %s\n", job->cmds[0].exec);
        //printf("exec are: %s\n", cmdPtr->exec);
    }
    printf("no results here\n");
    printf("number of pipes in jobs is: %d\n", count);
    printf("exec are: %s\n", job->cmds[0].exec);
    // printf("exec are: %s\n", cmd.exec);

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
		// pid_t pid;
		// int status;
		// int i,count;
		// char buf[256];
		// char pre_cd[256];
		// char *array[100];

		// struct Input user_input;
		// struct Cmd command;


		//char *path;
		//int fd;

		printf("sshell$ ");

		// Read user input, and append \0 at the end.
		fgets(userInput, MAX_INPUT, stdin);
		char *lastCharPos;
		if ((lastCharPos = strchr(userInput, '\n')) != NULL) {
			*lastCharPos = '\0';
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

        Jobs *myjobPtr, myjob;
        myjobPtr = &myjob;



        // userInput is modified in this function.
        jobsConstructor(userInput, myjobPtr);
        printf("background: %d\n", myjobPtr->background); //1->false; 0->true
        printf("exec are: %s\n", myjobPtr->cmds[0].exec);
        // printf("filein are: %s\n", myjobPtr->cmds[0]->fileIn);
        // printf("fileout are: %s\n", myjobPtr->cmds[0]->fileOut);
        // for (int i = 0; i < 10;i++) {
        //     printf("args: %s\n", myjobPtr->cmds[0]->args[i]);
        // }























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
