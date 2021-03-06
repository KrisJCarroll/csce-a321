/* tail.c
Author: Kristopher Carroll
CSCE A321 - University of Alaska Anchorage
Dr. Christoph Lauter

This program implements the head function as it is used in the Linux command line using
no calls to higher level C functions (other than strerror()) and is based primarily on
the syscalls of read() and write().
*/

#include <string.h> // only used for strerror()
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


int _strlen(const char *str) {
    if (*str)
        return 1 + _strlen(++str);
    return 0;
}

int _atoi(const char *str) {
    int res = 0;
    while (*str) {
        int next_digit = (*str) - '0';
        if (next_digit < 0 || next_digit > 9) {
            errno = EINVAL;
            write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
            exit(-1);
        }
        res = res * 10 + (*str) - '0';
        ++str;
    }
    return res;
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

    // close file to clean up memory
    if (close(fd) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        return -1;
    }

    return 0;
}