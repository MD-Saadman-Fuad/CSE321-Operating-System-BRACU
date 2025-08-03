// This program opens a file, writes a message, seeks to the start, reads it back, and prints it.

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    int fd;
    char buffer[80];
    static char message[] = "Hello, World";

    fd = open("myfile.txt", O_RDWR | O_CREAT, 0644);  // Create file if it doesn't exist

    if (fd != -1) {
        printf("myfile.txt opened for read/write access\n");

        write(fd, message, sizeof(message));
        lseek(fd, 0, SEEK_SET);  // Go back to the beginning of file
        read(fd, buffer, sizeof(message));

        printf("%s was written to myfile.txt\n", buffer);
        close(fd);
    } else {
        perror("open failed");
    }

    return 0;
}
