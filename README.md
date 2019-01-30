
# Project overview
There are four main tasks/difficulties to do for this project:

1. Command information storation
2. Pipelines implementation
3. Background task implementation
4. Error management

We handle the first task by constructing C struct. We do not realize that we
should use struct until we reach phase 5. Since we need to deal with file
redirection during that phase, we find it is extremely inconvenient to store all
user inputs in one array. There is no way to tell which field belongs to which
type of input (input file, output file, arguments, executor). Therefore, we
realize the importance of using structs, and review a lot of material about how
to construct an object and implement our own constructor under the circumstance
that C does not have constructor function.

We handle the second task by creating child and let every pair to have their
pipe, so that when the last command wants to output, we redirect its stdout
to pipe[1], and set the next command stdin to pipe[0] . We think it is an updated
version of file redirection.

We handle the third task by analyzing the logic behind background jobs. We let
main program fork child and let child to complete the job, and store child’s
information in arrays, so that every time parent is receiving new commands, it
knows whether some children has finished their jobs.

We handle the error management in each phase. There are basically two kinds
Of error. One is caught in parent, like mislocated symbol. Another is child’s
runtime error, which we have to let child execute it and receive the child’s
status to produce corresponding error Message. So in our program in some places,
before we fork child to execute the command, we let parent to check whether there
is something wrong. If there is, we throw the error and continue the main while
loop. We also compare the result of sshell_ref with our sshell and read carefully
the prompt to make sure we print the correct error message at correct position.
We also point out in piazza that if there are multiple error, it is very much
personal choice to print out which error first if we want to print very specific
error message. I have to say, we spend a lot of time arranging our code to produce
the same error message then realize it would be very hard.  

# Project implementation

### Struct implementation
We construct two structs, one is used to store if there is any background command,
the number of commands, and all user input commands. The other is used
to separate all user input commands into specific sub fields. For example, we
declare two char * and store our input file and output file separately in the
struct.

The reason for doing so is to make our lives easier when dealing with pipes and
background command. After storing information separately, we can easily use those
stored information. We can know whether we have certain information simply by
checking if it is NULL nor not.

Below are our structs
```
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
```

### Pipeline implementation
Since we learn how to implement file redirection during class, implementing
pipeline is just an update version of file redirection. However, there are
definitely more things we need to understand in order to complete this phase.
We use the link below as our guideline in this phase.
<https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell>

During our implementation, we find that for the first command, we want the
user input instead of input from the previous command; and for the last command,
we want to store output into STDOUT. Therefore, we do not connect those
specific command with the file. Besides those two commands, we connect the
read entry of the file with STDIN and the write entry of the file with STDOUT
before we execute other arguments.
There is a very tricky thing about the implementation is that, when parent fork
a child, it gives everything to it including its entry point. If that entry point
is a pipe index entry and it is closed in a parent, the child has a closed entry
at that position too. However, the closed one is the write index of the pipe, so
child doesn’t need to use it. What child wants is the read entry point and it is
not closed until dup it to its STDIN. And, since what we manipulate is always the
STDIN and STDOUT. We do not affect STDERR and child can print the error message
if it fails on execvp. Besides during this process, we never change the parent,
which is the main program’s STDIN and STDOUT. Parent only creates pipe for child
to use.


### Background implementation
The first task for this phase is to understanding how does background jobs works.
Unlike usual jobs that wait for the child processor to complete, background jobs
does not do that. Thus if we have a background job, we do not use:
>waitpit(-1, &status, 0).

Instead, we use the line below to check if the child has processor completed
or not.
> waitpid(jobPid[i], &jobStatus[i], WNOHANG) > 0

We store all the background jobs with its pid value, status and userinput in
separate arrays in order to use them to check its status. When the above statement
is true, it means the child processor has completed the background task.
Therefore, we print the "+ complete" statement and remove those completed jobs
out of the arrays.


The second task for this phase is where to check if the child processor has
completed. We check it at three places.
1. The first place is right after we print sshell and when the user press
return. We do so because when the user enters return when the job completes,
the statement "+ complete" should be printed on to the screen.
2. The second and third places are in the parent processor, we check whether a
child processor has completed when we currently receive a background job and
when we do not receive a background job. We need to check it under both
situation because in either situation, we want to print the "+ complete"
statement once the job has finished.

# Miscellaneous
### Pointers and string
In C since there is no string type, we can only use a char pointer to manipulate
a string. We review a lot of information and we find that as long as we hold this
rule, a lot of problems can be solved: in C if we want to represent an array, we
use a char pointer points to a char and let the string ends with ‘\0’, null
character. For example, when we are constructing the Command object in pipeline
group, we let pointer points to the first character of the command, and replace
‘|’ with null character. By this, the pointer represents a command and we store
the information in the object.

### Error management
Before this assignment we never thought error checking would be important. We
just assume files can be open and commands can be execute. However, this program
strengthens the importance of doing so. If we don’t check, the output is
different from correct answer and we lose points. As a result, we gradually
develop the habit to check error, and in order to know when to check error, we
read function’s manual page thoroughly. If it is in the past, we would only care
about the description of the capability of the function, but never care about
the return value, which includes possible error information.

### Limitation on background command
Our code cannot implement background commands including file redirection and
pipeline groups. We have an idea to do so, which is to create a big function to
wrap what we have now and fork a child to execute it, while the parent is waiting.
Even though we have the idea, we really don’t have enough time to implement. We
started to program as soon as professor lectured enough to finish phase 4, we
found we have to roll back and construct struct at the very beginning, which
costs a lot of time. Besides, we spent a lot of time learning C again after we
get used to code in high level language. However, we do learn a lot during this
experience, and we are confident to finish the next assignment with full
functioning.

