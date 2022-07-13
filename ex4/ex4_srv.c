#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define DIVIDE_BY_ZERO "CANNOT_DIVIDE_BY_ZERO\n"
#define TIMER_TIMEOUT_EXIT_MESSAGE "The server was closed because no service request was received for the last 60 seconds\n"
#define TIMER_TIMEOUT 60
#define SRV_FILE_PATH "to_srv"
#define TO_CLIENT_PATH_FORMAT "to_client_%d"
#define BUFFER_SIZE 1024
#define ERROR_MESSAGE "ERROR_FROM_EX4\n"
#define error_exit()           \
    {                          \
        printf(ERROR_MESSAGE); \
        exit(-1);              \
    }

char running;

// Timer timeout handler
void timer_timeout_handler(int sig)
{
    printf(TIMER_TIMEOUT_EXIT_MESSAGE);
    running = 0;
}

// Client handler
void client_handler(int sig)
{
    signal(SIGUSR1, client_handler);

    // Fork a child process to handle client
    pid_t pid;
    if ((pid = fork()) == -1)
    {
        error_exit();
    }
    else if (pid == 0)
    {
        signal(SIGALRM, SIG_IGN);
        // Child process
        FILE *file = fopen(SRV_FILE_PATH, "r");
        if (file == NULL)
        {
            error_exit();
        }
        char line[BUFFER_SIZE];
        int client_pid, n1, n2;
        char given_operator;
        char i = 0;

        // Read and parse line by line
        while (i < 4 && fgets(line, sizeof(line), file))
        {
            switch (i)
            {
            case 0:
                // Line contains the client pid
                client_pid = atoi(line);
                if (client_pid == 0)
                {
                    error_exit();
                }
                break;
            case 1:
                // Line contains the first operand
                n1 = atoi(line);
                break;
            case 2:
                // Line contains the operator
                given_operator = atoi(line);
                if (given_operator < 1 || given_operator > 4)
                {
                    error_exit();
                }
                break;
            case 3:
                // Line contains the second operand
                n2 = atoi(line);
                break;
            }
            ++i;
        }
        fclose(file);
        // Delete the file
        if (remove(SRV_FILE_PATH) != 0)
        {
            error_exit();
        }
        // Calculate the expression
        char result[BUFFER_SIZE];
        switch (given_operator)
        {
        case 1:
            // +
            sprintf(result, "%d\n", n1 + n2);
            break;
        case 2:
            // -
            sprintf(result, "%d\n", n1 - n2);
            break;
        case 3:
            // *
            sprintf(result, "%d\n", n1 * n2);
            break;
        case 4:
            // /
            if (n2 == 0)
            {
                strcpy(result, DIVIDE_BY_ZERO);
                break;
            }
            sprintf(result, "%d\n", n1 / n2);
            break;
        }
        // Write the result to file
        char client_file_name[BUFFER_SIZE];
        sprintf(client_file_name, TO_CLIENT_PATH_FORMAT, client_pid);
        file = fopen(client_file_name, "w");
        if (file == NULL)
        {
            error_exit();
        }
        fprintf(file, "%s", result);
        fclose(file);
        // Send signal to client that the result file is ready
        kill(client_pid, SIGUSR2);
        exit(0);
    }
}

int main()
{
    running = 1;
    // Set signal handlers
    signal(SIGALRM, timer_timeout_handler);
    signal(SIGUSR1, client_handler);

    // Set an alarm for TIMER_TIMEOUT seconds
    alarm(TIMER_TIMEOUT);
    while (running)
    {
        // Wait for signals
        pause();
        alarm(TIMER_TIMEOUT);
    }

    // Wait for all zombie processes
    while (wait(NULL) != -1);
}