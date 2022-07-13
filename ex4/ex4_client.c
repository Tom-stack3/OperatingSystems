#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/random.h>
#include <string.h>

#define TIMER_TIMEOUT_EXIT_MESSAGE "Client closed because no response was received from the server for 30 seconds\n"
#define TIMER_TIMEOUT 30
#define SRV_FILE_PATH "to_srv"
#define TO_CLIENT_PATH_FORMAT "to_client_%d"
#define BUFFER_SIZE 1024
#define ERROR_MESSAGE "ERROR_FROM_EX4\n"
#define error_exit()           \
    {                          \
        printf(ERROR_MESSAGE); \
        exit(-1);              \
    }

typedef struct task
{
    int server_pid;
    int n1;
    int n2;
    char given_operator;
} task;

char running;

// Timer timeout handler
void timer_timeout_handler(int sig)
{
    printf(TIMER_TIMEOUT_EXIT_MESSAGE);
    running = 0;
}

void result_from_server_handler(int sig)
{
    // Read the result from the server
    char result[BUFFER_SIZE];
    char client_file_name[BUFFER_SIZE];
    sprintf(client_file_name, TO_CLIENT_PATH_FORMAT, getpid());
    FILE *file = fopen(client_file_name, "r");
    if (file == NULL)
    {
        error_exit();
    }
    fgets(result, sizeof(result), file);
    fclose(file);
    // Print the result received
    printf("%s", result);
    // Delete the file
    if (remove(client_file_name) != 0)
    {
        error_exit();
    }
}

void send_task_to_server(task current)
{
    char i = 0;
    unsigned int seconds_to_sleep = 0;
    // While the file SRV_FILE_PATH exists
    while ((i < 10) && access(SRV_FILE_PATH, F_OK) != -1)
    {
        if (syscall(SYS_getrandom, &seconds_to_sleep, sizeof(int), 0) == -1)
        {
            error_exit();
        }
        seconds_to_sleep = seconds_to_sleep % 5 + 1;
        // Sleep
        sleep(seconds_to_sleep);
        ++i;
    }
    if (i == 10)
    {
        error_exit();
    }
    FILE *file = fopen(SRV_FILE_PATH, "w");
    if (file == NULL)
    {
        error_exit();
    }
    // Write the task to the file
    fprintf(file, "%d\n", getpid());
    fprintf(file, "%d\n", current.n1);
    fprintf(file, "%d\n", current.given_operator);
    fprintf(file, "%d\n", current.n2);
    fclose(file);
    // Send signal to the server
    kill(current.server_pid, SIGUSR1);
}

int main(int argc, char **argv)
{
    // Set signal handlers
    signal(SIGALRM, timer_timeout_handler);
    signal(SIGUSR2, result_from_server_handler);
    if (argc != 5)
    {
        error_exit();
    }
    // Parse command line arguments
    task current;
    current.server_pid = atoi(argv[1]);
    if (current.server_pid == 0)
    {
        error_exit();
    }
    current.given_operator = atoi(argv[3]);
    if (current.given_operator < 1 || current.given_operator > 4)
    {
        error_exit();
    }
    current.n1 = atoi(argv[2]);
    current.n2 = atoi(argv[4]);

    // Send the task to the server
    send_task_to_server(current);
    // Set an alarm
    alarm(TIMER_TIMEOUT);
    pause();
    if (!running)
    {
        // Cleanup

        // If file exists
        if (access(SRV_FILE_PATH, F_OK) != -1)
        {
            // Check if the file belongs to this client
            FILE *file = fopen(SRV_FILE_PATH, "r");
            if (file != NULL)
            {
                char my_pid[BUFFER_SIZE];
                sprintf(my_pid, "%d\n", getpid());
                char line[BUFFER_SIZE];
                fgets(line, sizeof(line), file);
                fclose(file);
                if (strcmp(line, my_pid) == 0)
                {
                    // Delete the file
                    if (remove(SRV_FILE_PATH) != 0)
                    {
                        error_exit();
                    }
                }
            }
        }
        char client_file_name[BUFFER_SIZE];
        sprintf(client_file_name, TO_CLIENT_PATH_FORMAT, getpid());
        // If file exists
        if (access(client_file_name, F_OK) != -1)
        {
            // Delete the file
            if (remove(client_file_name) != 0)
            {
                error_exit();
            }
        }
    }
}