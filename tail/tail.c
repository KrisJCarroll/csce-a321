#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

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
            exit(-1);
        }
        res = res * 10 + next_digit;
        ++str;
    }
    return res;
}

int _getline(int fd, char** buffer, int buf_size) {
    char ch;
    int len = 0;
    int read_result;
    *buffer = realloc(*buffer, sizeof(char) * buf_size);
    while((read_result = read(fd, &ch, 1)) != 0 && ch != '\n') {
        if (read_result < 0) return -1;
        (*buffer)[len++] = ch;
        if (len == buf_size) {
            buf_size *= 2;
            *buffer = realloc(*buffer, sizeof(char) * buf_size);
        }
    }
    if(read_result == 0) {
        return 0;
    }
    (*buffer)[len++] = '\n';
    *buffer = realloc(*buffer, sizeof(char) * len);
    return len;
}

int main(int argc, const char *argv[]) {
    const char* filename;
    int fd;
    int NUM_LINES = 10;

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
        }
    }

    // arguments handled, begin execution of tail functionality
    // if we have a filename, use the file for reading
    if (filename) {
        fd = open(filename, O_RDONLY);

        // error opening
        if (fd < 0) {
            write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
            return -1;
        }
    }
    // no filename provided, set fd to stdin
    else {
        fd = STDIN_FILENO;
    }

    // making space for buffer of last NUM_LINES lines
    char **lines = calloc(sizeof(char*), NUM_LINES);
    if (!lines) { // calloc error
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        return -1;
    }
    int line_num = 0;

    int read_result;
    int write_result;
    char *line;

    // begin reading until EOF
    while ((read_result = _getline(fd, &line, 10)) != 0) {
        if (read_result < 0) {
            write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
            return -1;
        }
        if(lines[line_num]) free(lines[line_num]);
        lines[line_num] = line;
        line = NULL;
        line_num = (line_num + 1) % NUM_LINES;
    }

    for(int i = 0; i < NUM_LINES; i++) {
        if(lines[i]) {
            write_result = write(STDOUT_FILENO, lines[i], _strlen(lines[i]));
            if (write_result < 0) {
                write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
                return -1;
            }
        }
    }

    return 0;
}