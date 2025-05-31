#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int processCount = 1;

void createExtraIfOdd() {
    pid_t pid = getpid();
    if (pid % 2 != 0) {
        pid_t extra = fork();
        if (extra == 0) {
            processCount = 0;
            exit(0);
        } else if (extra > 0) {
            processCount++;
            wait(NULL);
        }
    }
}

int main() {
    pid_t a = fork();
    if (a == 0) {
        createExtraIfOdd();
    } else {
        wait(NULL);  
    }

    pid_t b = fork();
    if (b == 0) {
        createExtraIfOdd();
    } else {
        wait(NULL); 

    pid_t c = fork();
    if (c == 0) {
        createExtraIfOdd();
    } else {
        wait(NULL);
    }

    wait(NULL);  
    printf("Total number of processes: %d\n", processCount);
    return 0;
}
