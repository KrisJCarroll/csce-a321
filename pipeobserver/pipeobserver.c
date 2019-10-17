#include <unistd.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <string.h>



int _strlen(const char *str) {
    if (*str)
        return 1 + _strlen(++str);
    return 0;
}

int parse_args(int argc, const char* argv[], int *fd) {
    const char* filename = argv[1];
    if ((*fd = open(filename, O_WRONLY | O_APPEND | O_CREAT)) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return -1;
    }
    return 0;
}

int main(int argc, const char* argv[]) {

    int filefd = 0;

    int fd1[2]; // piped process writes to fd1[1] for parent to read at fd1[0]
    int fd2[2]; // parent writes at fd2[1] for child1 to read at fd2[0]
    int fd3[3]; // parent writes at fd3[1] for piped process to read at fd3[0]
    pid_t pid1;
    pid_t pid2;
    
    parse_args(argc, argv, &filefd);


    return 0;
}