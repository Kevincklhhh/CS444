#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
int fg_pids[MAX_TOKEN_SIZE]; // record running foreground processes
int num_fg_pids = 0;
int bg_pids[MAX_TOKEN_SIZE]; // record running background processes
int num_bg_pids = 0;
int sigint_received = 0; // indicate if ctrl c is pressed
/* Splits the string by space and returns the array of tokens
 *
 */
char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for (i = 0; i < strlen(line); i++)
	{

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t')
		{
			token[tokenIndex] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
		}
		else
		{
			token[tokenIndex++] = readChar;
		}
	}
	free(token);
	tokens[tokenNo] = NULL;
	return tokens;
}

void reap_child(int status)
{ // iterate over the list of background processes
	for (int i = 0; i < MAX_NUM_TOKENS; i++)
	{
		if (bg_pids[i] != 0)
		{
			if (waitpid(bg_pids[i], &status, WNOHANG) < 0)
			{
				printf("Shell: Background process finished\n");
				bg_pids[i] = 0;
				num_bg_pids--;
			}
		}
	}
}
// signal handler for SIGINT
void sigint_handler(int sig)
{
	sigint_received = 1;
	if (num_fg_pids == 0)
	{
		// No foreground processes to terminate
		return;
	}
	else
	{
		// Terminate all foreground processes
		printf("\n");
		int i;
		for (i = 0; i < num_fg_pids; i++)
		{
			kill(fg_pids[i], SIGINT);
		}
	}
}

int main(int argc, char *argv[])
{

	char line[MAX_INPUT_SIZE];
	char **tokens;
	int i;
	int status;						// stores child process status
	signal(SIGINT, sigint_handler); // register signal handler for SIGINT

	FILE *fp;
	if (argc == 2)
	{
		fp = fopen(argv[1], "r");
		if (fp < 0)
		{
			printf("File doesn't exists.");
			return -1;
		}
	}

	while (1)
	{
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if (argc == 2)
		{ // batch mode
			if (fgets(line, sizeof(line), fp) == NULL)
			{ // file reading finished
				break;
			}
			line[strlen(line) - 1] = '\0';
		}
		else
		{ // interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}

		reap_child(status); // reap finished background processes

		line[strlen(line)] = '\n'; // terminate with new line
		tokens = tokenize(line);

		sigint_received = 0;
		int is_background = 0; // they indicate if the user input is background, sequence or parallel
		int is_sequence = 0;
		int is_parallel = 0;
		int command_indices[MAX_NUM_TOKENS]; // record the index of starting of a command in the tokens array
		int num_commands = 1;
		command_indices[0] = 0;

		for (i = 0; tokens[i] != NULL; i++)
		{ // parse the commands. If there are multiple commands, record the start index of a command in the tokens array
			if (strcmp(tokens[i], "&") == 0)
			{
				is_background = 1;
				tokens[i] = NULL;
			}
			else if (strcmp(tokens[i], "&&") == 0)
			{
				is_sequence = 1;
				tokens[i] = NULL;
				command_indices[num_commands] = i + 1;
				num_commands++;
			}
			else if (strcmp(tokens[i], "&&&") == 0)
			{
				is_parallel = 1;
				tokens[i] = NULL;
				command_indices[num_commands] = i + 1;
				num_commands++;
			}
		}

		if (tokens[0] == NULL)
		{
			// Empty command,display prompt again
			continue;
		}

		if (strcmp(tokens[0], "cd") == 0)
		{
			// Implement cd
			if (tokens[1] == NULL)
			{
				chdir(getenv("HOME"));
			}
			else
			{
				// cd failed
				if (chdir(tokens[1]) != 0)
				{
					printf("Shell: Incorrect command\n");
				}
			}
			continue;
		}
		else if (strcmp(tokens[0], "exit") == 0)
		{
			// Exit command terminate all background processes
			for (int i = 0; i < num_bg_pids; i++)
			{
				kill(bg_pids[i], SIGTERM);
			}
			// Free dynamically allocated memory
			for (i = 0; tokens[i] != NULL; i++)
			{
				free(tokens[i]);
			}
			free(tokens);
			exit(0);
		}

		// executing commands in different modes
		if (is_background)
		{
			pid_t pid = fork(); // fork a child process
			if (pid == 0)
			{
				setpgid(0, 0);
				if (execvp(tokens[0], tokens) < 0)
				{
					printf("Shell: Incorrect command\n");
					exit(EXIT_FAILURE);
				};
			}
			else if (pid < 0)
			{
				perror("fork");
			}
			else
			{ // parent process records running background processes
				bg_pids[num_bg_pids++] = pid;
			}
		}
		else if (is_parallel)
		{
			for (int i = 0; i < num_commands; i++)
			{ // iterate over all parsed commands and arguments
				pid_t pid = fork();
				if (pid == 0)
				{ // child process execute
					execvp(tokens[command_indices[i]], &tokens[command_indices[i]]);
					printf("Shell: Incorrect command\n");
					exit(1);
				}
				else if (pid < 0)
				{ // failed to fork
					perror("fork");
					exit(1);
				}
				else
				{ // parent process stores pid
					fg_pids[i] = pid;
					num_fg_pids++;
				}
			}
			for (int i = 0; i < num_commands; i++)
			{ // wait for all running process to finish
				if (fg_pids[i] == 0)
				{
					continue;
				}
				waitpid(fg_pids[i], &status, 0);
				fg_pids[i] = 0;
				num_fg_pids--;
			}
		}
		else if (is_sequence)
		{
			for (int i = 0; i < num_commands; i++)
			{ // iterate over all parsed commands and arguments
				if (sigint_received)
				{ // if ctrl c is pressed, ignore all subsequent commands
					sigint_received = 0;
					break;
				}
				pid_t pid = fork();
				if (pid == 0)
				{ // child process execute
					execvp(tokens[command_indices[i]], &tokens[command_indices[i]]);
					exit(1);
				}
				else if (pid < 0)
				{ // failed to fork
					perror("fork");
					exit(1);
				}
				else
				{ // wait for each running process to finish
					wait(&status);
					if (WIFEXITED(status))
					{
						int exit_status = WEXITSTATUS(status);
						if (exit_status != 0)
						{
							printf("Shell: Incorrect command\n");
						}
					}
				}
			}
			if (sigint_received)
			{
				sigint_received = 0;
				continue;
			}
		}
		else
		{
			// single command running in foreground
			pid_t pid = fork();
			if (pid == 0)
			{ // child process
				if (execvp(tokens[0], tokens) < 0)
				{
					printf("Shell: Incorrect command\n");
				};
				// execvp returns only if an error occurs
				exit(EXIT_FAILURE);
			}
			else if (pid < 0)
			{ // error occurred
				perror("fork");
			}
			else
			{ // parent process
				wait(&status);
			}
		}
		// Freeing the allocated memory
		for (i = 0; tokens[i] != NULL; i++)
		{
			free(tokens[i]);
		}
		free(tokens);
	}
	return 0;
}
