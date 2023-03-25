#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>


int main(int argc, char **argv) {

    char arg = argv[1][0];

    // if user doesn't enter a valid argument
    if (arg != 'w' && arg != 's') {
        // Inform user of invalid input
        printf("Please enter 'w' or 's' as an argument. \n");
        return(-1);
    }

    pid_t pid = fork();

    // Got from Operating Systems Concepts 10th Edition
    if (pid < 0) { /* Fork failed */
        fprintf(stderr, "Fork Failed");
        return 1;
    } else if (pid == 0){
        // if child process, replace with myclock execution process
        execl("./myclock.txt", "myclock", "out1", (char *) NULL);
    }
    
    // Check if argument is 'w' or 's'
    if (argv[1][0] == 'w') {
        // if w, wait for termination of child process
        int status;
        waitpid(pid, &status, 0);
    } else if (argv[1][0] == 's') {
        // if s, sleep for 2 minutes
        sleep(120);
    }
    return 0;
}