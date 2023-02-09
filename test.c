#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAX 1000 // max number of letters to be supported
#define LIST 100 // max number of commands to be supported

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

// Greeting shell during startup
void init_shell()
{
	clear();
	printf("\n\n\n\n------------------------------------------------"
		"------------------------------------------------");
	printf("\n\n\n\t****Bash****");
	printf("\n\n\tEnter commands or Type batch to run file");
	printf("\n\n\n\n--------------------------------------------"
		"---------------------------------------------------------");
	char* user = getenv("USER");
	printf("\n\n\nLogged in: @%s", user);
	printf("\n");
	sleep(1);
	clear();
}

// Function to take input
int getInput(char* str)
{
	char* b;
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("\n%s", cwd);
	b = readline(">>> ");
	if (strlen(b) != 0) {
		add_history(b);
		strcpy(str, b);
		return 0;
	} else {
		return 1;
	}
}
void printDir()
{
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("%s", cwd);
}

void execArgs(char** parsed)
{
	// Forking a child
	pid_t pid = fork();

	if (pid == -1) {
		printf("\nFailed forking child..");
		return;
	} else if (pid == 0) {
		if (execvp(parsed[0], parsed) < 0) {
			printf("\nCould not execute command..");
		}
		exit(0);
	} else {
		// waiting for child to terminate
		wait(NULL);
		return;
	}
}

void execArgsPiped(char** parsed, char** parsedpipe)
{
	// 0 is read end, 1 is write end
	int pipefd[2];
	pid_t p1, p2;

	if (pipe(pipefd) < 0) {
		printf("\nPipe could not be initialized");
		return;
	}
	p1 = fork();
	if (p1 < 0) {
		printf("\nCould not fork");
		return;
	}

	if (p1 == 0) {
		// Child 1 executing..
		// It only needs to write at the write end
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);

		if (execvp(parsed[0], parsed) < 0) {
			printf("\nCould not execute command 1..");
			exit(0);
		}
	} else {
		// Parent executing
		p2 = fork();

		if (p2 < 0) {
			printf("\nCould not fork");
			return;
		}

		// Child 2 executing..
		// It only needs to read at the read end
		if (p2 == 0) {
			close(pipefd[1]);
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[0]);
			if (execvp(parsedpipe[0], parsedpipe) < 0) {
				printf("\nCould not execute command 2..");
				exit(0);
			}
		} else {
			// parent executing, waiting for two children
			wait(NULL);
			wait(NULL);
		}
	}
}

// Help command builtin
void openHelp()
{
	puts("\n************** WELCOME ************"
		"\nEnter commands or type 'batch'"
		"\n>improper space handling");

	return;
}

// Function to execute builtin commands
int ownCmdHandler(char** parsed)
{
	int NoOfOwnCmds = 4, i, switchOwnArg = 0;
	char* ListOfOwnCmds[NoOfOwnCmds];
	char* username;

	ListOfOwnCmds[0] = "exit";
	ListOfOwnCmds[1] = "cd";
	ListOfOwnCmds[2] = "help";
	ListOfOwnCmds[3] = "hello";

	for (i = 0; i < NoOfOwnCmds; i++) {
		if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
			switchOwnArg = i + 1;
			break;
		}
	}

	switch (switchOwnArg) {
	case 1:
		printf("\nGoodbye\n");
		exit(0);
	case 2:
		chdir(parsed[1]);
		return 1;
	case 3:
		openHelp();
		return 1;
	case 4:
		username = getenv("USER");
		printf("\nHello %s.", username);
		return 1;
	default:
		break;
	}

	return 0;
}

int parsePipe(char* str, char** strpiped)
{
	int i;
	for (i = 0; i < 2; i++) {
		strpiped[i] = strsep(&str, "|");
		if (strpiped[i] == NULL)
			break;
	}

	if (strpiped[1] == NULL)
		return 0; // returns zero if no pipe is found.
	else {
		return 1;
	}
}

// function for parsing command words
void parseSpace(char* str, char** parsed)
{
	int i;

	for (i = 0; i < LIST; i++) {
		parsed[i] = strsep(&str, " ");

		if (parsed[i] == NULL)
			break;
		if (strlen(parsed[i]) == 0)
			i--;
	}
}

int processString(char* str, char** parsed, char** parsedpipe)
{

	char* strpiped[2];
	int piped = 0;

	piped = parsePipe(str, strpiped);

	if (piped) {
		parseSpace(strpiped[0], parsed);
		parseSpace(strpiped[1], parsedpipe);

	} else {

		parseSpace(str, parsed);
	}

	if (ownCmdHandler(parsed))
		return 0;
	else
		return 1 + piped;
}

int main()
{
	char inputString[MAX], *parsedArgs[LIST];
	char* parsedArgsPiped[LIST];
	int execFlag = 0;
	init_shell();

	while (1) {
		if (getInput(inputString))
			continue;
		char splitStrings[100][100], batchStrings[100][100];
		int i = 0, j = 0, cnt = 0, ex = 0;
		if (!strcmp(inputString,"batch"))
                {
                        FILE* fpt;
                        fpt = fopen("./aa.bat", "r");
                        while(fgets(batchStrings[j], MAX, fpt))
                        {
                                batchStrings[j][strlen(batchStrings[j]) - 1] = '\0';
                                j++;
                        }
			cnt = j;
			for (i = 0; i < cnt; i++){
                      		if (strcmp(batchStrings[i], "exit")){
					printf("Entered command is: %s\n", batchStrings[i]);
                            		execFlag = processString(batchStrings[i],parsedArgs, parsedArgsPiped);
                           		if (execFlag == 1)
                                		execArgs(parsedArgs);
                              		 if (execFlag == 2)
                                		execArgsPiped(parsedArgs, parsedArgsPiped);
                      		 }else{
                              			ex =1;
                     			 }
                	}
                	if (ex){
				printf("Entered command is: EXIT");
                        	printf("\nGoodbye\n");
                        	exit(0);
                	}
				

		}else{
			for (i = 0; i <= (strlen(inputString)); i++) {
	                        if (inputString[i] == ';' || inputString[i] == '\0') {
                	                splitStrings[cnt][j] = '\0';
        	                        cnt++; //for next word
                        	        j = 0; //for next word, init index to 0
                        	}else {
                                	splitStrings[cnt][j] = inputString[i];
                                	j++;
                        	}
                	}
                	for (i = 0; i < cnt; i++){
                      		if (strcmp(splitStrings[i], "exit")){
                              		execFlag = processString(splitStrings[i],parsedArgs, parsedArgsPiped);
                                	if (execFlag == 1)
                                        	execArgs(parsedArgs);
                                	if (execFlag == 2)
                                        	execArgsPiped(parsedArgs, parsedArgsPiped);
                      		}else {
                             	ex = 1;
                      		}
                	}
                	if (ex){
                        	printf("\nGoodbye\n");
                        	exit(0);
                	}
		}
	}
	return 0;
}

