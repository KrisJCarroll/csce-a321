#include <unistd.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

#define READ_END 0
#define WRITE_END 1


// recursive implementation of strlen()
int _strlen(const char *str) {
    if (*str)
        return 1 + _strlen(++str);
    return 0;
}

// implementation of strcmp()
int _strcmp(const char* first, const char* second) {
    while ((*first) && (*second)) {
        if (*first < *second) return -1;
        if (*first > *second) return 1;
        first++;
        second++;
    }
    return 0; // they are the same strings
}

// TODO: get cmds and argvs by reference so they're available to main()
int parse_args(int argc, const char* argv[], int *fd) {
    int i;
    const char* filename = argv[1];
    const char* cmd1;
    const char* cmd2;
    const char* argv1[5];
    const char* argv2[5];

    const char* usage = "ERROR: expected usage is ./pipeobserver <filename> [ process1 args ] [ process2 args ]\n";
    if ((*fd = open(filename, O_RDWR | O_TRUNC | O_APPEND | O_CREAT, S_IRWXO | S_IRWXU | S_IRWXG)) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return -1;
    }

    // parsing first executable and associated args
    if (_strcmp(argv[2], "[") != 0) {
        errno = EINVAL;
        write(STDERR_FILENO, usage, _strlen(usage));
        return -1;
    }
    cmd1 = argv[3];
    argv1[0] = argv[3];
    i = 4;
    int count = 1;
    while (_strcmp(argv[i], "]") != 0) {
        if (i + 1 > argc) {
            errno = EINVAL;
            write(STDERR_FILENO, usage, _strlen(usage));
            return -1;
        }
        argv1[count] = argv[i];
        i++;
        count++;
    }
    argv1[count] = NULL;

    // parsing second executable and associated args
    if (_strcmp(argv[++i], "[") != 0) {
        write(STDERR_FILENO, "abc", 3);
        errno = EINVAL;
        write(STDERR_FILENO, usage, _strlen(usage));
        return -1;
    }
    cmd2 = argv[++i];
    i++;
    argv2[0] = cmd2;
    count = 1;
    while (_strcmp(argv[i], "]") != 0) {
        if (i + 1 > argc) {
            errno = EINVAL;
            write(STDERR_FILENO, usage, _strlen(usage));
            return -1;
        }
        argv2[count] = argv[i];
        i++;
        count++;
    }
    argv2[count] = NULL;

    return 0;
}

void pipe_it(int fds[2]) {
    if (pipe(fds) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        exit(1);
    }
}

void fork_it(pid_t* pid) {
    if ((*pid = fork()) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        exit(1);
    }
}

void dup_it(int fd, int* pipe) {
    close(fd);
    if(dup2(pipe[fd], fd) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        exit(1);
    }
    close(pipe[1 - fd]); // close the end of the pipe we didn't duplicate
}

void dup_and_exec(int fd, int* pipe, char* cmd[]) {
    dup_it(fd, pipe);
    execvp(cmd[0], cmd);
}

int main(int argc, const char* argv[]) {

    int filefd = 0;

    int pipe1[2]; // child1 will connect STDOUT to pipe1[1], child2 will connect STDIN to pipe1[0]
    pid_t pid1;
    pid_t pid2;
    pid_t pid3;
    pid_t pid4;

    // checking for successful parsing of args
    if (parse_args(argc, argv, &filefd) < 0) {
        errno = EINVAL;
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1;
    }

    pipe_it(pipe1);

    // making first child
    fork_it(&pid1);
    // child 1
    char* cmd1[] = {"ls", "-l", 0};
    if (pid1 == 0) {
        dup_and_exec(STDOUT_FILENO, pipe1, cmd1); // replace stdout with write end of pipe
    }
    
    // making second child
    fork_it(&pid2);
    // child 2
    if (pid2 == 0) {
        // replace stdin with read end of pipe
        dup_it(STDIN_FILENO, pipe1);
        // establish new pipe
        pipe_it(pipe1);

        // making grandchild 1 (pid3)
        fork_it(&pid3);
        // grandchild 1 (pid3)
        if (pid3 == 0) {
            dup_it(STDOUT_FILENO, pipe1);
            char* buffer = malloc(sizeof(char) * 512);
            ssize_t read_result;
            // continue reading 512 bytes at a time until EOF reached
            while((read_result = read(STDIN_FILENO, buffer, sizeof(buffer))) != 0) {
                if(read_result < 0) {
                    write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
                    write(STDERR_FILENO, "\n", 1);
                    return 1;
                }
                write(filefd, buffer, read_result); // only write the amount read (for the end of the file)
                write(STDOUT_FILENO, buffer, read_result);
            }
            close(filefd); // close the file
            return 0;
        }

        // making grandchild 2 (pid4)
        fork_it(&pid4);
        // grandchild 2 (pid4)
        if (pid4 == 0) {
            char* cmd2[] = {"wc", 0};
            dup_and_exec(STDIN_FILENO, pipe1, cmd2);
        }
        
    
        close(pipe1[0]);
        close(pipe1[1]);
        
        waitpid(pid3, NULL, 0);
        waitpid(pid4, NULL, 0);
    }


    // parent process
    close(pipe1[0]);
    close(pipe1[1]);
    
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}