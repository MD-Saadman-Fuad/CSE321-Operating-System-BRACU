
#include <stdio.h>
#include <string.h>

// Function to calculate sum
int sum(int a, int b) {
    int total = a + b;
    return total;
}

// Function to swap two integers using pointers
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Struct definition
struct student {
    char name[20];
    int roll;
};

int main() {

    // 1. Fixed string assignment and fgets
    char line1[20];
    strcpy(line1, "CSE321");
    fgets(line1, sizeof(line1), stdin);
    printf("line = %s, size = %lu, length = %lu\n", line1, sizeof(line1), strlen(line1));

    // 2. User string input using fgets
    char buff[100];
    int n = 10;
    printf("Enter a String: \n");
    fgets(buff, n, stdin);
    printf("You have entered: %s\n", buff);

    // 3. Pointer assignment and dereferencing
    int *ptr, num = 5;
    printf("num = %d\n", num);
    printf("Location of num = %p\n", (void*)&num);
    ptr = &num;
    *ptr = 10;
    printf("ptr = %p\n", (void*)ptr);
    printf("Value using ptr = %d\n", *ptr);
    printf("Updated num = %d\n", num);

    // 4. Sum function demonstration
    int num1 = 5, num2 = 10;
    int final = sum(num1, num2);
    printf("Sum = %d\n", final);

    // 5. Swap function demonstration
    printf("Before Swap: num1 = %d (at %p), num2 = %d (at %p)\n", num1, (void*)&num1, num2, (void*)&num2);
    swap(&num1, &num2);
    printf("After Swap: num1 = %d (at %p), num2 = %d (at %p)\n", num1, (void*)&num1, num2, (void*)&num2);

    // 6. Struct usage
    struct student s1;
    s1.roll = 20;
    strcpy(s1.name, "xyz");
    printf("name = %s, roll = %d\n", s1.name, s1.roll);

    // 7. ASCII manipulation using char arithmetic
    char c = 'a';
    char result = c + 2;
    printf("ASCII of 'a' + 2 = %d\n", result);  // Should print 99

    return 0;
}
