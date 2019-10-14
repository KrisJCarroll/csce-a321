#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>


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
    int fd;
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

    // arguments handled, begin execution of head functionality
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

    int linecount = 0;
    char curr_char;
    ssize_t read_result;
    ssize_t write_result;
    // begin reading and writing
    while(linecount < NUM_LINES) {
        read_result = read(fd, &curr_char, 1);
        // read error
        if(read_result < 0) {
            write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
            return -1;
        }

        // check for newline character '\n'
        if (curr_char == '\n') {
            linecount++;
        }
        write_result = write(STDOUT_FILENO, &curr_char, 1);

        // write error
        if (write_result < 0) {
                write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
                return -1;
        }
    }
}