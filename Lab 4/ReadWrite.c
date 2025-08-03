#include <fcntl.h>    // for open()
#include <unistd.h>   // for write(), read(), lseek(), close()
#include <stdio.h>    // for printf()

int main() {
    int fd;                                // File descriptor
    char buffer[80];                       // Buffer to read into
    static char message[] = "Hello, world"; // Message to write

    fd = open("myfile", O_RDWR);           // Open file for read/write
    if (fd != -1) {
        printf("myfile opened for read/write access\n");

        write(fd, message, sizeof(message));  // Write message to file
        lseek(fd, 0, 0);                      // Move file pointer to beginning
        read(fd, buffer, sizeof(message));    // Read data back into buffer

        printf("%s was written to myfile\n", buffer);  // Print buffer content

        close(fd);                            // Close file
    }
}
