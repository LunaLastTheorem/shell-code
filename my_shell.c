#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>


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

void background_process(char ***commands) {
    int i = 0;
    while (commands[i] != NULL) {
		if (strcmp(commands[i][0], "cd") == 0)
		{
			if (commands[i][1] == NULL || chdir(commands[i][1]) != 0)
			{
				perror("Invalid directory\n");
			}
			i++;
			continue;
		}

        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
			if (execvp(commands[i][0], commands[i]) == -1) {
				printf("Command '%s' not found or failed to execute\n", commands[i][0]);
                exit(1);
            }
        }else{
			printf("Shell: Started background process with PID[%d]\n", pid);
		}
        i++;
    }
}

void sequence_process(char ***commands)
{
	int i = 0;
	while (commands[i] != NULL)
	{
		if (strcmp(commands[i][0], "cd") == 0)
		{
			if (commands[i][1] == NULL || chdir(commands[i][1]) != 0)
			{
				perror("Invalid directory\n");
			}
			i++;
			continue;
		}

		pid_t pid = fork();

		if (pid < 0)
		{
			perror("Fork failed");
			exit(1);
		}
		else if (pid == 0)
		{
			if (execvp(commands[i][0], commands[i]) == -1)
			{
				printf("Commands \'%s\' not found, or an error has occured during execution\n", commands[i][0]);
				exit(1);
			}
		}
		else
		{
			waitpid(pid, 0, 0);
		}
		i++;
	}
}

void parallel_in_foreground(char ***commands)
{
	int i = 0;
	pid_t pids[MAX_NUM_TOKENS];
	int num_commands = 0;

	while (commands[i] != NULL)
	{
		if (strcmp(commands[i][0], "cd") == 0)
		{
			if (commands[i][1] == NULL || chdir(commands[i][1]) != 0)
			{
				fprintf(stderr, "Invalid directory\n");
			}
			i++;
			continue;
		}

		pid_t pid = fork();

		if (pid < 0)
		{
			perror("Fork failed");
			exit(1);
		}
		else if (pid == 0)
		{
			if (execvp(commands[i][0], commands[i]) == -1)
			{
				printf("Commands \'%s\' not found, or an error has occured during execution\n", commands[i][0]);
				exit(1);
			}
		}
		else
		{
			pids[i] = pid;
			num_commands++;
		}

		i++;
	}

	for (int j = 0; j < num_commands; j++)
	{
		waitpid(pids[j], NULL, 0);
	}
}

void reap_background_processes(int sig) {
    int status;
    pid_t pid;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Shell: Background process %d finished\n", pid);
    }
}

int main(int argc, char *argv[])
{
	signal(SIGCHLD, reap_background_processes);

	char line[MAX_INPUT_SIZE];
	char **tokens;
	int i;

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
			reap_background_processes(0);
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}
		//		printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; // terminate with new line
		tokens = tokenize(line);

		// do whatever you want with the commands, here we just print them

		bool background = 0; // &
		bool sequence = 0;	 // &&
		bool foreground = 0; // &&&

		char ***commands_array = (char ***)malloc(MAX_NUM_TOKENS * sizeof(char **));
		int commands_pointer = 0;
		int token_pointer = 0;
		commands_array[commands_pointer] = (char **)malloc(MAX_TOKEN_SIZE * sizeof(char *));

		for (i = 0; tokens[i] != NULL; i++)
		{
			// printf("found token %s (remove this debug output later)\n", tokens[i]);

			if (strcmp(tokens[i], "&") == 0 || strcmp(tokens[i], "&&") == 0 || strcmp(tokens[i], "&&&") == 0)
			{
				commands_array[commands_pointer][token_pointer] = NULL;
				commands_pointer++;
				token_pointer = 0;
				commands_array[commands_pointer] = (char **)malloc(MAX_TOKEN_SIZE * sizeof(char *));

				if (strcmp(tokens[i], "&") == 0)
				{
					background = 1;
				}
				else if (strcmp(tokens[i], "&&") == 0)
				{
					sequence = 1;
				}
				else if (strcmp(tokens[i], "&&&") == 0)
				{
					foreground = 1;
				}

				continue;
			}

			commands_array[commands_pointer][token_pointer] = strdup(tokens[i]);
			token_pointer++;
		}

		if (background == 1)
		{
			background_process(commands_array);
		}
		else if (sequence == 1)
		{
			sequence_process(commands_array);
		}
		else if (foreground == 1)
		{
			parallel_in_foreground(commands_array);
		}
		else if (strcmp(tokens[0], "cd") == 0)
		{
			if (tokens[1] == NULL || chdir(tokens[1]) != 0)
			{
				fprintf(stderr, "Invalid directory\n");
			}
		}
		else
		{
			pid_t pid;

			pid = fork();

			if (pid < 0)
			{
				fprintf(stderr, "Fork Fail");
				return 1;
			}
			else if (pid == 0)
			{
				if (execvp(tokens[0], tokens) == -1)
				{
					printf("Commands \'%s\' not found, or an error has occured during execution\n", tokens[0]);
					exit(1);
				}
			}
			else
			{
				wait(NULL);
			}
		}

		// Freeing the allocated memory
		for (i = 0; tokens[i] != NULL; i++)
		{
			free(tokens[i]);
		}
		for (int i = 0; commands_array[i] != NULL; i++)
		{
			for (int j = 0; commands_array[i][j] != NULL; j++)
			{
				free(commands_array[i][j]);
			}
			free(commands_array[i]);
		}

		free(commands_array);
	}
	return 0;
}