#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main (int argc, char *argv[]){
    FILE *fp;
    char str[101];


    if (argc < 2){
        printf("Please Provide file name that will be created along the reference(./<ref> <filename.txt>): ");
        return 1; //Meaning program is facing error
    }


    fp = fopen(argv[1], "a"); //accessing 2nd file of array , a means append, will create new file if not found
    if (fp==NULL){
        printf("Error opening file!!!!!\n");
        return 1;
    }


    while(1){
        printf("Enter a String (type -1 to stop input): ");


        fgets(str, sizeof(str), stdin);
        str[strcspn(str, "\n")] = '\0';


        if (strcmp(str, "-1") == 0){
            break;
        }


        fprintf(fp, "%s\n", str);
   
    }


    fclose(fp);
    printf("Inputs are written successfully");
    return 0;
}