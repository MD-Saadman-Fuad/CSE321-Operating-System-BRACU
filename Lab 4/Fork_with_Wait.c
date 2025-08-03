// This program forks a child and uses wait() to pause the parent until the child finishes.

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    int pid, status;
    pid = fork();

    if (pid == -1) {
        printf("Fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        printf("Child here!\n");
    } else {
        wait(&status);
        printf("Well done kid\n");
    }

    return 0;
}
