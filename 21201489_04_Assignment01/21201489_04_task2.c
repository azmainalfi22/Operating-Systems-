#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid1 = fork();

    if (pid1 == 0) {
        pid_t pid2 = fork();

        if (pid2 == 0) {
            printf("I am grandchild\n");
        } else if (pid2 > 0) {
            wait(NULL);
            printf("I am child\n");
        }

        return 0;
    } else if (pid1 > 0) {
        wait(NULL);
        printf("I am parent\n");
    }

    return 0;
}
