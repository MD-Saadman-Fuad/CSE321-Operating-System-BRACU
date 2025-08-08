#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int compare(const void *a, const void *b) {
    return (*(int *)b - *(int *)a);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s num1 num2 ...\n", argv[0]);
        return 1;
    }

    int arr[argc - 1];
    for (int i = 1; i < argc; i++)
        arr[i - 1] = atoi(argv[i]);

    pid_t pid = fork();

    if (pid == 0) {
        // Child sorts
        qsort(arr, argc - 1, sizeof(int), compare);
        printf("Child sorted array (desc): ");
        for (int i = 0; i < argc - 1; i++)
            printf("%d ", arr[i]);
        printf("\n");
    } else {
        wait(NULL); // Parent waits for sorting
        printf("Parent checks even/odd:\n");
        for (int i = 0; i < argc - 1; i++) {
            printf("%d is %s\n", arr[i], (arr[i] % 2 == 0) ? "Even" : "Odd");
        }
    }

    return 0;
}