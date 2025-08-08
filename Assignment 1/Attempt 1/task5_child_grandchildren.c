#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t child = fork();

    if (child == 0) {
        // Inside child
        printf("Child process ID: %d\n", getpid());

        for (int i = 0; i < 3; i++) {
            pid_t gpid = fork();
            if (gpid == 0) {
                printf("Grandchild process ID: %d\n", getpid());
                return 0;
            }
        }

        for (int i = 0; i < 3; i++)
            wait(NULL);

    } else {
        printf("Parent process ID: %d\n", getpid());
        wait(NULL);
    }

    return 0;
}