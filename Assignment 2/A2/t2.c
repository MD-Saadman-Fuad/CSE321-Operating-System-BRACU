#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define CHAIRS 3
#define STUDENTS 10

sem_t student_sem;
sem_t st_sem;
pthread_mutex_t mutex;


int waiting = 0;
int served = 0;
int finished = 0;




void *student(void *arg) {
    int id = *(int *)arg;

    pthread_mutex_lock(&mutex);
    if (waiting < CHAIRS) {
        waiting++;
        printf("Student %d started waiting for consultation\n", id);
        sem_post(&student_sem);
        pthread_mutex_unlock(&mutex);
        sem_wait(&st_sem);




        printf("Student %d is getting consultation\n", id);
        sleep(1);
        printf("Student %d finished getting consultation and left\n", id);

        pthread_mutex_lock(&mutex);
        served++;
        finished++;
        printf("Number of served students: %d\n", served);



        if (finished == STUDENTS) {
            sem_post(&student_sem); 
        }
        pthread_mutex_unlock(&mutex);
    } else {


        printf("No chairs remaining in lobby. Student %d Leaving.....\n", id);
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);
        finished++;


        if (finished == STUDENTS) {
            sem_post(&student_sem); 
        }


        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

void *st(void *arg) {
    while (1) {
        sem_wait(&student_sem); 

        pthread_mutex_lock(&mutex);
        if (finished >= STUDENTS) {
            pthread_mutex_unlock(&mutex);
            break; 
        }




        if (waiting > 0) {
            waiting--;


            printf("A waiting student started getting consultation\n");
            printf("Number of students now waiting: %d\n", waiting);
        }
        pthread_mutex_unlock(&mutex);

        printf("ST giving consultation\n");
        sem_post(&st_sem);
        sleep(1);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t st_thread, student_threads[STUDENTS];
    int ids[STUDENTS];



    sem_init(&student_sem, 0, 0);
    sem_init(&st_sem, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    srand(time(NULL));

    pthread_create(&st_thread, NULL, st, NULL);

    for (int i = 0; i < STUDENTS; i++) {
        ids[i] = i;
        pthread_create(&student_threads[i], NULL, student, &ids[i]);
        sleep(rand() % 2 + 1);
    }




    for (int i = 0; i < STUDENTS; i++)
        pthread_join(student_threads[i], NULL);

    pthread_join(st_thread, NULL);
    sem_destroy(&student_sem);
    sem_destroy(&st_sem);
    pthread_mutex_destroy(&mutex);

    return 0;
}

