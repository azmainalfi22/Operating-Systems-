#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    pid_t childProcessId, grandchildProcessId1, grandchildProcessId2, grandchildProcessId3;

    printf("1. Parent process ID: %d\n", getpid());

    // Creating the child process
    childProcessId = fork();

    if (childProcessId == 0) { 
        printf("2. Child process ID: %d\n", getpid());
       
        grandchildProcessId1 = fork();
        if (grandchildProcessId1 == 0) {
            printf("3. Grandchild process ID: %d\n", getpid());
        } else {
            wait(NULL); 
        }

        grandchildProcessId2 = fork();
        if (grandchildProcessId2 == 0) {
            printf("4. Grandchild process ID: %d\n", getpid());
        } else {
            wait(NULL);  
        }

        grandchildProcessId3 = fork();
        if (grandchildProcessId3 == 0) {
            printf("5. Grandchild process ID: %d\n", getpid());
        } else {
            wait(NULL);  
        }
    } else {
        wait(NULL);
    }

    return 0;
}
