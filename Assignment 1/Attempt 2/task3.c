#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/mman.h>

int main() {
    pid_t a, b, c, x;
    pid_t original_parent = getpid();

    int *process_count = mmap(NULL, sizeof *process_count, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *process_count = 0;

    a = fork();
    b = fork();
    c = fork();

    if (getpid() % 2 != 0){
        x = fork();
    }

    (*process_count)++;

    while(wait(NULL)>0);

    if(getpid() == original_parent){
        printf("Total processes created (including parent): %d\n", *process_count);
    }
    return 0;
}