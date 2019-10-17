#include <unistd.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <string.h>



int _strlen(const char *str) {
    if (*str)
        return 1 + _strlen(++str);
    return 0;
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

// TODO: get cmds and argvs by reference so they're available to main()
int parse_args(int argc, const char* argv[], int *fd) {
    int i;
    const char* filename = argv[1];
    const char* cmd1;
    const char* cmd2;
    const char* argv1[5];
    const char* argv2[5];

    const char* usage = "ERROR: expected usage is ./pipeobserver <filename> [ process1 args ] [ process2 args ]\n";
    if ((*fd = open(filename, O_RDWR | O_APPEND | O_CREAT), 0777) < 0) {
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

int main(int argc, const char* argv[]) {

    int filefd = 0;

    int pipe1[2]; // child1 will connect STDOUT to pipe1[1], child2 will connect STDIN to pipe1[0]
    pid_t pid1;
    pid_t pid2;
    
    if (parse_args(argc, argv, &filefd) < 0) {
        errno = EINVAL;
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1;
    }

    if (pipe(pipe1) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1;
    }

    if ((pid1 = fork()) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1;
    }

    return 0;
}