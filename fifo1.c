
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

#define FIFO_FILE "/tmp/my_fifo"
#define FILE_NAME "hello.txt"

void server(void);
void client(void);

int main(void) {
    pid_t pid;

    if (mkfifo(FIFO_FILE, 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(EXIT_FAILURE);
        }
    }

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        unlink(FIFO_FILE);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        client();
        exit(EXIT_SUCCESS);
    } else {
        sleep(1);
        server();
        wait(NULL);
        unlink(FIFO_FILE);
    }

    return 0;
}

void server(void) {
    int fifo_fd, file_fd;
    char buffer[1024];
    ssize_t bytes_read, bytes_written;

    file_fd = open(FILE_NAME, O_RDONLY);
    if (file_fd == -1) {
        perror("Error opening hello.txt");
        exit(EXIT_FAILURE);
    }

    fifo_fd = open(FIFO_FILE, O_WRONLY);
    if (fifo_fd == -1) {
        perror("Error opening FIFO for writing");
        close(file_fd);
        exit(EXIT_FAILURE);
    }

    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(fifo_fd, buffer, bytes_read);
        if (bytes_written == -1) {
            perror("Error writing to FIFO");
            close(file_fd);
            close(fifo_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read == -1) {
        perror("Error reading file");
    }

    close(file_fd);
    close(fifo_fd);
}

void client(void) {
    int fifo_fd;
    char buffer[1024];
    ssize_t bytes_read;

    fifo_fd = open(FIFO_FILE, O_RDONLY);
    if (fifo_fd == -1) {
        perror("Error opening FIFO for reading");
        exit(EXIT_FAILURE);
    }

    while ((bytes_read = read(fifo_fd, buffer, sizeof(buffer))) > 0) {
        if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
            perror("Error writing to stdout");
            close(fifo_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read == -1) {
        perror("Error reading from FIFO");
    }

    close(fifo_fd);
}