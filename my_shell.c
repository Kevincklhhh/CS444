#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

// signal handler for SIGINT
void sigint_handler(int sig) {
    printf("\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;    
	int i;          
	int status;//stores child process status

	// register signal handler for SIGINT
    signal(SIGINT, sigint_handler);

	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r");
		if(fp < 0) {
			printf("File doesn't exists.");
			return -1;
		}
	}

	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}
			line[strlen(line) - 1] = '\0';
		} else { // interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}
		//printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		int is_background = 0;
        int is_sequence = 0;
        int is_parallel = 0;

        for (i = 0; tokens[i]!=NULL; i++) {
            if (strcmp(tokens[i], "&") == 0) {
                is_background = 1;
                tokens[i] = NULL;
                break;
            } else if (strcmp(tokens[i], "&&") == 0) {
                is_sequence = 1;
                tokens[i] = NULL;
                break;
            } else if (strcmp(tokens[i], "&&&") == 0) {
                is_parallel = 1;
                tokens[i] = NULL;
                break;
            }
        }

		for(i=0;tokens[i]!=NULL;i++){
			printf("found token %s (remove this debug output later)\n", tokens[i]);
		}
		printf("%d",is_background);

		if (strcmp(tokens[0],"cd")==0){
			if (tokens[1]==NULL){
				chdir(getenv("HOME"));
			}
			else {
				if (chdir(tokens[1]) != 0) {
                    printf("Shell: Incorrect command\n");
                }
			}
			continue;
		}
		
		if(is_background){
			pid_t pid = fork();// fork a child process
            if (pid == 0) {
                if (execvp(tokens[0], tokens)<0){
					printf("Shell: Incorrect command\n");
					exit(EXIT_FAILURE); 
				};
            } else if (pid < 0) {
                perror("fork");
            }
		}else if(is_parallel){
			int j = 0;
			int num_commands = 0;
			while (tokens[j] != NULL) {
				num_commands++;
				j++;
			}
			pid_t pids[num_commands];
			for (int k = 0; k < num_commands; k++) {
				pid_t pid = fork();
				if (pid == 0) {
					execvp(tokens[k], &tokens[k]);
					perror("exec");
					exit(1);
				} else if (pid < 0) {
					perror("fork");
					exit(1);
				} else {
					pids[k] = pid;
				}
			}
			for (int k = 0; k < num_commands; k++) {
				waitpid(pids[k], &status, 0);
				if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
						// command completed successfully
				} else {
					fprintf(stderr, "Shell: Incorrect command\n");
				}
			}

		}else if (is_sequence){

		}else{
			pid_t pid = fork();
			if (pid == 0) {
				// child process
				if (execvp(tokens[0], tokens)<0){
					printf("Shell: Incorrect command\n");
				};
				// execvp returns only if an error occurs
				exit(EXIT_FAILURE);
			} else if (pid < 0) {
				// error occurred
				perror("fork");
			}else {
				// parent process
				wait(&status);
			}
			
			if (WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV) {
				printf("Shell: Incorrect command\n");
			}
		}
        

       

		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
