/* findlocationfastmemory.c
Author: Kristopher Carroll
CSCE A321 - University of Alaska Anchorage
Dr. Christoph Lauter

This program searches for the user supplied North American phone prefix (first 6 digits)
by mapping the appropriate prefix file (nanpa) into memory and performing binary search.
*/

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>

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
            exit(1);
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

    // checking for too many arguments
    if (argc > 3) {
        errno = E2BIG;
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDIN_FILENO, "\n", 1);
        return 1;
    }
    // checking for filename arg
    if (!argv[1]) {
        errno = ENOENT;
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDIN_FILENO, "\n", 1);
        return 1;
    }
    filename = argv[1];

    if(argv[2]) {
        // prefix must be 6 characters
        if (_strlen(argv[2]) != 6) {
            errno = EINVAL;
            write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
            write(STDIN_FILENO, "\n", 1);
            return 1;
        }
        _atoi(argv[2]); // checking for proper integer format
        user_prefix = argv[2];
    }
    else {
        return 1;
    }

    // struct for casting data read from file into its expected form
    // source: Dr. Christoph Lauter
    typedef struct {
        char prefix[6];
        char location[25];
        char padding;
    } entry_t;

    // opening file
    if ((fd = open(filename, O_RDONLY)) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1;
    }

    off_t size = lseek(fd, 0, SEEK_END);
    int num_entries = size / sizeof(entry_t);

    // putting file in memory and casting into entry_t structs
    void* file_in_mem = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_in_mem == MAP_FAILED) { // error handling
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1;
    }

    // close file as we're done with it
    if (close(fd) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1;
    }

    entry_t* entries = (entry_t *) file_in_mem; // casting as array of entry_t for easy access

    // binary search
    int window_min = 0;
    int window_max = num_entries - 1;
    int loc = (window_min + window_max) / 2;

    while(loc > window_min && loc < window_max) {
        
        int result = _strcmp(user_prefix, entries[loc].prefix);
        // user_prefix is greater than search prefix, search top half
        if (result > 0) {
            window_min = loc + 1;
        }
        // user_prefix is less than search prefix, search bottom half
        else if (result < 0) {
            window_max = loc - 1;
        }
        // found match, print correct location, unmap memory and exit with code 0
        else if (result == 0) {
            write(STDOUT_FILENO, entries[loc].location, sizeof(((entry_t*)0)->location)); // https://stackoverflow.com/a/3553314
            write(STDOUT_FILENO, "\n", 1);
            if (munmap(entries, size) < 0) write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
            return 0;
        }
        loc = (window_min + window_max) / 2;
    }
    // prefix not found
    char* msg = "ERROR: Prefix not found.\n";
    write(STDERR_FILENO, msg, _strlen(msg));
    return -1;
}