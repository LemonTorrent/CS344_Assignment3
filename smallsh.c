#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>

// global variables
const int inputLength = 100;
int status = 0;
int background_status;
int foreground_mode = 0;

// structure to store prompt information
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

/*
struct argument {
    char *value;
    struct argument *next;
};
*/

/*
 * This function was used to print the prompt structure while coding
*/
void printPrompt(struct prompt* aprompt){
    // print command, output, input, and background information
    printf("%s, is Output: %i, Ofile: %s, is Input: %i, Ifile: %s, Bground: %i ", aprompt->command,
               aprompt->isOutput,
               aprompt->ofile,
               aprompt->isInput,
               aprompt->ifile,
			   aprompt->bground
			   );

    // print arguments
    printf("args: ");
    for (int i = 0; i < aprompt->numArgs; i++) {
        printf("%s ", aprompt->arglist[i]);
    }
    
    printf("\n");
}

/*
 * Expands variables $$ to process ID number
*/
char* expandVar(char str1[]) {
    // define variables
    char str2 [inputLength];
    char temp [inputLength];
    char tempCmd [inputLength];

    // copy str1 into tempCmd
    strcpy(tempCmd, str1);
    
    // get process ID
    int numVal = getpid();
    sprintf(temp, "%d", numVal);

    // add newline character
    tempCmd[strlen(tempCmd)] = '\n';

    // tokenize string by $$ and insert process id in place of $$
    char * token1 = strtok(tempCmd, "$$");
    strcpy(str2, token1);
    strcat(str2, temp);
    char * token2 = strtok(NULL, "$$");
    
    // continue replacing while there are still tokens in the string
    while (token2 != NULL) {
        strcpy(token1, token2);
        token2 = strtok(NULL, "$$");
        strcat(str2, token1);

        if (token2 != NULL){
            strcat(str2, temp);
        }
        
    }

    // remove the newline character
    str2[strcspn(str2, "\n")] = 0;
    strcpy(str1, str2);

    // return final string
    return str1;
}

/* 
*  Parse the current line which was taken as input and create a
*  prompt struct with the data in this line
*  This function was taken from the student example in Assignment 1.
*/
struct prompt *createNode(char *string, int length)
{
    // allocate memory for prompt
    struct prompt *currPrompt = malloc(sizeof(struct prompt));


    // set default arguments of isOfile and isIfile
    bool isOfile = false;
    bool isIfile = false;
    
    // start counting the number of arguments at 0
    int argIndex = 0;

    // flush output
    fflush(stdout);

    // if there is a & character at the end, set bground to true
    if (string[length - 1] == '&') {
        currPrompt->bground = true;
        
        // remove & char from end
        string[strlen(string)-1] = '\0'; // removing the last character &
    }
    
    // flush output
    fflush(stdout);

    // tokenize remaining string
    char * token = strtok(string, " ");
    
    // allocate space for command, then copy first word as the command
    currPrompt->command = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currPrompt->command, token);

    // also copy to the argument list
    currPrompt->arglist[argIndex] = token;
    
    // increment number of words by 1
    argIndex += 1;
    
    // while there are still tokens, move tokens to part of the data structure
    while( token != NULL) {
        // get next token
        token = strtok(NULL, " ");

        if (token != NULL){

            // if token is <, indicate that there should be an input file
            if (strcmp(token, "<") == 0) {
                isIfile = true;
                isOfile = false;
                currPrompt->isInput = true;
            } else if (strcmp(token, ">") == 0){
                // if token is >, indicate that there should be output file
                isOfile = true;
                isIfile = false;
                // currPrompt->isOutput = true;
            }

            // if neither input nor output file, add word to argument list
            if (isIfile == false && isOfile== false && strcmp(token, "<") != 0 &&  strcmp(token, ">") != 0) {
                currPrompt->arglist[argIndex] = token;
                
                // increment argument size by one
                argIndex += 1;

            }

            // copy token as input file
            if (isIfile == true && strcmp(token, "<") != 0 &&  strcmp(token, ">") != 0 ){
                currPrompt->ifile = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currPrompt->ifile, token);
                isIfile = false;
            } else if (isOfile == true && strcmp(token, "<") != 0 &&  strcmp(token, ">") != 0 ){
                // copy token as output file
                currPrompt->ofile = calloc(strlen(token) + 1, sizeof(char));
                strcpy(currPrompt->ofile, token);
                isOfile = false;
            }

        }
        
    }

    // set numArgs to the current index
    currPrompt->numArgs = argIndex;

    // set last value in array to null
    currPrompt->arglist[argIndex] = NULL;
    
    // return prompt structure
    return currPrompt;
}

/*
 * Calls the sleep function
*/
int sleepCall(struct prompt *cmdLine) {
    int file;
    int id;
    bool killedSleep = false;   // stores whether sleep was interupted by signal

    // set up signals
    struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0}, ignore_action = {0};
    ignore_action.sa_handler = SIG_IGN;
    // sigaction(SIGTSTP, &ignore_action, NULL);

    int childStatus;

    // Fork a new process
    pid_t spawnPid = fork();

    switch(spawnPid){
        case -1:
            // return error if fork fails
            perror("fork()\n");
            exit(1);
            break;
        case 0:
            // Child processes

            // If running in background, record stdout output to file
            if (cmdLine->ofile != NULL) {
                file = open(cmdLine->ofile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                dup2(file, 1);
            }
            
            // If in background, print process id
            if (cmdLine->bground == 1){
                printf("background pid is %i\n", getpid());
                printf(": ");
                fflush(stdout);
                // printf(": ");
            }
            
            // call sleep as command line process
            int exec_output = execvp(cmdLine->arglist[0], cmdLine->arglist);
            
            // exec only returns if there is an error
            perror(cmdLine->command);

            if (exec_output != NULL) {
                // return error if error from execvp
                printf("error executing\n");
                exit(2);
            } else {
                printf("successful execution\n");
            }

            

            exit(0);
            break;
        default:
            // In the parent process
            // Wait for child's termination
            spawnPid = waitpid(spawnPid, &childStatus, 0);
            
            // print success message after completing background sleep
            if (cmdLine->bground == 1){
                if (childStatus == 0) {
                    printf("background pid %i is done: exit value 0\n", spawnPid);
                }
                
            } else {
                // otherwise, set status for foreground sleep
                if (childStatus != 0) {
                    status = childStatus;
                } else if (childStatus == 0 ){
                    status = 0;
                }
                if (spawnPid == -1) {
                    status = 2;
                    // printf("bad and canceled\n");
                }
            }

            // Handle signal termination and print terminating message
            if (childStatus != 0) {
                printf("background pid %i is done: terminated by signal %i\n", spawnPid, childStatus);
                status = childStatus;
                killedSleep = true;
                // exit(15);
            }
            
            id = spawnPid;
            
            break;
            
    } 

    // if currently in background, print prompt for next line
    if (cmdLine->bground == true && killedSleep == false) {
        printf(": ");
    }
    
    status = 0;
    return 0;
}

/*
 * Changes directory inside command prompt
*/
int cdCall(struct prompt *cmdLine) {
    char s[inputLength];
    
    if (cmdLine->numArgs == 1) {
        // if there are no arguments after ch, go to home directory
        chdir(getenv("HOME"));
    } else {
        // change directory to provided address
        chdir(cmdLine->arglist[1]);
    }
    
    // Set status to 0
    status = 0;

    return 0;
}


/*
 * Prints the current value of status
*/
int statusCall(struct prompt *cmdLine) {

    printf("exit value %d\n", status);

    return 0;
}

/*
 * Calculates number of words, lines, and bytes in a file
*/
int countWords (char *currLine) {
    
    char *saveptr;

    // count words in the line
    int wordCount = 0;

    // Remove newline char
    currLine[strcspn(currLine, "\n")] = 0;
    // printf("Line after removing newline %s\n", currLine);

    // Divide the line into tokens
    char *token = strtok(currLine, " ");

    // while there are still tokens, increment the number of tokens by 1
    while( token != NULL) {
        
        token = strtok(NULL, " ");
        // printf("token = %s\n", token);
        wordCount += 1;
    }

    // return the number of words
    return wordCount;

}

/*
 * Calculates number of words, lines, and bytes in a file
*/
int wordCountFunctionality(struct prompt *cmdLine) {
    int numLines = 0;
    int numWords = 0;

    // store file name in variable filePath
    char filePath [50];
    strcpy(filePath, cmdLine->ifile);
    
    size_t nread;
    char *currLine = NULL;
    size_t len = 5;

    // open input file
    int file_descriptor = open(filePath, O_RDONLY);

    // give error if file descriptor = -1
	if (file_descriptor == -1){
		// printf("open() failed on \"%s\"\n", newFilePath);
		printf("Cannot open %s for input\n", filePath);
		exit(1);
	}

    // close if file is good input
    close(file_descriptor);

    // open for input
    FILE *file = fopen(filePath, "r");
    getline(&currLine, &len, file);
    
    // if the length of the current line, add the count of the words
    // and increment the line count by 1
    if (strlen(currLine) > 0) {
        numWords += countWords(currLine);
        numLines += 1;
    }
    
    // while there are still lines to read, add number of words and increment 
    // the number of lines
    while ((nread = getline(&currLine, &len, file)) != -1) {
        numWords += countWords(currLine);
        numLines += 1;
    }

    // print number of lines and words
    printf("  %i  %i  ", numLines, numWords);

    // get number of bytes from the stat property of the file
    struct dirent *aDir;
    time_t lastModifTime;
    struct stat dirStat;
    int i = 0;
    char entryName[256];

    stat(filePath, &dirStat);

    // print number of bytes
    printf("%i\n", dirStat.st_size);

    //close input file
    close(file_descriptor);
    
    return 0;
}

/*
 * Calls defined word count function in child by forking
*/
int wordCount(struct prompt *cmdLine){

    int childStatus;
    int file;
    int id;

    // Fork a new process
    pid_t spawnPid = fork();

    switch(spawnPid){
        case -1:
            // return error if fork fails
            perror("fork()\n");
            exit(1);
            break;
        case 0:
            // child processes
            if (cmdLine->ofile != NULL) {
                // printf("Opening output file\n");
                file = open(cmdLine->ofile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                dup2(file, 1);
            }
            
            // print word, line, and byte count to screen
            wordCountFunctionality(cmdLine);
            
            // printf("after calling wc\n");

            // perror(cmdLine->command);

            exit(0);
            break;
        default:
            // In the parent process
            // Wait for child's termination
            spawnPid = waitpid(spawnPid, &childStatus, 0);

            // adjust child return status
            if (childStatus != 0) {
                status = 1;
            } else {
                status = 0;
            }
            id = spawnPid;
            // exit(0);
            
            break;

    }
    return 0;
}


/*
 * Calls command line functions that already exist
*/
int existingFunct (struct prompt *cmdLine) {
    
    int file;
    int id;
    
    
    if (strcmp(cmdLine->command, "sleep") == 0) {
        // call sleep function if choice is to sleep
        sleepCall(cmdLine);

    } else {
        // call other functions
        int childStatus;

        // Fork a new process
        pid_t spawnPid = fork();

        switch(spawnPid){
            case -1:
                // runs if error forking
                perror("fork()\n");
                exit(1);
                break;
            case 0:
                // run child processes

                // if indicated to output to file, put in file "ofile"
                if (cmdLine->ofile != NULL) {
                    file = open(cmdLine->ofile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                    dup2(file, 1);
                }
                
                // run process
                int exec_output = execvp(cmdLine->arglist[0], cmdLine->arglist);
                // exec only returns if there is an error
                perror(cmdLine->command);
                if (exec_output != NULL) {
                    
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
                
                // adjust the exit status of the child
                if (childStatus != 0) {
                    status = 1;
                } else {
                    status = 0;
                }
                id = spawnPid;
                
                break;
                
        } 
        
    }
    
    return 0;
}

/*
 * Function detailing response when SIGINT is encountered
*/
void handle_SIGINT(int signo){
    // print termination information
	char* message = "terminated by signal 2\n";
    write(STDOUT_FILENO, message, 15);
    status = 2;
    return;
}

/*
 * Function detailing response when SIGTSTP is encountered
*/
void handle_SIGTSTP(int signo){
	char* message1 = "\nEntering foreground only mode\n";
    char* message2 = "\nExiting foreground only mode\n";

    if (foreground_mode == 0) {
        // if not in foreground mode, enter foreground mode
        write(STDOUT_FILENO, message1, 31);
        foreground_mode = 1;
    } else {
        // if  in foreground mode, exit foreground mode
        write(STDOUT_FILENO, message2, 31);
        foreground_mode = 0;
    }
	
    return;
	// exit(0);
}

int main(int argc, char *argv[]){
    struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0}, ignore_action = {0};

    
    char *buffer;
    size_t bufsize = 5;
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
        // ignore SIGINT
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
        
        // if we are in a child, return 0 to return to parent
        if (spawnpid == 0) {
            return 0;
        }

        // if the previous command wasn't a background process, print command line prompt
        if (hasSpawned == false ){
            printf(": ");
        }
        
        hasSpawned = false;

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
                        // printf("I am the child! processID = %d\n", getpid());
                        
                        break;
                    default:
                        // printf("I am the parent! processID = %d\n", getpid());
                        // delay (1);
                        // NULL;
                        // NULL;
                        hasSpawned = true;
                }
            }

            
            // if the current process isn't a parent node who has just forked:
            // perform the following functionality
            if (hasSpawned == false){
                // Check if command is "echo"
                if (strcmp(list->command, "echo") == 0){
                    // print contents of echo
                    for (int i = 1; i < list->numArgs; i ++) {
                        printf("%s ", list->arglist[i]);
                    }

                    printf("\n");
                
                // check if command is for sleep, then call respective function
                } else if (strcmp(list->command, "sleep") == 0) {
                    
                    sleepCall(list);

                // check if command is cd, call respecive function
                } else if (strcmp(list->command, "cd") == 0) {
                    // printf("cd\n");
                    cdCall(list);

                // check if command is status, call respective function
                } else if (strcmp(list->command, "status") == 0) {
                    
                    statusCall(list);

                // check if command is wc, call respective functionality
                } else if (strcmp(list->command, "wc") == 0) {
                    
                    // if there was a < when taking input, check input file
                    if (list->isInput == true) {

                        // Open file to check if it exists
                        int file_descriptor = open(list->ifile, O_RDONLY);

                        // if file doesn't exist, print error message
                        if (file_descriptor == -1){
                            printf("cannot open %s for input\n", list->ifile);

                        } else {
                            // otherwise call word count function
                            close(file_descriptor);
                            wordCount(list);
                        }
                    // check if there is no < when taking input
                    } else if (list->arglist[1] != NULL) {
                        
                        // check file input
                        int file_descriptor = open(list->arglist[1], O_RDONLY);

                        // if file doesn't exist, print error message
                        if (file_descriptor == -1){
                            printf("cannot open %s for input\n", list->arglist[1]);

                        } else {
                            // if file exists, call respective function
                            close(file_descriptor);
                            existingFunct(list);
                        }
                        
                    } else {
                        // input was solely "wc" and doesn't have enough arguments
                        printf("wc: not enough arguments\n");
                    }

                } else {
                    // if the command isn't one of the above functions, call the 
                    // functionality for existing functions
                    existingFunct(list);
                        
                }

            }
        }

    // repeat while there isn't a command for exit
    } while (strcmp(buffer, "exit") !=0);

	return 0;
}