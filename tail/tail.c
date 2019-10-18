/* tail.c
Author: Kristopher Carroll
CSCE A321 - University of Alaska Anchorage
Dr. Christoph Lauter

This program implements the tail function as it is used in the Linux command line using
no calls to higher level C functions (other than strerror()) and is based primarily on
the syscalls of read() and write().
*/

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

// main logic for handling line reads - takes the fd to read, a char* buffer by reference to be used for line storage
// and int for initial size of buffer. this function will reallocate buffer as needed (doubles in size every time it fills)
// returns the length of line for buffer if successful, returns 0 if EOF reached, and returns -1 if read failure occurs
int _getline(int fd, char** buffer, int buf_size) {
    char ch;
    int len = 0;
    int read_result;
    // ensure fresh allocation according to parameterized size
    *buffer = realloc(*buffer, sizeof(char) * buf_size);

    // loop for adding to buffered line until EOF or newline
    while((read_result = read(fd, &ch, 1)) != 0 && ch != '\n') {
        if (read_result < 0) return -1; // read error - pass back to caller
        (*buffer)[len++] = ch;
        // buffer is full, increase memory allocated
        if (len == buf_size) {
            buf_size *= 2;
            *buffer = realloc(*buffer, sizeof(char) * buf_size);
        }
    }
    // EOF found, pass this information to caller
    if(read_result == 0) {
        return 0;
    }
    (*buffer)[len++] = '\n'; // add the newline
    *buffer = realloc(*buffer, sizeof(char) * len); // reallocate buffer to correct size
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
        if(lines[line_num]) free(lines[line_num]); // line storage is full so free the correct spot to make room for new line
        lines[line_num] = line;
        line = NULL; // 
        line_num = (line_num + 1) % NUM_LINES; // update which line_num we're working on, wrapped around NUM_LINES
    }

    // EOF was reached, write out the appropriate number of lines
    for(int i = 0; i < NUM_LINES; i++) {
        // check if line exists (for cases where number of lines stored was less than NUM_LINES)
        if(lines[i]) {
            write_result = write(STDOUT_FILENO, lines[i], _strlen(lines[i]));
            if (write_result < 0) {
                write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
                return -1;
            }
        }
    }

    // close file to clean up memory
    if (close(fd) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        return -1;
    }

    // we made it! return success
    return 0;
}