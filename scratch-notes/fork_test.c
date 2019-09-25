/* ----------------------------------------------------------------------------------------------------------
            CAUTION: FORK TESTING HAPPENING BELOW THIS LINE - PROCEED WITH CAUTION OR GET FORKED
    ---------------------------------------------------------------------------------------------------------- */
    #include <unistd.h>
    #include <errno.h>
    #include <stdio.h>
    #include <string.h>
    #include <sys/types.h>
    #include <sys/wait.h>

    int main() {
        pid_t child_pid, child_two_pid;
        int pipes[2];
        int in, out;
        int a,b;

        if (pipe(pipes) < 0) {
            fprintf(stderr, "pipe() did not work: %s\n", strerror(errno));
            return -1;
        }
        in = pipes[0];
        out = pipes[1];


        printf("Hello, I am the process %d before the fork \n", (int)getpid());

        child_pid = fork();
        // on failure, -1 returned to parent
        if (child_pid < 0) {
            fprintf(stderr, "fork() did not work: %s\n", strerror(errno));
            return -1;
        }
        // Fork worked, we are either the child or the parent
        if (child_pid == 0) { 
            // I am the child, id of 0
            printf("Hello, I am the first child. My PID is: %d\n", (int) getpid());
            a = 42;
            write(out, &a, sizeof(a)); // TODO: Error handling (fd, buffer, size)
            sleep(10);
            
            printf("I am the first child. I am going to die.\n");

            return 0;
        }

        // I am the parent
        printf("My first child process has PID: %d\n", (int) child_pid);


        /* SECOND CHILD DOWN HERE */

        child_two_pid = fork();
        // on failure, -1 returned to parent
        if (child_two_pid < 0) {
            fprintf(stderr, "fork() did not work: %s\n", strerror(errno));
            wait(NULL);
            return -1;
        }
        // Fork worked, we are either the child or the parent
        if (child_two_pid == 0) { 
            // I am the child, id of 0
            printf("Hello, I am the second child. My PID is: %d\n", (int) getpid());
            b = 17;
            read(in, &b, sizeof(b)); // TODO: Error handling
            printf("I am the second child. My sibling transmitted %d to me\n", b);
            sleep(15);
            printf("I am the second child. I am going to die.\n");

            return 0;
        }

        // I am the parent
        printf("My second child process has PID: %d\n", (int) child_two_pid);
        // wait for children to die
        wait(NULL); // TODO: Do error handling
        wait(NULL);
        printf("I am the parent, PID: %d. My child processes died. I am going to terminate myself.\n", (int) getpid());
    }