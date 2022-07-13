#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#define ERROR -1
#define IDENTICAL 1
#define DIFFERENT 2
#define SIMILAR 3
#define BUFFER_SIZE 1024

#define NO_C_FILE_GRADE 0
#define COMPILATION_ERROR_GRADE 1
#define TIMEOUT_GRADE 2
#define WRONG_GRADE 3
#define SIMILAR_GRADE 4
#define EXCELLENT_GRADE 5

#define EXECUTABLE_FILE_NAME "temp.out"
#define OUTPUT_FILE_NAME "temp_output.txt"

// Create results.csv and errors.txt files
void create_initial_files(int *results_fd, int *errors_fd)
{
    int temp_fd;
    // Create results.csv file
    temp_fd = open("results.csv", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1)
    {
        write(2, "Error in: open\n", 15);
        exit(ERROR);
    }
    *results_fd = temp_fd;
    // Create errors.txt file
    temp_fd = open("errors.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1)
    {
        write(2, "Error in: open\n", 15);
        exit(ERROR);
    }
    *errors_fd = temp_fd;
}

// Grade a student
void grade_student(int results_fd, char *student_dir_name, char student_grade_category)
{
    char *grades[] = {"0", "10", "20", "50", "75", "100"};
    char *grades_name[] = {"NO_C_FILE", "COMPILATION_ERROR", "TIMEOUT", "WRONG", "SIMILAR", "EXCELLENT"};
    char grade_row[BUFFER_SIZE];

    if (memset(grade_row, 0, BUFFER_SIZE) == NULL)
    {
        write(2, "Error in: memset\n", 17);
        return;
    }
    strcpy(grade_row, student_dir_name);
    strcat(grade_row, ",");
    strcat(grade_row, grades[student_grade_category]);
    strcat(grade_row, ",");
    strcat(grade_row, grades_name[student_grade_category]);
    strcat(grade_row, "\n");
    // Write grade to results.csv
    if (write(results_fd, grade_row, strlen(grade_row)) == -1)
    {
        return;
    }
}

// Run .out and check output
char run_and_check_output(int input_fd, char *comp_full_path, char *expected_out_full_path, char *executable_path)
{
    int return_value = ERROR;
    // Create temp_output.txt file
    int temp_output_fd = open(OUTPUT_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_output_fd == -1)
    {
        write(2, "Error in: open\n", 15);
        return ERROR;
    }
    // lseek to the beginning of the input file
    if (lseek(input_fd, 0, SEEK_SET) == -1)
    {
        write(2, "Error in: lseek\n", 16);
        return ERROR;
    }
    // Fork a child process and run the executable file with the given input file
    pid_t pid;
    if ((pid = fork()) == -1)
    {
        write(2, "Error in: fork\n", 15);
        return ERROR;
    }
    else if (pid == 0)
    {
        // Child process
        // Dup stdout to temp_output.txt and input to stdin
        if (dup2(temp_output_fd, 1) == -1 || dup2(input_fd, 0) == -1)
        {
            write(2, "Error in: dup2\n", 15);
            return ERROR;
        }
        // Run the executable file
        if (execl(executable_path, executable_path, NULL) == -1)
        {
            write(2, "Error in: execl1\n", 16);
            return ERROR;
        }
    }
    // Wait for the child process to finish
    wait(NULL);
    // Fork a child process and run comp.out with the expected output file and the output file
    if ((pid = fork()) == -1)
    {
        write(2, "Error in: fork\n", 15);
        return ERROR;
    }
    else if (pid == 0)
    {
        // Child process
        if (execl(comp_full_path, comp_full_path, expected_out_full_path, OUTPUT_FILE_NAME, NULL) == -1)
        {
            write(2, "Error in: execl2\n", 16);
            return ERROR;
        }
    }
    // Wait for the child process to finish and check its exit status
    int wstatus;
    if (waitpid(pid, &wstatus, 0) == -1)
    {
        write(2, "Error in: wait\n", 15);
        return ERROR;
    }
    // If the file exited successfully
    if (WIFEXITED(wstatus))
    {
        return_value = WEXITSTATUS(wstatus);
        switch (return_value)
        {
        case DIFFERENT:
            return_value = WRONG_GRADE;
            break;
        case SIMILAR:
            return_value = SIMILAR_GRADE;
            break;
        case IDENTICAL:
            return_value = EXCELLENT_GRADE;
            break;
        default:
            break;
        }
    }
    // Remove temp_output.txt file
    if (remove(OUTPUT_FILE_NAME) == -1)
    {
        write(2, "Error in: remove\n", 17);
        return ERROR;
    }
    close(temp_output_fd);
    // Remove temp.out file
    if (remove("temp.out") == -1)
    {
        write(2, "Error in: remove\n", 17);
        return ERROR;
    }
    return return_value;
}

// Compile a .c file
char compile_file(char *file_path, int errors_fd)
{
    // Fork a child process and compile the file using execvp
    pid_t pid;
    if ((pid = fork()) == -1)
    {
        write(2, "Error in: fork\n", 15);
        return ERROR;
    }
    if (pid == 0)
    {
        // Child process

        // Redirect stdout and stderr to errors.txt
        if (dup2(errors_fd, 1) == -1 || dup2(errors_fd, 2) == -1)
        {
            write(2, "Error in: dup2\n", 15);
            exit(ERROR);
        }
        // Exec gcc
        char *args[] = {"gcc", "-w", file_path, "-o", EXECUTABLE_FILE_NAME, NULL};
        if (execvp(args[0], args) == -1)
        {
            write(2, "Error in: execvp\n", 17);
            return ERROR;
        }
    }
    // Wait for the child process to finish and check its exit status
    int wstatus;
    if (wait(&wstatus) == -1)
    {
        write(2, "Error in: wait\n", 15);
        return ERROR;
    }
    // Check if the file compiled successfully and can access the executable file
    if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) == 0 && access(EXECUTABLE_FILE_NAME, F_OK) != -1)
    {
        // If the file compiled successfully, return 1
        return 1;
    }
    return 0;
}

void handle_student(int results_fd, int errors_fd, int input_fd, char *student_dir_path, char *comp_out_full_path, char *expected_out_full_path, char *student_dir_name)
{
    char system_error = 0;
    // Grade conditions
    char found_file = 0;
    char student_grade_type = EXCELLENT_GRADE;
    char c_file_path[BUFFER_SIZE];
    // Open student's directory
    DIR *student_dir;
    struct dirent *file;

    if ((student_dir = opendir(student_dir_path)) == NULL)
    {
        return;
    }
    // Loop through the student's directory
    while ((file = readdir(student_dir)) != NULL)
    {
        // If current file is not a file
        if (!(file->d_type == DT_REG))
        {
            continue;
        }
        // If current file is "." or ".."
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
        {
            continue;
        }
        // If current file is a .c file
        if (strcmp(file->d_name + strlen(file->d_name) - 2, ".c") == 0)
        {
            found_file = 1;
            // Copy the file path to c_file_path
            strcpy(c_file_path, student_dir_path);
            strcat(c_file_path, "/");
            strcat(c_file_path, file->d_name);
            // Switch case
            switch (compile_file(c_file_path, errors_fd))
            {
            // If there was a system error
            case ERROR:
                system_error = 1;
                break;
            // If the file didn't compile successfully
            case 0:
                student_grade_type = COMPILATION_ERROR_GRADE;
                break;
            // If the file compiled successfully
            case 1:
                // Check output of the compiled .c file
                student_grade_type = run_and_check_output(input_fd, comp_out_full_path, expected_out_full_path, EXECUTABLE_FILE_NAME);
                if (student_grade_type == ERROR)
                {
                    system_error = 1;
                }
                break;
            }
            break;
        }
    }
    closedir(student_dir);

    // If there was a system error
    if (system_error)
    {
        // Return and continue to the next student
        return;
    }
    // If the student didn't have a .c file
    if (!found_file)
    {
        student_grade_type = NO_C_FILE_GRADE;
    }
    // Grade the student
    grade_student(results_fd, student_dir_name, student_grade_type);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        write(2, "Error: wrong number of parameters\n", 34);
        exit(ERROR);
    }
    // Open config file from argv[1]
    int conf_fd = open(argv[1], O_RDONLY);

    if (conf_fd == -1)
    {
        write(2, "Error in: open\n", 15);
        // Try to close config file
        close(conf_fd);
        // Exit with error
        exit(ERROR);
    }

    // Read first line from config file
    char buffer[BUFFER_SIZE];
    int read_bytes = read(conf_fd, buffer, BUFFER_SIZE);
    if (read_bytes == -1)
    {
        write(2, "Error in: read\n", 15);
        close(conf_fd);
        exit(ERROR);
    }
    // Get the first three lines from config file
    char *students_path = strtok(buffer, "\n");
    char *input_path = strtok(NULL, "\n");
    char *output_path = strtok(NULL, "\n");

    close(conf_fd);

    int input_fd = open(input_path, O_RDONLY);
    if (input_fd == -1)
    {
        write(2, "Error in: open\n", 15);
        // Try to close config file
        close(input_fd);
        // Exit with error
        exit(ERROR);
    }

    // Save full path to comp.out
    char comp_out_full_path[BUFFER_SIZE];
    if (getcwd(comp_out_full_path, BUFFER_SIZE) == NULL)
    {
        write(2, "Error in: getcwd\n", 17);
        exit(ERROR);
    }
    strcat(comp_out_full_path, "/comp.out");
    // Save full path to the expected output file
    char expected_out_full_path[BUFFER_SIZE];
    if (getcwd(expected_out_full_path, BUFFER_SIZE) == NULL)
    {
        write(2, "Error in: getcwd\n", 17);
        exit(ERROR);
    }
    strcat(expected_out_full_path, "/");
    strcat(expected_out_full_path, output_path);

    // Create results.csv and errors.txt files
    int results_fd, errors_fd;
    create_initial_files(&results_fd, &errors_fd);

    // Open students directory
    DIR *students_dir;
    struct dirent *single_student_dir;
    if ((students_dir = opendir(students_path)) == NULL)
    {
        write(2, "Not a valid directory\n", 22);
        exit(ERROR);
    }
    char student_dir_path[BUFFER_SIZE];
    // Loop through directory
    while ((single_student_dir = readdir(students_dir)) != NULL)
    {
        // If current file is not a directory
        if (!(single_student_dir->d_type == DT_DIR))
        {
            continue;
        }
        // If current file is "." or ".."
        if (strcmp(single_student_dir->d_name, ".") == 0 || strcmp(single_student_dir->d_name, "..") == 0)
        {
            continue;
        }
        strcpy(student_dir_path, students_path);
        strcat(student_dir_path, "/");
        strcat(student_dir_path, single_student_dir->d_name);
        // Handle student
        handle_student(results_fd, errors_fd, input_fd, student_dir_path, comp_out_full_path, expected_out_full_path, single_student_dir->d_name);
    }
    // Cleanup
    closedir(students_dir);
    close(input_fd);
    close(results_fd);
    close(errors_fd);
    return 0;
}