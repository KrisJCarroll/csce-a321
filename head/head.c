#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>


int _strlen(const char *str) {
    if (*str)
        return 1 + _strlen(++str);
    return 0;
}

int _atoi(const char *str) {
    int res = 0;
    while (*str) {
        res = res * 10 + (*str) - '0';
        ++str;
    }
    return res;
}

int main(int argc, const char *argv[]) {
    int NUM_LINES = 10;
    int BUFF_SIZE = 4096;

    if(argc > 4) {
        errno = E2BIG;
        char* error_str = strerror(errno);
        write(STDERR_FILENO, error_str, _strlen(error_str));
        return -1;
    }
    for (int i = 1; i < argc; i++) {
        if(*argv[i] == '-') {
            if(*(++argv[i]) != 'n') {
                errno = EINVAL;
                char* error_str = strerror(errno);
                write(STDERR_FILENO, error_str, _strlen(error_str));
                return -1;
            }
            NUM_LINES = _atoi(argv[i + 1]);
            i++;
        }
        else {
            const char* filename = argv[i];
            printf("Filename: %s\n", filename);
        }
    }

    printf("Number of lines: %d\n", NUM_LINES);
}