#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    if (argc < 2){
        printf("Please provide numbers as command line arguments.\n");
        return 1;
    }

    int n = argc -1;
    int arr[n];

    for (int i = 0; i < n; i++){
        arr[i] = atoi(argv[i+1]);
    }

    pid_t pid = fork();

    if (pid == 0){
            for(int i = 0; i<n; i++){
            for(int j = 0; j<n-i-1; j++){
                if(arr[j]<arr[j+1]){
                    int temp = arr[j];
                    arr[j] = arr[j+1];
                    arr[j+1] = temp;
                }
            }
        }

        printf("Sorted Array in Decending Order\n");
        for(int k = 0; k < n; k++){
            printf("%d, ", arr[k]);
        }
        printf("\n");

        FILE *fp = fopen("sort.txt", "w");
        for (int i = 0; i < n; i++){
            fprintf(fp, "%d ", arr[i]);
        }
        fclose(fp);
        exit(0);
    }else{
        wait(NULL);

        FILE *fp = fopen("sort.txt", "r");
        
        int num;
        printf("ODD or EVEN CHECK\n");

        while(fscanf(fp, "%d", &num)==1){
            if (num % 2 == 0){
                printf("%d is EVEN\n", num);
            }else{
                printf("%d is ODD\n", num);
            }

        }
        fclose(fp);
    }
    return 0;
}