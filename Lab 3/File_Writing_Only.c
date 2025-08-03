#include <stdio.h>
#include <string.h>

int main() {
    FILE *fp1, *fp2;
    // char line[20];
    int num1, num2;

    fp1 = fopen("text1.txt", "r");
    fgets(line, sizeof(line), fp1);
    printf("\n%s\n", line);
    fscanf(fp1, "%d%d", &num1, &num2);
    printf("\n%d\n", num1);
    printf("\n%d\n", num2);
    fclose(fp1);

    fp2 = fopen("text2.txt", "w");
    fputs("hello\n", fp2);
    fprintf(fp2, "the number is %d", 12);
    fclose(fp2);

    return 0;
}
