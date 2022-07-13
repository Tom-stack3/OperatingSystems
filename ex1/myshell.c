#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CWD_LENGTH 4096
#define MAX_COMMAND_LEGTH 150
#define HISTORY_LENGTH 100
#define print_prompt() ({printf("$ "); fflush(stdout); })

char history[HISTORY_LENGTH][MAX_COMMAND_LEGTH];
int history_index = 0;
char history_full = 0;

// Add a command to the history
void add_to_history(long pid, char *command)
{
    char temp[MAX_COMMAND_LEGTH];
    sprintf(temp, "%ld %s", pid, command);
    if (history_index == HISTORY_LENGTH)
    {
        history_index = 0;
        history_full = 1;
    }
    strcpy(history[history_index], temp);
    history_index++;
}

// Print the history of commands
void print_history()
{
    int i;
    // If the history is full
    if (history_full)
    {
        for (i = history_index; i < HISTORY_LENGTH; i++)
        {
            printf("%s\n", history[i]);
        }
        for (i = 0; i < history_index; i++)
        {
            printf("%s\n", history[i]);
        }
    }
    else
    {
        for (i = 0; i < history_index; i++)
        {
            printf("%s\n", history[i]);
        }
    }
}

// Implementation of the history command
void history_command(char *command)
{
    // Add full command to history (including ignored arguments)
    add_to_history(getpid(), command);
    print_history();
}

// Implementation of the cd command
void cd_command(char *command)
{
    // Add full command to history (including ignored arguments)
    char temp[MAX_COMMAND_LEGTH];
    sprintf(temp, "cd %s", command);
    add_to_history(getpid(), temp);
    // Get the wanted path
    char *path = strtok(command, " ");
    if (chdir(path) == -1)
    {
        perror("chdir failed");
    }
}

void handle_other_commands(char *command)
{
    // Parse the command and prepare the args array
    char temp[MAX_COMMAND_LEGTH];
    strcpy(temp, command);
    char *args[MAX_COMMAND_LEGTH];
    int i = 0;
    char *token;
    token = strtok(temp, " ");
    while (token != NULL)
    {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = 0;

    // Fork
    pid_t pid = fork();
    if (pid == 0)
    {
        // In the child process
        if (execvp(args[0], args) == -1)
        {
            perror("execvp failed");
            exit(1);
        }
    }
    else if (pid < 0)
    {
        // Fork failed
        perror("fork failed");
    }
    else
    {
        // In the parent process

        // Add the full command to the history
        add_to_history(pid, command);
        // Wait for the child process
        wait(NULL);
    }
}

int main(int argc, char *argv[])
{
    // Add arguments to PATH
    char *path = getenv("PATH");
    int i;
    for (i = 1; i < argc; i++)
    {
        // Add arg to PATH
        strcat(path, ":");
        strcat(path, argv[i]);
    }

    char buffer[MAX_COMMAND_LEGTH];
    char *command_start;
    while (1)
    {
        print_prompt();
        // Read command from user
        // Wanted to use fgets(buffer, MAX_COMMAND_LEGTH, stdin); but was sadly forced to use scanf instead :(
        
        scanf(" %[^\n]", buffer);
        // If the user enters a new line, continue
        if (buffer[0] == '\n')
        {
            continue;
        }
        // Get to the start of the command
        command_start = buffer;
        while (*command_start == ' ')
        {
            command_start++;
        }

        // If user enters exit, exit
        if (strcmp(command_start, "exit") == 0 || strncmp(command_start, "exit ", 5) == 0)
        {
            exit(0);
        }
        // If the user enters the history command
        if (strcmp(command_start, "history") == 0 || strncmp(command_start, "history ", 8) == 0)
        {
            history_command(command_start);
            continue;
        }
        // If the user enters the cd command
        if (strncmp(command_start, "cd ", 3) == 0)
        {
            cd_command(command_start + 3);
            continue;
        }
        // Else, the user entered a command that is not a built in command
        handle_other_commands(command_start);
    }

    return 0;
}