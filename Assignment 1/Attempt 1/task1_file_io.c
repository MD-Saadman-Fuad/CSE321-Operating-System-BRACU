#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("File open error");
        return 1;
    }

    char buffer[1024];
    while (1) {
        printf("Enter a string (-1 to exit): ");
        fgets(buffer, sizeof(buffer), stdin);
        if (strncmp(buffer, "-1", 2) == 0)
            break;
        write(fd, buffer, strlen(buffer));
    }

    close(fd);
    return 0;
}