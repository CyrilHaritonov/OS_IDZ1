#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(void) {
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