#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments\n");
        return 0;
    }
    mkfifo("fifo_1", 0666);
    mkfifo("fifo_2", 0666);
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        int fd_input = open(argv[1], O_RDONLY);
        if (fd_input == -1) {
            printf("Failed to open input file\n");
            return 0;
        }
        char first_string[5000], second_string[5000], common_buffer[5000];
        ssize_t bytes_read = read(fd_input, common_buffer, sizeof(common_buffer));
        if (bytes_read == -1) {
            printf("Failed reading file\n");
            return 0;
        }
        int didFindNewLine = 0, j = 0, k = 0;
        for (int i = 0; i < strlen(common_buffer); ++i) {
            if (common_buffer[i] == '\n') {
                didFindNewLine = 1;
                continue;
            }
            if (didFindNewLine) {
                second_string[k++] = common_buffer[i];
            } else {
                first_string[j++] = common_buffer[i];
            }
        }
        close(fd_input);
        char concatenated[10001];
        sprintf(concatenated, "%s\n%s", first_string, second_string);
        int fd_1_write = open("fifo_1", O_WRONLY);
        ssize_t check = write(fd_1_write, concatenated, strlen(concatenated) + 1);
        close(fd_1_write);
        if (check < 0) {
            printf("Failed to write string to fifo 1\n");
            return 0;
        }
        char buffer[257];
        int fd_2_read = open("fifo_2", O_RDONLY);
        check = read(fd_2_read, buffer, sizeof(buffer));
        close(fd_2_read);
        if (check < 0) {
            printf("Failed to read from second fifo\n");
            return 0;
        }
        int fd_output = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd_output == -1) {
            printf("Failed to open output file\n");
            return 0;
        }
        char output_buffer[10001];
        int offset = 0;
        for (int j = 0; j < 128; ++j) {
            if (buffer[j] == '1' && buffer[j + 128] == '0') {
                sprintf(output_buffer + offset++, "%c", j);
            }
        }
        sprintf(output_buffer + offset++, "\n");
        for (int j = 0; j < 128; ++j) {
            if (buffer[j] == '0' && buffer[j + 128] == '1') {
                sprintf(output_buffer + offset++, "%c", j);
            }
        }
        write(fd_output, output_buffer, strlen(output_buffer));
        close(fd_output);
        return 0;
    } else if (pid == -1) {
        printf("Failed to create first process\n");
        return 0;
    }
    pid = fork();
    if (pid == -1) {
        printf("Failed to create second process\n");
        return 0;
    } else if (pid == 0) {
        char buffer[10001];
        int fd_1_read = open("fifo_1", O_RDONLY);
        ssize_t check = read(fd_1_read, buffer, sizeof(buffer));
        close(fd_1_read);
        if (check < 0) {
            printf("Failed to access the fifo 1 from child process\n");
            return 0;
        }
        char first_str_helper[129], second_str_helper[129];
        for (int i = 0; i < 128; ++i) {
            first_str_helper[i] = '0';
            second_str_helper[i] = '0';
        }
        int did_find_delimiter = 0;
        int i = 0;
        while (!did_find_delimiter || buffer[i] != '\0') {
            if (buffer[i] == '\n') {
                did_find_delimiter = 1;
                i++;
                continue;
            }
            if (did_find_delimiter) {
                second_str_helper[buffer[i]] = '1';
            } else {
                first_str_helper[buffer[i]] = '1';
            }
            i++;
        }
        char helpers[257];
        sprintf(helpers, "%s%s", first_str_helper, second_str_helper);
        int fd_2_write = open("fifo_2", O_WRONLY);
        check = write(fd_2_write, helpers, strlen(helpers) + 1);
        close(fd_2_write);
        if (check < 0) {
            printf("Failed to write string to fifo 2\n");
            return 0;
        }
    }
    wait(NULL);
    unlink("fifo_1");
    unlink("fifo_2");
    return 0;
}