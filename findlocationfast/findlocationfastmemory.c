#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

// recursive implementation of strlen()
int _strlen(const char *str) {
    if (*str)
        return 1 + _strlen(++str);
    return 0;
}

// iterative implementation of atoi()
int _atoi(const char *str) {
    int res = 0;
    while (*str) {
        int next_digit = (*str) - '0'; // get decimal integer value of ASCII character
        // not a number, error out
        if(next_digit < 0 || next_digit > 9) {
            errno = EINVAL;
            write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
            write(STDIN_FILENO, "\n", 1);
            exit(-1);
        }
        res = res * 10 + next_digit;
        ++str;
    }
    return res;
}

int _strcmp(const char* first, const char* second) {
    while ((*first) && (*second)) {
        if (*first < *second) return -1;
        if (*first > *second) return 1;
        first++;
        second++;
    }
    return 0; // they are the same strings
}

int main(int argc, const char* argv[]) {
    const char* filename;
    const char* user_prefix;
    int fd;
    int read_result;

    // checking for too many arguments
    if (argc > 3) {
        errno = E2BIG;
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDIN_FILENO, "\n", 1);
        return -1;
    }
    // checking for filename arg
    if (!argv[1]) {
        errno = ENOENT;
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDIN_FILENO, "\n", 1);
        return -1;
    }
    filename = argv[1];

    if(argv[2]) {
        // prefix must be 6 characters
        if (_strlen(argv[2]) != 6) {
            errno = EINVAL;
            write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
            write(STDIN_FILENO, "\n", 1);
            return -1;
        }
        _atoi(argv[2]); // checking for proper integer format
        user_prefix = argv[2];
    }
    else {
        return -1;
    }

    
    return 0;
}