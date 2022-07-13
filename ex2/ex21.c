#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#define ERROR -1
#define IDENTICAL 1
#define DIFFERENT 2
#define SIMILAR 3
#define BUFFER_SIZE 1024
#define cleanup(fd1, fd2) \
    close(fd1);           \
    close(fd2);

// Check if the files are identical
char are_identical(int fd1, int fd2)
{
    char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];
    int read_bytes1, read_bytes2, i;
    // Do while able to read from both files
    do
    {
        read_bytes1 = read(fd1, buffer1, BUFFER_SIZE);
        read_bytes2 = read(fd2, buffer2, BUFFER_SIZE);

        if (read_bytes1 == -1 || read_bytes2 == -1)
        {
            write(2, "Error in: read\n", 15);
            cleanup(fd1, fd2);
            exit(ERROR);
        }

        if (read_bytes1 != read_bytes2)
        {
            return DIFFERENT;
        }
        for (i = 0; i < read_bytes1; i++)
        {
            if (buffer1[i] != buffer2[i])
            {
                return DIFFERENT;
            }
        }

    } while (read_bytes1 > 0 && read_bytes2 > 0);

    // Both files are identical
    return IDENTICAL;
}

// Check if the files are similar
char are_similar(int fd1, int fd2)
{
    char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];
    char clean_buffer1[BUFFER_SIZE], clean_buffer2[BUFFER_SIZE];
    int read_bytes1, read_bytes2, i;
    int clean_bytes_read1 = 0;
    int clean_bytes_read2 = 0;
    // Do while not done reading from both files
    do
    {
        read_bytes1 = read(fd1, buffer1, BUFFER_SIZE - clean_bytes_read1);
        read_bytes2 = read(fd2, buffer2, BUFFER_SIZE - clean_bytes_read2);

        if (read_bytes1 == -1 || read_bytes2 == -1)
        {
            write(2, "Error in: read\n", 15);
            cleanup(fd1, fd2);
            exit(ERROR);
        }

        for (i = 0; i < read_bytes1; i++)
        {
            if (buffer1[i] != '\n' && buffer1[i] != '\t' && buffer1[i] != ' ')
            {
                // Lowercase the character and add it to the clean buffer
                clean_buffer1[clean_bytes_read1] = tolower(buffer1[i]);
                clean_bytes_read1++;
            }
        }
        for (i = 0; i < read_bytes2; i++)
        {
            if (buffer2[i] != '\n' && buffer2[i] != '\t' && buffer2[i] != ' ')
            {
                // Lowercase the character and add it to the clean buffer
                clean_buffer2[clean_bytes_read2] = tolower(buffer2[i]);
                clean_bytes_read2++;
            }
        }
        // If both clean buffers are full
        if (clean_bytes_read1 == BUFFER_SIZE && clean_bytes_read2 == BUFFER_SIZE)
        {
            for (i = 0; i < BUFFER_SIZE; i++)
            {
                if (clean_buffer1[i] != clean_buffer2[i])
                {
                    return DIFFERENT;
                }
            }
            // The buffers are identical
            // Clear the buffers and continue reading the files
            clean_bytes_read1 = 0;
            clean_bytes_read2 = 0;
            if (memset(clean_buffer1, 0, BUFFER_SIZE) == NULL || memset(clean_buffer2, 0, BUFFER_SIZE) == NULL)
            {
                write(2, "Error in: memset\n", 17);
                cleanup(fd1, fd2);
                exit(ERROR);
            }
        }
    } while (!(read_bytes1 <= 0 && read_bytes2 <= 0));

    // Check if the clean buffers are identical
    if (clean_bytes_read1 == clean_bytes_read2)
    {
        for (i = 0; i < clean_bytes_read1; i++)
        {
            if (clean_buffer1[i] != clean_buffer2[i])
            {
                return DIFFERENT;
            }
        }
    }
    else
    {
        return DIFFERENT;
    }

    // Both files are similar
    return SIMILAR;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        write(2, "Error: wrong number of parameters\n", 34);
        exit(ERROR);
    }
    // Open the files
    int fd1 = open(argv[1], O_RDONLY);
    int fd2 = open(argv[2], O_RDONLY);

    // Check if the files opened successfully
    if (fd1 == -1 || fd2 == -1)
    {
        write(2, "Error in: open\n", 15);
        // Try to close the files
        cleanup(fd1, fd2);
        // Exit with error
        exit(ERROR);
    }
    // Check if the files are identical
    if (are_identical(fd1, fd2) == IDENTICAL)
    {
        // Close the files before exiting
        cleanup(fd1, fd2);
        return IDENTICAL;
    }
    // lseek to the beginning of the files
    if (lseek(fd1, 0, SEEK_SET) == -1 || lseek(fd2, 0, SEEK_SET) == -1)
    {
        write(2, "Error in: lseek\n", 15);
        // Try to close the files
        cleanup(fd1, fd2);
        // Exit with error
        exit(ERROR);
    }

    // Check if the files are similar
    if (are_similar(fd1, fd2) == SIMILAR)
    {
        // Close the files before exiting
        cleanup(fd1, fd2);
        return SIMILAR;
    }

    // Close the files before exiting
    cleanup(fd1, fd2);
    return DIFFERENT;
}
