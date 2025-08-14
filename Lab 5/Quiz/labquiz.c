#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock; // mutex for I/O synchronization

void *multiples_3_5(void *arg) {
    int n, count = 0;

    pthread_mutex_lock(&lock);
    printf("Enter n: ");
    scanf("%d", &n);
    pthread_mutex_unlock(&lock);

    pthread_mutex_lock(&lock);
    printf("Multiples of 3 and 5 up to %d:\n", n);
    pthread_mutex_unlock(&lock);

    for (int i = 1; i <= n; i++) {
        if (i % 3 == 0 && i % 5 == 0) {
            pthread_mutex_lock(&lock);
            printf("%d ", i);
            pthread_mutex_unlock(&lock);
            count++;
        }
    }

    pthread_mutex_lock(&lock);
    printf("\nCount: %d\n", count);
    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}

void *sum_odd(void *arg) {
    int m, sum = 0;

    pthread_mutex_lock(&lock);
    printf("Enter m: ");
    scanf("%d", &m);
    pthread_mutex_unlock(&lock);

    for (int i = 1; i <= m; i += 2) {
        sum += i;
    }

    pthread_mutex_lock(&lock);
    printf("Sum of odd numbers from 1 to %d: %d\n", m, sum);
    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}

int main() {
    pthread_t t1, t2;

    pthread_mutex_init(&lock, NULL); // initialize mutex

    pthread_create(&t1, NULL, multiples_3_5, NULL);
    pthread_create(&t2, NULL, sum_odd, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&lock); // cleanup mutex

    return 0;
}