// This program forks a child process that runs `ls`,
// then the parent waits and runs `pwd`.

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>  // for wait()

int main() {
    printf("1\n");
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        printf("Child process id = %d\n", getpid());
        execl("/bin/ls", "ls", NULL);
        perror("child exec failed"); // Only if exec fails
    } 
    else if (pid > 0) {
        // Parent process
        wait(NULL); // Wait for child to finish
        printf("Parent process id = %d\n", getpid());
        execl("/bin/pwd", "pwd", NULL);
        perror("parent exec failed"); // Only if exec fails
    } 
    else {
        // Fork failed
        perror("Fork failed");
    }

    return 0;
}
