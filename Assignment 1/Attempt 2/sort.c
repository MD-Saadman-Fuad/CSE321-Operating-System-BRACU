//Using Bubble sort 

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    if (argc < 2){
        printf("Please provide numbers as command line arguments.\n");
        return 1;
    }

    int n = argc -1;
    int arr[n];

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

    return 0;
}