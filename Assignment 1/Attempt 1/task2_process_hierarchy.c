#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t child = fork();

    if (child == 0) {
        // Inside child
        pid_t grandchild = fork();
        if (grandchild == 0) {
            // Inside grandchild
            printf("I am grandchild\n");
        } else {
            wait(NULL);  // Wait for grandchild
            printf("I am child\n");
        }
    } else {
        wait(NULL);  // Wait for child
        printf("I am parent\n");
    }

    return 0;
}