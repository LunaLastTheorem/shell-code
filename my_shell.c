#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>

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

void background_process()
{
}

void sequence_process()
{
}

void parallel_in_foreground()
{
}

int main(int argc, char *argv[])
{
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
		for (int i = 0; i < MAX_NUM_TOKENS; i++)
		{
			commands_array[i] = (char **)malloc(MAX_TOKEN_SIZE * sizeof(char *));
			for (int j = 0; j < MAX_TOKEN_SIZE; j++)
			{
				commands_array[i][j] = (char *)malloc(20 * sizeof(char));
			}
		}
		int commands_pointer = 0;

		for (i = 0; tokens[i] != NULL; i++)
		{
			printf("found token %s (remove this debug output later)\n", tokens[i]);
			strcpy(commands_array[commands_pointer][i], tokens[i]);

			if (strcmp(tokens[i], "&") == 0)
			{
				printf("%s", "background\n"); // TODO remove
				commands_pointer++;
				background = 1;
			}
			else if (strcmp(tokens[i], "&&") == 0)
			{
				printf("%s", "sequence\n"); // TODO remove
				commands_pointer++;
				sequence = 1;
			}
			else if (strcmp(tokens[i], "&&&") == 0)
			{
				printf("%s", "foreground\n"); // TODO remove
				commands_pointer++;
				foreground = 1;
			}
		}

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 20; j++) {
				printf("Token [%d][%d]: %s\n", i, j, commands_array[i][j]);
			}
		}

		if (background == 1)
		{
		}
		else if (sequence == 1)
		{
		}
		else if (foreground == 1)
		{
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
				if (strcmp(tokens[0], "cd") != 0)
				{
					if (execvp(tokens[0], tokens) == -1)
					{
						printf("Commands \'%s\' not found, or an error has occured during execution\n", tokens[0]);
						exit(1);
					}
				}
				else
				{
					if (chdir(tokens[1]) != 0)
					{
						fprintf(stderr, "Invalid directory\n");
					}
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
		free(tokens);
	}
	return 0;
}