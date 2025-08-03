// This program forks a child process that replaces its image using execl() to run 'ls'.

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Starting program\n");

    pid_t pid = fork();

    if (pid == -1) {
        perror("Fork failed");
        exit(1);
    }

    if (pid == 0) {
        // Child process
        printf("Child process ID = %d\n", getpid());
        execl("/bin/ls", "ls", NULL);

        // If execl fails
        perror("execl failed");
    } else {
        // Parent process
        wait(NULL);
        printf("Parent process done\n");
    }

    return 0;
}
