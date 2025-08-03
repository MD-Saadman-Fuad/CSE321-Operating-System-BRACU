// This program creates a child process using fork() and identifies the parent and child.

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid;
    pid = fork();

    if (pid == 0)
        printf("I'm the child process\n");
    else if (pid > 0)
        printf("I'm the parent process. My child's PID is %d\n", pid);
    else
        perror("Error in fork");

    return 0;
}
