#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    printf("argc = %d\n", argc);           // Print argument count

    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);  // Print each argument
    }

    return 0;
}
