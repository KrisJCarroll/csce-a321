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
            exit(-1);
        }
        res = res * 10 + next_digit;
        ++str;
    }
    return res;
}

int main(int argc, const char* argv[]) {
    const char* filename;
    const char* prefix;
    int fd;
    int read_result;


    if (argc > 3) {
        errno = E2BIG;
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        return -1;
    }
    // checking for filename arg
    if (!argv[1]) {
        errno = ENOENT;
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        return -1;
    }
    filename = argv[1];

    if(argv[2]) {
        // prefix must be 6 characters
        if (_strlen(argv[2]) != 6) {
            errno = EINVAL;
            write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
            return -1;
        }
        _atoi(argv[2]); // checking for proper integer format
        prefix = argv[2];
    }
    else {
        return -1;
    }


    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        return -1;
    }

    // adapted from example by Dr. Christoph Lauter
    typedef struct {
        char prefix[6];
        char location[25];
        char padding;
    } entry_t;

    // getting size
    off_t size = lseek(fd, size, SEEK_END);
    //int entries = (int) size / sizeof(entry_t);
    //write(STDOUT_FILENO, &entries, sizeof(entries));

    off_t loc = lseek(fd, (size / 2), SEEK_SET);
    char* test_prefix;
    entry_t entry;
    read_result = read(fd, &entry, sizeof(entry));
    write(STDOUT_FILENO, entry.prefix, sizeof(entry.prefix));
    write(STDOUT_FILENO, "\n", 1);
    write(STDOUT_FILENO, entry.location, sizeof(entry.location));
    
    
}