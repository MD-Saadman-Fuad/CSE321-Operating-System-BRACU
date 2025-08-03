#include <stdio.h>
#include <string.h>

int is_even(int a) {
    if (a % 2 == 0) {
        return 1;
    } else {
        return 0;
    }
}

int main() {
    char word[100];
    int num;

    scanf("%s", word);
    scanf("%d", &num); // Use & to take address

    int call = is_even(num); // declare variable correctly

    if (call == 1) {
        for (int k = 0; k < strlen(word); k++) {
            if (word[k] >= 'a' && word[k] <= 'z') {
                word[k] = word[k] - 32; // convert to uppercase
            }
        }
        printf("%s\n", word);
    } else {
        printf("No changes needed\n");
    }

    return 0;
}
