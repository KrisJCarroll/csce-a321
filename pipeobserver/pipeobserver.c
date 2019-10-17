#include <unistd.h>
#include <sys/errno.h>


int _strlen(const char *str) {
    if (*str)
        return 1 + _strlen(++str);
    return 0;
}

int main(int argc, const char* argv[]) {

    int fd1[2]; // piped process writes to fd1[1] for parent to read at fd1[0]
    int fd2[2]; // parent writes at fd2[1] for child1 to read at fd2[0]
    int fd3[3]; // parent writes at fd3[1] for piped process to read at fd3[0]
    pid_t pid1;
    pid_t pid2;
    
    // error checking during piping process
    if (pipe(fd1) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1; // fd[0] for one end and fd[1] for the other
    }
    if (pipe(fd2) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1; // fd[0] for one end and fd[1] for the other
    }
    if (pipe(fd3) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1; // fd[0] for one end and fd[1] for the other
    }

    pid1 = fork();
    // fork error
    if (pid1 < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1;
    }

    // child 1
    if (pid1 == 0) {

    }

    // parent
    pid2 = fork();

    if (pid2 < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1;
    }

    // child 2
    if (pid2 == 0) {
        
    }


    return 0;
}