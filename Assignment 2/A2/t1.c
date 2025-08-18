#include <stdio.h>
#include <pthread.h>

int n, s;
int fib[41];
int search[100];
int result[100];

void *generate_fibonacci(void *arg) {
    fib[0] = 0;
    if (n > 0) fib[1] = 1;
    for (int i = 2; i <= n; i++)
        fib[i] = fib[i-1] + fib[i-2];
    pthread_exit(NULL);
}

void *search_fibonacci(void *arg) {
    for (int i = 0; i < s; i++) {
        int idx = search[i];


        if (idx >= 0 && idx <= n)
            result[i] = fib[idx];
        else
            result[i] = -1;
    }


    pthread_exit(NULL);
}

int main() {
    pthread_t t1, t2;

    printf("Enter the term of fibonacci sequence:\n");
    scanf("%d", &n);
    printf("How many numbers you are willing to search?:\n");
    scanf("%d", &s);



    for (int i = 0; i < s; i++) {
        printf("Enter search %d:\n", i+1);
        scanf("%d", &search[i]);
    }

    pthread_create(&t1, NULL, generate_fibonacci, NULL);
    pthread_join(t1, NULL);
    for (int i = 0; i <= n; i++)
        printf("a[%d] = %d\n", i, fib[i]);



    pthread_create(&t2, NULL, search_fibonacci, NULL);
    pthread_join(t2, NULL);
    for (int i = 0; i < s; i++)
        printf("result of search #%d = %d\n", i+1, result[i]);

    return 0;
}

