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

    // opening file and checking for open errors
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDIN_FILENO, "\n", 1);
        return -1;
    }
    // struct for holding the expected format of data in nanpa for easy, understandable access
    // adapted from example by Dr. Christoph Lauter
    typedef struct {
        char prefix[6];
        char location[25];
        char padding;
    } entry_t;

    // getting size of file and initializing search window sizes
    off_t window_max = lseek(fd, 0, SEEK_END);
    off_t window_min = 0;
    off_t loc = (window_max / 2) + (window_max % sizeof(entry_t));


    entry_t entry;

    // binary search algorithm adapted to maintain 32-byte frame alignment
    // TODO: optimize by checking first and last entries (we can do this easily while getting size of file)
    //       to see if pattern falls outside of range of values found in file
    while (loc > window_min && loc < window_max) {
        lseek(fd, loc, SEEK_SET);
        if( (read_result = read(fd, &entry, sizeof(entry_t))) < 0) {
            write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
            return -1;
        }
        int result = _strcmp(user_prefix, entry.prefix);
        // found a match, output location and end with code 0
        if (result == 0) {
            write(STDOUT_FILENO, entry.location, sizeof(entry.location));
            write(STDOUT_FILENO, "\n", 1);
            return 0;
        }
        // user_prefix is larger than current prefix - update search window min and move search position
        else if (result > 0) {
            window_min = loc;
            loc += (window_max - window_min) / 2;
            loc += (loc % sizeof(entry_t)); // ensure alignment with 32-byte frame
        }

        // user_prefix smaller than current prefix - update search window max and move search position
        else if (result < 0) {
            window_max = loc;
            loc -= (window_max - window_min) / 2;
            loc -= (loc % sizeof(entry_t));
        }
    }

    // location not found
    char* msg = "ERROR: Prefix not found.\n";
    write(STDERR_FILENO, msg, _strlen(msg));
    return -1;
}