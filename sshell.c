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
    bool lessExist;
    bool largerExist;
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
    cmd->lessExist = false;
    cmd->largerExist = false;
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
            cmd->lessExist = true;
            i += 1;
            if (i != length) {
                cmd->fileIn = cmdArray[i];
            }
        }
        else if (strcmp(cmdArray[i],">") == 0) {
            cmd->largerExist = true;
            i += 1;
            if (i != length) {
                cmd->fileOut = cmdArray[i];
            }
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
}



int main(int argc, char *argv[])
{
	while (1) {
		// https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
		char userInput[MAX_INPUT];
		char userInputCopy[MAX_INPUT];
		pid_t pid;
		int status;
		// int count;
		char buf[256];
		char pre_cd[256];
        int saveStdout = dup(STDOUT_FILENO);
        int saveStdin = dup(STDIN_FILENO);
		printf("sshell$ ");

		// Read user input, and append \0 at the end.
		fgets(userInput, MAX_INPUT, stdin);

        /* Print command line if we're not getting stdin from the
		 * terminal */
		if (!isatty(STDIN_FILENO)) {
			printf("%s", userInput);
			fflush(stdout);
		}

		char *lastCharPos;
        int userInputLength;
		if ((lastCharPos = strchr(userInput, '\n')) != NULL) {
			*lastCharPos = '\0';
            userInputLength = lastCharPos - userInput;
		}
		else {
			perror("something goes wrong\n");
		}

        // Check if userInput is only spaces and tabs.
         int spaceTabCount = 0;
         for (int i = 0; i < userInputLength; i++) {
             if (userInput[i] == ' ' || userInput[i] == '\t') {
                 spaceTabCount++;
             }
         }
         if (spaceTabCount == userInputLength) {
             continue;
         }

		strcpy(userInputCopy, userInput);

        Jobs *myjobPtr, myjob;
        myjobPtr = &myjob;
        // Jobs *myjobPtr = malloc(sizeof(Jobs));


        // userInput is modified in this function.
        jobsConstructor(userInput, userInputLength, myjobPtr);
        // printf("In main function, background: %d\n", myjobPtr->background); //0->false; 1->true
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
            printf("%s\n",buf);
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

        //input redirection
        if (myjobPtr->cmdCount == 1) {
            if (myjobPtr->cmds[0].lessExist && myjobPtr->cmds[0].fileIn == NULL) {
                fprintf(stderr,"Error: no input file\n");
                continue;
            }
            else if (myjobPtr->cmds[0].lessExist && myjobPtr->cmds[0].fileIn) {
                printf("Goes to fileIn \n");
                int fileInDes = open(myjobPtr->cmds[0].fileIn, O_RDWR);
                if (fileInDes < 0) {
                    fprintf(stderr, "Error: cannot open input file\n");
                    continue;
                }
                dup2(fileInDes, STDIN_FILENO);
                close(fileInDes);
            }
        }

        //output redirection
        //if file redirection exists
        if ((myjobPtr->cmds[0].largerExist == 1) && (myjobPtr->cmdCount == 1)) {
            // printf("fileout %s\n",myjobPtr->cmds[0].fileOut );
            if ((myjobPtr->cmds[0].largerExist == 1) && (myjobPtr->cmds[0].fileOut == NULL)) {
                fprintf(stderr,"Error: no output file\n");
                continue;
            }
            int fd = open(myjobPtr->cmds[0].fileOut, O_WRONLY | O_CREAT, 0777);
            if (fd < 0) {
                fprintf(stderr,"Error: cannot open output file\n");
                continue;
            }
            if (fd) {
                dup2(fd,STDOUT_FILENO);
                close(fd);
            }

        }
        //pipeline
        int i;
        int fd[2];
        if (myjobPtr->cmdCount > 1) {
            for (i = 0; i < (myjobPtr->cmdCount - 1); i++) {

                pipe(fd);
                if (fork() != 0) {
                    //parent
                    // close(fd[0]);
                    dup2(fd[1],STDOUT_FILENO);
                    close(fd[1]);
                    //exec process1
                    execvp(myjobPtr->cmds[i].exec, myjobPtr->cmds[i].args);
                }
                else {
                    //child
                    // close(fd[1]);
                    dup2(fd[0],STDIN_FILENO);
                    close(fd[0]);
                    //exec process2
                    // execvp(myjobPtr->cmds[i+1].exec, myjobPtr->cmds[i].args);

                }
                // execvp(myjobPtr->cmds[i].exec, myjobPtr->cmds[i].args);


            }
            if (i == (myjobPtr->cmdCount - 1)) {
                execvp(myjobPtr->cmds[i].exec, myjobPtr->cmds[i].args);
            }
            printf("finished tube\n");
            continue;
        }

        pid = fork();
        if (pid == 0) {
        	// child
            // printf("exec are: %s\n",myjobPtr->cmds[0].exec);
            execvp(myjobPtr->cmds[0].exec, myjobPtr->cmds[0].args);

            fprintf(stderr,"Error: command not found\n");
            exit(1);

        } else if (pid > 0) {
        	// parent
        	waitpid(-1, &status, 0);
        	fprintf(stderr, "+ completed '%s' [%d]\n", userInputCopy, WEXITSTATUS(status));
            dup2(saveStdout,STDOUT_FILENO);
            dup2(saveStdin,STDIN_FILENO);
            close(saveStdin);
            close(saveStdout);
        } else {
        	perror("fork");
        	exit(1);

        }

        for (int i = 0; i < ptrToFreeCount; i++) {
            free(ptrToFree[i]);
            ptrToFree[i] = NULL;
            ptrToFreeCount = 0;
        }

	}
}
