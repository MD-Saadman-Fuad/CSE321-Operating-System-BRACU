#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int is_prime(int num) {
    if (num < 2) return 0;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) return 0;
    }
    return 1;
}

int main() {
    int fd[2];
    pid_t a;
    char buff[200];

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    a = fork();
    if (a < 0) {
        perror("fork");
        exit(1);
    }
    else if (a == 0) {  
        // Child process
        close(fd[0]); // Close read end

        int num;
        printf("Enter a number: ");
        scanf("%d", &num);

        if (is_prime(num))
            strcpy(buff, "The number is a prime number.");
        else
            strcpy(buff, "The number is not a prime number.");

        printf("Writing data for sending...\n");
        write(fd[1], buff, strlen(buff) + 1);
        printf("Writing done.\n");

        close(fd[1]);
    }
    else {  
        // Parent process
        wait(NULL);
        close(fd[1]); // Close write end

        printf("Reading data after receiving...\n");
        read(fd[0], buff, sizeof(buff));
        printf("Data received: %s\n", buff);

        close(fd[0]);
    }

    return 0;
}
