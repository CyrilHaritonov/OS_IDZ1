#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments\n");
        return 0;
    }
    int fd[2];
    if (pipe(fd) == -1) {
        printf("Failed to create pipe\n");
        return 0;
    }
    pid_t pid;
    int status1;
    pid = fork();
    if (pid == 0) {
        FILE *input_file = fopen(argv[1], "r");
        if (input_file == NULL) {
            printf("Failed to open input file\n");
            return 0;
        }
        char first_string[5000], second_string[5000];
        fscanf(input_file, "%s %s", first_string, second_string);
        fclose(input_file);
        close(fd[0]);
        char concatenated[10001];
        sprintf(concatenated, "%s|%s", first_string, second_string);
        ssize_t check = write(fd[1], concatenated, strlen(concatenated) + 1);
        if (check < 0) {
            printf("Failed to write string to pipe\n");
            return 0;
        }
        return 0;
    } else if (pid == -1) {
        printf("Failed to create second process\n");
        return 0;
    }
    pid_t w_pid = wait(&status1);
    if (w_pid == -1) {
        printf("Failed to wait\n");
    }
    int status2;
    int fd2[2];
    if (pipe(fd2) == -1) {
        printf("Failed to create pipe\n");
        return 0;
    }
    pid = fork();
    if (pid == -1) {
        printf("Failed to create second process\n");
        return 0;
    } else if (pid == 0) {
        close(fd[1]);
        char buffer[10001];
        ssize_t check = read(fd[0], buffer, sizeof(buffer));
        if (check < 0) {
            printf("Failed to access the pipe from child process\n");
        }
        char first_str_helper[129], second_str_helper[129];
        for (int i = 0; i < 128; ++i) {
            first_str_helper[i] = '0';
            second_str_helper[i] = '0';
        }
        int did_find_delimiter = 0;
        int i = 0;
        while (!did_find_delimiter || buffer[i] != '\0') {
            if (buffer[i] == '|') {
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
        close(fd2[0]);
        char helpers[257];
        sprintf(helpers, "%s%s", first_str_helper, second_str_helper);
        check = write(fd2[1], helpers, strlen(helpers) + 1);
        if (check < 0) {
            printf("Failed to write string to pipe\n");
            return 0;
        }
        return 0;
    }
    w_pid = wait(&status2);
    if (w_pid == -1) {
        printf("Failed to wait\n");
    }
    pid = fork();
    if (pid == -1) {
        printf("Failed to create third process\n");
        return 0;
    } else if (pid == 0) {
        close(fd2[1]);
        char buffer[257];
        ssize_t check = read(fd2[0], buffer, sizeof (buffer));
        if (check < 0) {
            printf("Failed to read from second pipe\n");
            return 0;
        }
        FILE *output_file = fopen(argv[2], "w");
        if (output_file == NULL) {
            printf("Failed to open output file\n");
            return 0;
        }
        for (int j = 0; j < 128; ++j) {
            if (buffer[j] == '1' && buffer[j + 128] == '0') {
                fprintf(output_file, "%c", j);
            }
        }
        fprintf(output_file, "\n");
        for (int j = 0; j < 128; ++j) {
            if (buffer[j] == '0' && buffer[j + 128] == '1') {
                fprintf(output_file, "%c", j);
            }
        }
        fclose(output_file);
        return 0;
    }
    return 0;
}