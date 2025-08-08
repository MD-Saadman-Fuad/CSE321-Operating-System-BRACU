#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2){
         printf("Please provide numbers as command line arguments.\n");
        return 1;
    }

    printf("ODD or EVEN ?\n");
    for (int i = 1; i<argc; i++){
        int num = atoi(argv[i]);

        if (num % 2 == 0){
            printf("%d is EVEN\n", num);
        }else{
            printf("%d is ODD\n", num);
        }
    }
    return 0;
}