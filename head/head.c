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
    const char* filename;
    int NUM_LINES = 10;
    int BUFF_SIZE = 4096;

    // checking for too many arguments
    if(argc > 4) {
        errno = E2BIG;
        char* error_str = strerror(errno);
        write(STDERR_FILENO, error_str, _strlen(error_str));
        return -1;
    }

    // handling arguments
    for (int i = 1; i < argc; i++) {
        // looking for -n flag
        if(*argv[i] == '-') {
            // flag found wasn't -n, error out
            if(*(++argv[i]) != 'n') {
                errno = EINVAL;
                char* error_str = strerror(errno);
                write(STDERR_FILENO, error_str, _strlen(error_str));
                return -1;
            }
            // -n found, set number of lines
            NUM_LINES = _atoi(argv[i + 1]);
            i++;
        }

        // arg is assumed to be a filename, store it
        else {
            filename = argv[i];
            printf("Filename: %s\n", filename);
        }
    }

    printf("Number of lines: %d\n", NUM_LINES);
}