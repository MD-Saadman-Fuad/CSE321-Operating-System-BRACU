#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t child, grandchild1, grandchild2, grandchild3;
    printf("1. Parent process ID : %d\n", getpid()); //taking real process id, 0 is not available irl
    child = fork();

    if (child == 0){
        printf("2. Child process ID: %d\n", getpid());

        grandchild1 = fork();

        if (grandchild1 == 0) {
            printf("3. Grand Child process ID: %d\n", getpid());
            exit(0);
        }
        grandchild2 = fork();

        if (grandchild2 == 0) {
            printf("4. Grand Child process ID: %d\n", getpid());
            exit(0);
        }
        grandchild3 = fork();

        if (grandchild3 == 0) {
            printf("5. Grand Child process ID: %d\n", getpid());
            exit(0);
        }

        wait(NULL);
        wait(NULL);
        wait(NULL);
    }else{
        wait(NULL);
    }
    return 0;
}