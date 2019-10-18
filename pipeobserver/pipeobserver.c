/* pipeobserver.c
Author: Kristopher Carroll
CSCE A321 - University of Alaska Anchorage
Dr. Christoph Lauter

This program functions similarly to the tee command in the Linux command line.
The function takes two commands that are to be piped together as well as a filename to write
the contents of the first command to.
*/

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
/* parse_args
   Inputs:
        - int argc from main()
        - const char* argv[] from main()
        - int* fd for the file descriptor in main to be used for open()
        - char* cmd[10] for commands on either side of pipeobserver as well as their args
            *** NOTE - this means that each command can only support at most 9 args (final arg must be NULL) ***
    Returns:
        - returns 0 if all args were parsed successfully
        - returns -1 if any args were parsed unsuccessfully and outputs expected usage to STDERR
*/
int parse_args(int argc, const char* argv[], int *fd, char* cmd1[10], char* cmd2[10]) {
    int i;
    const char* filename = argv[1]; // grab the filename to open
    const char* usage = "ERROR: expected usage is ./pipeobserver <filename> [ process1 args ] [ process2 args ]\n";
    
    // try opening the file with write permissions - overwrite the file if it already exists
    if ((*fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXO | S_IRWXU | S_IRWXG)) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return -1;
    }

    // parsing first executable and associated args
    if (_strcmp(argv[2], "[") != 0) { // required formatting
        errno = EINVAL;
        write(STDERR_FILENO, usage, _strlen(usage));
        return -1;
    }
    cmd1[0] = (char*)argv[3];
    i = 4;
    int count = 1;
    // loop to get all args for first executable
    while (_strcmp(argv[i], "]") != 0) { // required formatting
        if (i + 1 > argc) {
            errno = EINVAL;
            write(STDERR_FILENO, usage, _strlen(usage));
            return -1;
        }
        cmd1[count] = (char*)argv[i]; // appending arg
        i++;
        count++;
    }
    cmd1[count] = NULL; // append NULL to the end of args list

    // parsing second executable and associated args
    if (_strcmp(argv[++i], "[") != 0) {
        write(STDERR_FILENO, "abc", 3);
        errno = EINVAL;
        write(STDERR_FILENO, usage, _strlen(usage));
        return -1;
    }
    cmd2[0] = (char*)argv[++i];
    i++;
    count = 1;
    while (_strcmp(argv[i], "]") != 0) {
        if (i + 1 > argc) {
            errno = EINVAL;
            write(STDERR_FILENO, usage, _strlen(usage));
            return -1;
        }
        cmd2[count] = (char*)argv[i];
        i++;
        count++;
    }
    cmd2[count] = NULL;

    return 0;
}

// accessory function for piping and checking for errors
void pipe_it(int fds[2]) {
    if (pipe(fds) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        exit(1);
    }
}

// accessory function for forking and checking for errors
void fork_it(pid_t* pid) {
    if ((*pid = fork()) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        exit(1);
    }
}

// accessory function for dup2() operaitons and checking for errors
// also closes appropriate unused file descriptors from pipe
void dup_it(int fd, int* pipe) {
    if(dup2(pipe[fd], fd) < 0) {
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        exit(1);
    }
    close(pipe[1 - fd]); // close the end of the pipe we didn't duplicate
}

// accessory function for running dup2() followed by execvp() operations
void dup_and_exec(int fd, int* pipe, char* cmd[]) {
    dup_it(fd, pipe);
    execvp(cmd[0], cmd);
    // if we get here, execvp failed
    write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
    write(STDERR_FILENO, "\n", 1);
    exit(1);
}

int main(int argc, const char* argv[]) {

    int filefd = 0;
    char* cmd1[10]; // storage for first command and associated args
    char* cmd2[10]; // storage for second command and associated args

    int pipe1[2]; 
    pid_t pid1;
    pid_t pid2;
    pid_t pid3;
    pid_t pid4;

    // checking for successful parsing of args
    if (parse_args(argc, argv, &filefd, cmd1, cmd2) < 0) {
        errno = EINVAL;
        write(STDERR_FILENO, strerror(errno), _strlen(strerror(errno)));
        write(STDERR_FILENO, "\n", 1);
        return 1;
    }

    pipe_it(pipe1); // create pipe

    // making first child
    fork_it(&pid1);
    if (pid1 == 0) { // child 1
        dup_and_exec(STDOUT_FILENO, pipe1, cmd1); // replace stdout with write end of pipe
    }
    
    // making second child
    fork_it(&pid2);
    if (pid2 == 0) { // child 2
        dup_it(STDIN_FILENO, pipe1); // replace stdin with read end of pipe
        pipe_it(pipe1); // establish new pipe

        // making grandchild 1 (pid3)
        fork_it(&pid3);
        if (pid3 == 0) { // grandchild 1 (pid3)
            dup_it(STDOUT_FILENO, pipe1); // replace STDOUT with write end of pipe
            char* buffer = malloc(sizeof(char) * 512); // create a decently sized buffer to read/write from
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
            return 0; // VERY IMPORTANT - unexpected behavior will happen if this child is not forced to exit here
        }

        // making grandchild 2 (pid4)
        fork_it(&pid4);
        
        if (pid4 == 0) { // grandchild 2 (pid4)
            dup_and_exec(STDIN_FILENO, pipe1, cmd2);
        }
        
        // clean up
        close(pipe1[0]);
        close(pipe1[1]);
        
        // wait for children to terminate
        waitpid(pid3, NULL, 0);
        waitpid(pid4, NULL, 0);
        return 0;
    }
/* ---------------------------------------------------------------------------------------------------
    PARENT PROCESS CONTINUES HERE
   -------------------------------------------------------------------------------------------------*/

    // cleanup
    close(pipe1[0]);
    close(pipe1[1]);
    
    // wait for children to terminate
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    // we made it, celebrate appropriately with a success code
    return 0;
}