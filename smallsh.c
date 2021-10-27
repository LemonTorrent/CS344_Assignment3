#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>


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

/* Parse the current line which is space delimited and create a
*  student struct with the data in this line
*  This function was taken from the student example in Assignment 1.
*/
struct prompt *createNode(char *string, int length)
{
    // printf("startin createNode\n");
    // printf("string %s\n", string);
    struct prompt *currPrompt = malloc(sizeof(struct prompt));

    char *saveptr;

    bool isArg = true;
    bool isOfile = false;
    bool isIfile = false;
    bool isBground = false;
    struct argument *thisarg = NULL;
    int argIndex = 0;

    printf("after setting up \n");

    fflush(stdout);

    // printf("%c\n", string[length - 1]);
    if (string[length - 1] == '&') {
        // currPrompt->bground = calloc(1, sizeof(bool));

        currPrompt->bground = true;
        // printf("background: %i\n", currPrompt->bground);
        string[strlen(string)-1] = '\0'; // removing the last character &
    }
    // }
    

    char *buffer [length];
    
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
        bool isBground = false;
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
                // printf("after adding to arglist\n");
                /*
                if (currPrompt->arglist == NULL){
                    
                    // thisarg = currPrompt->arglist;
                    printf("thisarg = %s\n", thisarg);
                    currArg->value = calloc(strlen(token) + 1, sizeof(char));
                    strcpy(currArg->value, token);
                    printf("after first arg\n");
                    currPrompt->arglist = currArg;
                } else {
                    thisarg = currPrompt->arglist;
                    while (thisarg->next != NULL) {
                        thisarg = thisarg->next;
                    }

                    currArg->value = calloc(strlen(token) + 1, sizeof(char));
                    strcpy(currArg->value, token);
                    thisarg->next = currArg;
                }
                */
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

int forkLs(){
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
	    execlp("ls", "ls", ">", "junk", NULL);
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

int sleepCall(struct prompt *cmdLine) {
    int length = atoi(cmdLine->arglist[1]);
    printf("sleeping %i\n", length);
    sleep(length);
    return 0;
}

int existingFunct (struct prompt *cmdLine) {
    printf("running existing function \n");
    int file;
    // if (cmdLine->ofile != NULL) {

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
                //if (exec_output == NULL) {
                    //return 0;
                // }
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
        

            NULL;
            // call functions with execvp here!
    }
    return 0;
}

int main(int argc, char *argv[]){

    char *buffer;
    size_t bufsize = 5;
    size_t characters;
    bool reprompt = false;
    int inputLen = 30;
    char input [inputLen];
    

    buffer = (char *)malloc(bufsize * sizeof(char));
    if( buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }

    do {
        reprompt = false;
        printf(": ");

        // get input
        fgets(input, inputLen, stdin);

        // remove the newline character at the end of input
        input[strcspn(input, "\n")] = 0;
        printf("%s\n", input);
        
        // if input is exit, exit from command prompt
        if (strcmp(input, "exit") == 0) {
            return 0;
        }

        // return;
        // printf("%zu characters were read.\n",characters);
        // printf("You typed: %s",buffer);
        // printf("%c\n ", buffer[0]);

        if (strlen(input) == 0 || input[0] == '#'){
            printf("reprompt\n");
            reprompt = true;
        }

        // printf("%d\n", reprompt);
        // return 0;

        if (reprompt == false){
            printf("Doing something\n");
            struct prompt *list = createNode(input, strlen(input));
            // printPrompt(list);
            printf("after creation\n");
            // return 0;
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
            } else if (strcmp(list->command, "status") == 0) {
                printf("status\n");
            } else {
                printf("doing something\n");
                existingFunct(list);
                // struct prompt *list = createNode(input, strlen(input));
                // printf("after node creation");
                //printf("after creation, printing prompt:\n");
                //printf("%s\n", list->command);
                //printf("Comparing to sleep: %i\n", strcmp(list->command, "sleep") == 0);

                // printPrompt(list);

                /*
                if (strcmp(list->command, "sleep") == 0) {
                    // printf("inside if loop\n");
                    // sleep(atoi(list->arglist->value));
                    // printf("%i\n", atoi(list->arglist->value));
                    int lenSleep = atoi(list->arglist->value);
                    sleep(lenSleep);
                } else if (strcmp(list->command, "cd") == 0) {
                    printf("Run implemented cd\n");
                } else if (strcmp(list->command, "status")) {
                    printf("Run impelemented status\n");
                } else {
                    printf("%i\n", strcmp(list->command, "sleep") == 0);
                    if (strcmp(list->command, "sleep") == 0) {
                        printf("entering sleep\n");
                        sleep(atoi(list->arglist->value));
                    }
                    printf("Running preimplemented commands\n");
                    */

                    
            }
        }
            /*
            } else if (strcmp(buffer, "ls\n") == 0) {
                // Run /bin/ls and pass it the argument -al
                // execl("/bin/ls", "/bin/ls", NULL);
                // createNode(buffer, characters);
                // forkLs();

                /* exec returns only on error */
                // perror("execl");  
                /*
            } else if (strcmp(buffer, "")) {
                NULL;
            } else {
                createNode(buffer, characters);

            }

            */

        // }

    } while (strcmp(buffer, "exit") !=0);
    // scanf()

	return 0;
}