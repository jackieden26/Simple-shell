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
#define MAX_BACKJOB 100

typedef struct {
    char *exec;
    char *args[MAX_ARG + 1];
    char *fileIn;
    char *fileOut;
    bool lessExist;
    bool largerExist;
    // andValid is true iff there is only one '&' at the end of command.
    bool andValid;
    int argCount;
    int andCount;
} Command;

typedef struct {
    // background is true as long as there exists '&' in userInput.
    bool background;
    int cmdCount;
    Command cmds[MAX_INPUT];
} Jobs;

/*
 * This array record dynamically allocated strings for each commandConstructor.
 * It is free and count is reset at the end of while loop.
 */
char *ptrToFree[MAX_ARG];
int ptrToFreeCount = 0;

void commandConstructor(char *cmdStr, int count, Command* cmd) {
    int strLength = strlen(cmdStr);

    // Reserve one more byte at the end for '\0'.
    char *cmdStrC = malloc((strLength+1) * sizeof(char));
    ptrToFree[ptrToFreeCount++] = cmdStrC;

    // Initialize every Command property.
    cmd->exec = NULL;
    cmd->fileIn = NULL;
    cmd->fileOut = NULL;
    cmd->lessExist = false;
    cmd->largerExist = false;
    cmd->andValid = false;
    cmd->argCount = 0;
    cmd->andCount = 0;
    for (int i = 0; i < MAX_ARG + 1; i++) {
        cmd->args[i] = NULL;
    }

    /*
     * Format strings by seperate every token with exactly one space.
     * j is the index for cmdStrC, which would be the formatted result.
     */
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

    // Set the one last char to '\0' to indicate the end of string.
    cmdStrC[j] = '\0';

    // Strtok input by whitespace.
    int i = 0;
    int length = 0;
    char *split = strtok(cmdStrC, " ");
    char *cmdArray[MAX_INPUT];

    while (split != NULL) {
        cmdArray[i++] = split;
        split = strtok(NULL, " ");
        length++;
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
        else if (strcmp(cmdArray[i],"&") == 0) {
            cmd->andCount++;
            if (i == length -1 && cmd->andCount == 1) {
                cmd->andValid = true;
            }
        }
        else {
            cmd->args[j] = cmdArray[i];
            cmd->argCount++;
            j++;
        }
    }
}

void jobsConstructor(char* userInput, int userInputLength, Jobs* job) {
    // Initialize jobs property.
    job->background = false;
    // Count how many commands in userInput with multiple pipeline.
    int ptrCount = 1;

    char *startPtr[MAX_ARG];
    startPtr[0] = userInput;
    // Replace pipeline char with '\0' to seperate commands as strings.
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
        commandConstructor(startPtr[i], ptrCount, &myCommandArray[i]);
        job->cmds[i] = myCommandArray[i];
    }

    job->cmdCount = ptrCount;
}

void checkBgComplete(int *jobCount, int *jobPid, int *jobStatus, char *jobInput[MAX_BACKJOB]) {
    //check if any background job complete
    for (int i = 0; i < *jobCount; i++) {
        if (waitpid(jobPid[i], &jobStatus[i], WNOHANG) > 0) {
            fprintf(stderr, "+ completed '%s' [%d]\n", jobInput[i], WEXITSTATUS(jobStatus[i]));
            // Take the completed job out of the array and shift the array accordingly.
            free(jobInput[i]);
            for (int j = i; j < *jobCount; j++) {
                jobInput[j] = jobInput[j + 1];
                jobPid[j] = jobPid[j + 1];
                jobStatus[j] = jobStatus[j + 1];
            }
            i--;
            (*jobCount)--;
        }
    }
}

int main(int argc, char *argv[])
{
    int jobCount = 0;
    int jobPid[MAX_BACKJOB];
    int jobStatus[MAX_BACKJOB];
    char *jobInput[MAX_BACKJOB];
	while (1) {
		char userInput[MAX_INPUT];
		char userInputCopy[MAX_INPUT];
		pid_t pid;
		int status = 0;
		char buf[256];
		char pre_cd[256];
        int saveStdout = dup(STDOUT_FILENO);
        int saveStdin = dup(STDIN_FILENO);

        printf("sshell$ ");
		fgets(userInput, MAX_INPUT, stdin);

		if (!isatty(STDIN_FILENO)) {
			printf("%s", userInput);
			fflush(stdout);
		}

		char *lastCharPos;
        int userInputLength = 0;
        // Replace trailing new line char with '\0'.
		if ((lastCharPos = strchr(userInput, '\n')) != NULL) {
			*lastCharPos = '\0';
            userInputLength = lastCharPos - userInput;
		}
        // This shoud not happen if stdin/stdout set correctly.
		else {
			perror("possible ioCtrl issue\n");
		}

        // Check if userInput is only spaces and tabs.
        int spaceTabCount = 0;
        for (int i = 0; i < userInputLength; i++) {
            if (userInput[i] == ' ' || userInput[i] == '\t') {
                spaceTabCount++;
            }
        }
        if (spaceTabCount == userInputLength && jobCount == 0) {
            continue;
        }
        if (spaceTabCount == userInputLength && jobCount != 0) {
            checkBgComplete(&jobCount, jobPid, jobStatus, jobInput);
            continue;
        }

		strcpy(userInputCopy, userInput);
        Jobs *myjobPtr, myjob;
        myjobPtr = &myjob;
        // userInput is modified in this function.
        jobsConstructor(userInput, userInputLength, myjobPtr);

        // Check if '&' is the only argument and there is only one command in job.
        if (myjobPtr->background == 1 && myjobPtr->cmds[0].argCount < 1 && myjobPtr->cmdCount == 1) {
            fprintf(stderr,"Error: invalid command line\n");
            continue;
        }

        // Print exit.
        if (strcmp(myjobPtr->cmds[0].exec, "exit") == 0 && jobCount == 0) {
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

        // Print cd.
		else if ((strcmp(myjobPtr->cmds[0].exec, "cd") == 0)) {
			getcwd(buf,sizeof(buf));
			strcpy(pre_cd,buf);
			// cd .. or cd .
			if ((userInputCopy[strlen(userInputCopy)-1]) == '.') {
				if ((userInputCopy[strlen(userInputCopy)-2]) == '.') {
					chdir("..");
                    fprintf(stderr, "+ completed '%s' [0]\n", userInputCopy);
				}
				continue;
			}
			// cd filename.
			if (chdir(myjobPtr->cmds[0].args[1]) != 0){
				fprintf(stderr,"Error: no such directory\n");
                fprintf(stderr, "+ completed '%s' [1]\n", userInputCopy);
                continue;
			}
            else {
                fprintf(stderr, "+ completed '%s' [0]\n", userInputCopy);
                continue;
            }
		}

        // Handle background.
        bool mislocatedSign = false;
        if (myjobPtr->background == 1) {
        // Handle mislocated background sign.
            for (int i = 0; i < myjobPtr->cmdCount; i++) {
                // If '&' exists in command which is not the last command in job.
                if (i < myjobPtr->cmdCount -1) {
                    if (myjobPtr->cmds[i].andCount > 0) {
                        fprintf(stderr,"Error: mislocated background sign\n");
                        mislocatedSign = true;
                        break;
                    }
                }
                else {
                    if (myjobPtr->cmds[i].andValid == false) {
                        fprintf(stderr,"Error: mislocated background sign\n");
                        mislocatedSign = true;
                        break;
                    }
                }
            }
            if (mislocatedSign) {
                continue;
            }
        }

        // Input redirection when no pipe.
        if (myjobPtr->cmdCount == 1) {
            if (myjobPtr->cmds[0].lessExist && myjobPtr->cmds[0].fileIn == NULL) {
                fprintf(stderr,"Error: no input file\n");
                continue;
            }
            else if (myjobPtr->cmds[0].lessExist && myjobPtr->cmds[0].fileIn) {
                int fileInDes = open(myjobPtr->cmds[0].fileIn, O_RDWR);
                if (fileInDes < 0) {
                    fprintf(stderr, "Error: cannot open input file\n");
                    continue;
                }
                else {
                    dup2(fileInDes, STDIN_FILENO);
                    close(fileInDes);
                }
            }
        }

        // Output redirection when no pipe.
        if (myjobPtr->cmdCount == 1) {
            if (myjobPtr->cmds[0].largerExist == 1 && myjobPtr->cmds[0].fileOut == NULL) {
                fprintf(stderr,"Error: no output file\n");
                continue;
            }
            else if (myjobPtr->cmds[0].largerExist == 1 && myjobPtr->cmds[0].fileOut){
                int fileOutDes = open(myjobPtr->cmds[0].fileOut, O_WRONLY | O_CREAT, 0777);
                if (fileOutDes < 0) {
                    fprintf(stderr,"Error: cannot open output file\n");
                    continue;
                }
                else {
                    dup2(fileOutDes,STDOUT_FILENO);
                    close(fileOutDes);
                }
            }
        }

        // Pipeline job.
        // https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell
        int in, fd[2];
        in = 0;
        // Set error flag to break the commands loop.
        bool error = false;
        int exit_status[MAX_ARG];
        if (myjobPtr->cmdCount > 1) {
            for (int i = 0; i < (myjobPtr->cmdCount -1); i++) {
                // Check if output redirect only exists in last command.
                // Check if input redirect wrongly exists in the last command.
                if (i == 0) {
                    if (myjobPtr->cmds[myjobPtr->cmdCount-1].lessExist == 1) {
                        fprintf(stderr,"Error: mislocated input redirection\n");
                        error = true;
                        break;
                    }
                }
                if (myjobPtr->cmds[i].largerExist == 1) {
                    fprintf(stderr,"Error: mislocated output redirection\n");
                    error = true;
                    break;
                }

                // Check if input redirect wrongly exists in middle commands.
                if (myjobPtr->cmds[i].lessExist == 1 && i != 0) {
                    fprintf(stderr,"Error: mislocated input redirection\n");
                    error = true;
                    break;
                }

                // Connect pipe.
                pipe(fd);
                if (fork() == 0) {
                    // Child.
                    // Open first command's input redirect file.
                    if (i == 0 && myjobPtr->cmds[i].lessExist) {
                        if (myjobPtr->cmds[0].fileIn == NULL) {
                            fprintf(stderr,"Error: no input file\n");
                            error = true;
                            break;
                        }
                        else {
                            int fileInDes = open(myjobPtr->cmds[0].fileIn, O_RDWR);
                            if (fileInDes < 0) {
                                fprintf(stderr, "Error: cannot open input file\n");
                                error = true;
                                break;
                            }
                            else {
                                int fileInDes = open(myjobPtr->cmds[i].fileIn, O_RDWR);
                                dup2(fileInDes, STDIN_FILENO);
                                close(fileInDes);
                            }
                        }
                    }
                    // Connect pipe.
                    if (in != 0) {
                        dup2(in, 0);
                        close(in);
                    }
                    if (fd[1] != 1) {
                        dup2(fd[1], 1);
                        close(fd[1]);
                    }
                    execvp(myjobPtr->cmds[i].exec, myjobPtr->cmds[i].args);
                    fprintf(stderr,"Error: command not found\n");
                    exit(1);
                }
                // Parent.
                waitpid(-1, &status, 0);
                exit_status[i] = WEXITSTATUS(status);

                close(fd[1]);
                in = fd[0];
            }

            if (error) {
                continue;
            }

            pid = fork();
            if (pid == 0) {
                // Child.
                // Open last command's output redirect file.
                if (myjobPtr->cmds[myjobPtr->cmdCount-1].largerExist == 1) {
                    if (myjobPtr->cmds[myjobPtr->cmdCount-1].fileOut == NULL) {
                        fprintf(stderr,"Error: no out file\n");
                        continue;
                    }
                    else {
                        int fileOutDes = open(myjobPtr->cmds[myjobPtr->cmdCount-1].fileOut, O_WRONLY | O_CREAT, 0777);
                        if (fileOutDes < 0) {
                            fprintf(stderr, "Error: cannot open output file\n");
                            continue;
                        }
                        else {
                            dup2(fileOutDes,STDOUT_FILENO);
                            close(fileOutDes);
                        }
                    }
                }

                if (in != 0) {
                    dup2(in, 0);
                }
                execvp(myjobPtr->cmds[myjobPtr->cmdCount - 1].exec, myjobPtr->cmds[myjobPtr->cmdCount - 1].args);
                fprintf(stderr,"Error: command not found\n");
                exit(1);
            }
            else if (pid > 0) {
                // Parent.
                while(wait(&status) > 0);
                exit_status[myjobPtr->cmdCount-1] = WEXITSTATUS(status);
                fprintf(stderr, "+ completed '%s' ", userInputCopy);
                for (int i = 0; i < myjobPtr->cmdCount; i++) {
                    fprintf(stderr, "[%d]", exit_status[i]);
                }
                fprintf(stderr, "\n");
                continue;
            } else {
                perror("fork");
                exit(1);
            }
        }

        pid = fork();
        if (pid == 0) {
        	// Child.
            if (strcmp(myjobPtr->cmds[0].exec,"exit") == 0 && jobCount != 0) {
                fprintf(stderr,"Error: active jobs still running\n");
                exit(1);
            }
            execvp(myjobPtr->cmds[0].exec, myjobPtr->cmds[0].args);
            fprintf(stderr,"Error: command not found\n");
            exit(1);
        } else if (pid > 0) {
        	// Parent.
            // Store current command background job into arrays.
            if (myjobPtr->background) {
                int strLength = strlen(userInputCopy);
                jobInput[jobCount] = malloc((strLength + 1) * sizeof(char));
                for (int i = 0; i < strLength + 1; i++) {
                    jobInput[jobCount][i] = userInputCopy[i];
                }
                jobPid[jobCount] = pid;
                jobStatus[jobCount] = status;
                jobCount++;
                checkBgComplete(&jobCount, jobPid, jobStatus, jobInput);
            }

            // If current command has no '&'.
            if (myjobPtr->background == 0) {
                waitpid(pid, &status, 0);
                checkBgComplete(&jobCount, jobPid, jobStatus, jobInput);
                fprintf(stderr, "+ completed '%s' [%d]\n", userInputCopy, WEXITSTATUS(status));
                //We use parent process duing file redirection so we reset our stdin and stdout
                dup2(saveStdout,STDOUT_FILENO);
                dup2(saveStdin,STDIN_FILENO);
                close(saveStdin);
                close(saveStdout);
            }

        }
        else {
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
