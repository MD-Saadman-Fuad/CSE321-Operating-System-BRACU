#include <stdio.h>
#include <string.h>

int main() {
    FILE *fp1, *fp2;
    char line[20];
    int num1, num2;

    fp1 = fopen("text1.txt", "r");
    fgets(line, sizeof(line), fp1);         // Read one line from file
    printf("\n%s\n", line);                 // Print the line

    fscanf(fp1, "%d%d", &num1, &num2);      // Read two integers
    printf("\n%d\n", num1);                 // Print first number
    printf("\n%d\n", num2);                 // Print second number
    fclose(fp1);

    fp2 = fopen("text2.txt", "w");          // Open file for writing
    fputs("hello\n", fp2);                  // Write string to file
    fprintf(fp2, "the number is %d", 12);   // Write formatted number
    fclose(fp2);

    return 0;
}
