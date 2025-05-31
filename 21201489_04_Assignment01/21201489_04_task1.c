#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("You have not provided the file name\n");
        return 1;
    }

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (fd < 0) {
        perror("Failed to open file");
        return 1;
    }

    char buffer[100];
    while (1) {
        printf("Please enter a string: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }
        if (strcmp(buffer, "-1\n") == 0) {
            break;
        }
        write(fd, buffer, strlen(buffer));
    }

    close(fd);
    return 0;
}

