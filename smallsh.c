#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>


const int inputLength = 100;
int status = 0;
int background_status;
int foreground_mode = 0;

struct prompt
{
    char *command;
    // struct argument *arglist;
    char *arglist[10];
    char *ofile;
    char *ifile;
    bool bground;
    bool isOutput;
    bool isInput;
    int numArgs;

};

struct argument {
    char *value;
    struct argument *next;
};

void printPrompt(struct prompt* aprompt){
    printf("%s, is Output: %i, Ofile: %s, is Input: %i, Ifile: %s, Bground: %i ", aprompt->command,
               aprompt->isOutput,
               aprompt->ofile,
               aprompt->isInput,
               aprompt->ifile,
			   aprompt->bground
			   );
    // struct argument *thisarg = aprompt->arglist;
    printf("args: ");
    for (int i = 0; i < aprompt->numArgs; i++) {
        printf("%s ", aprompt->arglist[i]);
    }
    /*while (thisarg != NULL) {
        printf(" %s", thisarg->value);
        thisarg = thisarg->next;
    }
    */
    printf("\n");
}

// Expands variables $$
char* expandVar(char str1[]) {
    char str2 [inputLength];
    // char *ret;
    char temp [inputLength];
    char tempCmd [inputLength];
    strcpy(tempCmd, str1);
    // int j = 0;
    // int numVal = 147;
    int numVal = getpid();
    // printf("Process id = %d\n", getpid());
    
    sprintf(temp, "%d", numVal);

    tempCmd[strlen(tempCmd)] = '\n';

    char * token1 = strtok(tempCmd, "$$");
    strcpy(str2, token1);
    strcat(str2, temp);
    char * token2 = strtok(NULL, "$$");
    
    while (token2 != NULL) {
        strcpy(token1, token2);
        token2 = strtok(NULL, "$$");
        strcat(str2, token1);

        if (token2 != NULL){
            strcat(str2, temp);
        }
        
    }
    str2[strcspn(str2, "\n")] = 0;
    strcpy(str1, str2);
    return str1;
}

/* Parse the current line which is space delimited and create a
*  student struct with the data in this line
*  This function was taken from the student example in Assignment 1.
*/
struct prompt *createNode(char *string, int length)
{
    // printf("startin createNode\n");
    // printf("string %s\n", string);
    struct prompt *currPrompt = malloc(sizeof(struct prompt));

    // char *saveptr;

    // bool isArg = true;
    bool isOfile = false;
    bool isIfile = false;
    // bool isBground = false;
    // struct argument *thisarg = NULL;
    int argIndex = 0;

    // printf("after setting up \n");

    fflush(stdout);

    // printf("%c\n", string[length - 1]);
    if (string[length - 1] == '&') {
        // currPrompt->bground = calloc(1, sizeof(bool));

        currPrompt->bground = true;
        // printf("background: %i\n", currPrompt->bground);
        string[strlen(string)-1] = '\0'; // removing the last character &
    }
    // }
    

    // char *buffer [length];
    
    fflush(stdout);

    // printf("after printPrompt\n");

    char * token = strtok(string, " ");
    // printf("%s\n", token);

    // return;
    currPrompt->command = calloc(strlen(token) + 1, sizeof(char));
    // memset(currPrompt->command, 0, sizeof(currPrompt->command));
    strcpy(currPrompt->command, token);
    currPrompt->arglist[argIndex] = token;
    // printf("arglist %s\n", currPrompt->arglist[argIndex]);
    // printf("arglist %s\n", currPrompt->arglist[0]);
    argIndex += 1;
    // printf("after adding to arglist\n");

    // printf("%s\n", currPrompt->command);
    // loop through the string to extract all other tokens
    while( token != NULL) {
        // bool isBground = false;
        // printf( " %s\n", token ); //printing each token
        token = strtok(NULL, " ");
        // printf("token: %s\n", token);

        if (token != NULL){

            /*
            if (strcmp (token, "\n") == 0) {
                printf("newline\n");
            }

            if (token [strlen(token) - 1] == "\n") {
                printf("newline\n");
            }
            */
            
            if (strcmp(token, "<") == 0) {
                isIfile = true;
                isOfile = false;
            } else if (strcmp(token, ">") == 0){
                isOfile = true;
                isIfile = false;
            }

            if (isIfile == false && isOfile== false && strcmp(token, "<") != 0 &&  strcmp(token, ">") != 0) {
                // struct argument *currArg = malloc(sizeof(struct argument));
                // printf("parsing for arguments\n");

                currPrompt->arglist[argIndex] = token;
                // printf("arglist %s\n", currPrompt->arglist[argIndex]);
                // printf("arglist %s\n", currPrompt->arglist[0]);
                argIndex += 1;

            }

            if (isIfile == true && strcmp(token, "<") != 0 &&  strcmp(token, ">") != 0 ){
                currPrompt->ifile = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currPrompt->ifile, token);
                isIfile = false;
            } else if (isOfile == true && strcmp(token, "<") != 0 &&  strcmp(token, ">") != 0 ){
                currPrompt->ofile = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currPrompt->ofile, token);
                isOfile = false;
            }



        }
        
    }

    currPrompt->numArgs = argIndex;

    // printf("before setting index to null\n");
    currPrompt->arglist[argIndex] = NULL;
    // printf("after setting index to null\n");
    printPrompt(currPrompt);
    // printf("after printing prompt to null\n");

    return currPrompt;
}

/*
int exProcess(struct prompt *currNode) {
    int childStatus;

	// Fork a new process
	pid_t spawnPid = fork();

	switch(spawnPid){
        case -1:
            perror("fork()\n");
            exit(1);
            break;
        case 0:
            // In the child process
            // printf("CHILD(%d) running ls command\n", getpid());
            // Replace the current program with "/bin/ls"
            execlp("ls", "ls", NULL);
            // exec only returns if there is an error
            perror("execlp");
            // exit(2);
            break;
        default:
            // In the parent process
            // Wait for child's termination
            spawnPid = waitpid(spawnPid, &childStatus, 0);
            // printf("PARENT(%d): child(%d) terminated. Exiting\n", getpid(), spawnPid);
            // exit(0);
            break;
	} 

    return 0;
    
}
*/

int sleepCall(struct prompt *cmdLine) {
    int file;
    int id;
    printf("Sleep function\n");

    struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0}, ignore_action = {0};
    ignore_action.sa_handler = SIG_IGN;
    // sigaction(SIGTSTP, &ignore_action, NULL);

    int childStatus;

    // Fork a new process
    pid_t spawnPid = fork();

    switch(spawnPid){
        case -1:
            perror("fork()\n");
            exit(1);
            break;
        case 0:
            


            if (cmdLine->ofile != NULL) {
                file = open(cmdLine->ofile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                dup2(file, 1);
            }
            
            // In the child process
            // printf("CHILD(%d) running ls command\n", getpid());
            // Replace the current program with "/bin/ls"
            if (cmdLine->bground == 1){
                printf("background pid is %i\n", getpid());
                // printf(": ");
            }
            
            int exec_output = execvp(cmdLine->arglist[0], cmdLine->arglist);
            // exec only returns if there is an error
            perror(cmdLine->command);
            if (exec_output != NULL) {
                printf("error executing\n");
                // printf("background status now %d\n", background_status);
                // status = 1;
                // background_status = 1;
                // exit(1);
                // printf("background status now %d\n", background_status);
                exit(2);
            } else {
                printf("successful execution\n");
                // status = 0;
            }

            

            exit(0);
            break;
        default:
            // In the parent process
            // Wait for child's termination
            spawnPid = waitpid(spawnPid, &childStatus, 0);
            printf("Spawnpid %d\n", spawnPid);
            printf("childStatus %d\n", childStatus);


            if (cmdLine->bground == 1){
                if (childStatus == 0) {
                    printf("background pid %i is done: exit value 0\n", spawnPid);
                }
                
                // printf("\n: ");
            } else {
                if (childStatus != 0) {
                    status = 1;
                } else {
                    status = 0;
                }
                if (spawnPid == -1) {
                    status = 2;
                    printf("bad and canceled\n");
                }
            }
            id = spawnPid;
            // printf("PARENT(%d): child(%d) terminated. Exiting\n", getpid(), spawnPid);
            // exit(0);
            
            break;
            
    } 

    /*
    int length = atoi(cmdLine->arglist[1]);
    printf("sleeping %i\n", length);
    printf("Sleeping process %d\n", getpid());
    sleep(length);
    printf("\nsleeping complete, process id %d\n", getpid());
    */

    if (cmdLine->bground == true) {
        printf(": ");
    }
    
    status = 0;
    return 0;
}

int cdCall(struct prompt *cmdLine) {
    char s[inputLength];
    // printf("%s\n",getcwd(s,100));
    if (cmdLine->numArgs == 1) {
        chdir(getenv("HOME"));
    } else {
        chdir(cmdLine->arglist[1]);
    }
    // printf("%s\n",getcwd(s,100));
    status = 0;

    return 0;
}

int statusCall(struct prompt *cmdLine) {

    printf("exit value %d\n", status);

    return 0;
}

int existingFunct (struct prompt *cmdLine) {
    // printf("running existing function \n");
    int file;
    int id;
    // if (cmdLine->ofile != NULL) {
    // status = 0;

    // }
    
    if (strcmp(cmdLine->command, "sleep") == 0) {
        sleepCall(cmdLine);

    } else if (strcmp(cmdLine->command, "wc") == 0) {
        int num = cmdLine->numArgs;
        cmdLine->arglist[num] = cmdLine->ifile;
        cmdLine->numArgs += 1;
        int childStatus;

        // Fork a new process
        pid_t spawnPid = fork();

        switch(spawnPid){
            case -1:
                perror("fork()\n");
                status = 1;
                exit(1);
                break;
            case 0:
                if (cmdLine->ofile != NULL) {
                    file = open(cmdLine->ofile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                    dup2(file, 1);
                }
                
                // In the child process
                // printf("CHILD(%d) running ls command\n", getpid());
                // Replace the current program with "/bin/ls"
                execvp(cmdLine->arglist[0], cmdLine->arglist);
                // exec only returns if there is an error
                perror(cmdLine->command);
                // exit(2);
                break;
            default:
                // In the parent process
                // Wait for child's termination
                spawnPid = waitpid(spawnPid, &childStatus, 0);
                // printf("PARENT(%d): child(%d) terminated. Exiting\n", getpid(), spawnPid);
                // exit(0);
                break;
            
        } 
        // status = 0;
    } else {

        // uncomment to work on forking

        
        int childStatus;

        // Fork a new process
        pid_t spawnPid = fork();

        switch(spawnPid){
            case -1:
                perror("fork()\n");
                exit(1);
                break;
            case 0:
                if (cmdLine->ofile != NULL) {
                    file = open(cmdLine->ofile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                    dup2(file, 1);
                }
                
                // In the child process
                // printf("CHILD(%d) running ls command\n", getpid());
                // Replace the current program with "/bin/ls"
                int exec_output = execvp(cmdLine->arglist[0], cmdLine->arglist);
                // exec only returns if there is an error
                perror(cmdLine->command);
                if (exec_output != NULL) {
                    // printf("error executing\n");
                    // printf("background status now %d\n", background_status);
                    // status = 1;
                    // background_status = 1;
                    // exit(1);
                    // printf("background status now %d\n", background_status);
                    exit(2);
                } else {
                    // printf("successful execution\n");
                    // status = 0;
                }

                exit(0);
                break;
            default:
                // In the parent process
                // Wait for child's termination
                spawnPid = waitpid(spawnPid, &childStatus, 0);
                // printf("Spawnpid %d\n", spawnPid);
                // printf("childStatus %d\n", childStatus);
                if (childStatus != 0) {
                    status = 1;
                } else {
                    status = 0;
                }
                id = spawnPid;
                // printf("PARENT(%d): child(%d) terminated. Exiting\n", getpid(), spawnPid);
                // exit(0);
                
                break;
                
        } 
        

            NULL;
            // call functions with execvp here!
    }
    
    // printf("Spawnpid %d\n", id);

    return 0;
}

void handle_SIGINT(int signo){
	char* message = "terminated by signal 2\n";
    write(STDOUT_FILENO, message, 15);
    status = 3;
    
	// exit(0);
    return;
	// exit(0);
}

void handle_SIGTSTP(int signo){
	char* message1 = "Entering foreground only mode\n";
    char* message2 = "Exiting foreground only mode\n";

    if (foreground_mode == 0) {
        write(STDOUT_FILENO, message1, 30);
        foreground_mode = 1;
    } else {
        write(STDOUT_FILENO, message2, 30);
        foreground_mode = 0;
    }
	
    return;
	// exit(0);
}

int main(int argc, char *argv[]){
    struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0}, ignore_action = {0};

    // Register the ignore_action as the handler for SIGTERM, SIGHUP, SIGQUIT.
    



    char *buffer;
    size_t bufsize = 5;
    // size_t characters;
    bool reprompt = false;
    bool hasSpawned;
    int inputLen = inputLength;
    char input [inputLen];
    pid_t spawnpid = -5;
    background_status = 0;

    

    buffer = (char *)malloc(bufsize * sizeof(char));
    if( buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }

    // repeat while the input is not equal to exit
    do {
        ignore_action.sa_handler = SIG_IGN;
        sigaction(SIGINT, &ignore_action, NULL);
        // Fill out the SIGTSTP_action struct
        // Register handle_SIGTSTP as the signal handler
        SIGTSTP_action.sa_handler = handle_SIGTSTP;
        // Block all catchable signals while handle_SIGTSTP is running
        sigfillset(&SIGTSTP_action.sa_mask);
        // No flags set
        SIGTSTP_action.sa_flags = 0;
        sigaction(SIGTSTP, &SIGTSTP_action, NULL);

        // set reprompt and hasSpawned to false
        reprompt = false;
        hasSpawned = false;

        if (spawnpid == 0) {
            return 0;
        }

        printf(": ");

        // get input
        fgets(input, inputLen, stdin);

        // remove the newline character at the end of input
        input[strcspn(input, "\n")] = 0;
        // printf("Removing new line: %s\n", input);

        // if length of input = 30 and starts with "echo", print the argument
        if (strlen(input) >= 30) {
            if (strstr(input, "echo") != NULL) {
                for (int i = 5; i < strlen(input); i++) {
                    printf("%c", input[i]);
                }
                printf("\n");
                
            }

            
            reprompt = true;
        } else {
            // Call function to expand variable if $$ is found
            if (strstr(input, "$$") != NULL) {
                // printf("found substring\n");
                expandVar(input);
            }

            // Kill program if input is "exit"
            if (strcmp(input, "exit") == 0) {
                char *newargv[] = { "kill", "-SIGKILL", getpid()};
                int exec_output = execvp(newargv[0], newargv);
                return 0;

            }

            // If input starts with 0 or #, reprompt user
            if (strlen(input) == 0 || input[0] == '#'){
                // printf("reprompt\n");
                reprompt = true;
            }
        }

        // If user doesn't need to be reprompted, complete functionality
        if (reprompt == false){
            // printf("Doing something\n");
            // Create data structure containing input information
            struct prompt *list = createNode(input, strlen(input));
            
            // if in foreground mode, change command to foreground only
            if (foreground_mode == 1 && list->bground == 1) {
                list->bground = 0;
            }

            if (list->bground == 0) {
                // Fill out the SIGINT_action struct
                // Register handle_SIGINT as the signal handler
                SIGINT_action.sa_handler = handle_SIGINT;
                // Block all catchable signals while handle_SIGINT is running
                sigfillset(&SIGINT_action.sa_mask);
                // No flags set
                SIGINT_action.sa_flags = 0;
                sigaction(SIGINT, &SIGINT_action, NULL);
            }

            // If command is not equal to status
            if (list->bground == true && strcmp(list->command, "status") != 0) {
                spawnpid = fork();
                switch (spawnpid){

                    case -1:
                        // Code in this branch will be exected by the parent when fork() fails and the creation of child process fails as well
                        perror("fork() failed!");
                        exit(1);
                        break;
                    case 0:
                        // spawnpid is 0. This means the child will execute the code in this branch
                        // Change so 
                        sigaction(SIGINT, &ignore_action, NULL);
                        printf("I am the child! processID = %d\n", getpid());
                        
                        break;
                    default:
                        printf("I am the parent! processID = %d\n", getpid());
                        hasSpawned = true;
                }
            }

            // return 0;
            if (hasSpawned == false){
                sigaction(SIGTSTP, &ignore_action, NULL);

                if (strcmp(list->command, "echo") == 0){
                    for (int i = 1; i < list->numArgs; i ++) {
                        printf("%s ", list->arglist[i]);
                    }

                    // printf("yes in echo\n");
                    //struct argument *reading = list->arglist;
                    //while (reading != NULL){
                    //    printf("%s ", reading->value);
                    //    reading = reading->next;
                    //}
                    printf("\n");
                    
                } else if (strcmp(list->command, "sleep") == 0) {
                    // printf("sleeping\n");
                    sleepCall(list);
                } else if (strcmp(list->command, "cd") == 0) {
                    printf("cd\n");
                    cdCall(list);
                } else if (strcmp(list->command, "status") == 0) {
                    printf("status\n");
                    statusCall(list);
                } else if (strcmp(list->command, "wc") == 0) {
                    printf("word count\n");
                // } else if (strcmp(list->command, "kill") == 0 ||
                //         strcmp(list->command, "pkill") == 0) {
                //     printf("killing\n");
                } else {
                    // printf("doing something\n");
                    existingFunct(list);
                        
                }

            }
            // printf("Previous command: %s\n", list->command);
            // list = NULL;
            //strcpy(list->command, "");
        }


        
        // printf("after\n");

    } while (strcmp(buffer, "exit") !=0);
    // scanf()

	return 0;
}