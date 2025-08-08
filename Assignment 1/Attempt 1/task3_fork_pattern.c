#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int count = 1; // start with original process

    pid_t a = fork();
    if (a == 0) {
        if (getpid() % 2 != 0) {
            fork();
            count++;
        }
    } else {
        wait(NULL);
        pid_t b = fork();
        if (b == 0) {
            if (getpid() % 2 != 0) {
                fork();
                count++;
            }
        } else {
            wait(NULL);
            pid_t c = fork();
            if (c == 0) {
                if (getpid() % 2 != 0) {
                    fork();
                    count++;
                }
            } else {
                wait(NULL);
            }
        }
    }

    sleep(1);
    if (getppid() != 1) // Avoid zombie detection
        printf("Process PID: %d\n", getpid());

    return 0;
}