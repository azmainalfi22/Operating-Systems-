#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

struct shared {
    char sel[100];
    int balance;
};

void addFunds(struct shared *data);
void withdrawFunds(struct shared *data);
void checkBalance(const struct shared *data);

int main() {
    key_t shm_key = 2025;
    int shm_id = shmget(shm_key, sizeof(struct shared), IPC_CREAT | 0666);
    struct shared *shared = (struct shared *)shmat(shm_id, NULL, 0);

    int pipe_fd[2];
    pipe(pipe_fd);

    shared->balance = 1000;

    printf("Please choose an option:\n");
    printf("1. Type 'a' to Add Money\n");
    printf("2. Type 'w' to Withdraw Money\n");
    printf("3. Type 'c' to Check Balance\n");

    fgets(shared->sel, sizeof(shared->sel), stdin);
    shared->sel[strcspn(shared->sel, "\n")] = '\0';  // Remove newline

    printf("Your selection: %s\n", shared->sel);

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }

    if (pid == 0) {
        // Child process - performs operations
        close(pipe_fd[0]); // Close read end

        if (strcmp(shared->sel, "a") == 0) {
            addFunds(shared);
        } else if (strcmp(shared->sel, "w") == 0) {
            withdrawFunds(shared);
        } else if (strcmp(shared->sel, "c") == 0) {
            checkBalance(shared);
        } else {
            printf("Invalid selection\n");
        }

        const char *message = "Thank you for using Azmain Bank\n";
        write(pipe_fd[1], message, strlen(message) + 1);
        close(pipe_fd[1]);
        exit(0);
    } else {
        // Parent process - waits and prints final message
        close(pipe_fd[1]); // Close write end
        wait(NULL);

        char buffer[100];
        read(pipe_fd[0], buffer, sizeof(buffer));
        printf("%s", buffer);

        close(pipe_fd[0]);
        shmctl(shm_id, IPC_RMID, NULL); // Clean up shared memory
    }

    return 0;
}

void addFunds(struct shared *data) {
    int amount;
    printf("Enter amount to add:\n");
    scanf("%d", &amount);

    if (amount > 0) {
        data->balance += amount;
        printf("Balance added successfully.\n");
        printf("Updated balance: %d\n", data->balance);
    } else {
        printf("Adding failed, Invalid amount\n");
    }
}

void withdrawFunds(struct shared *data) {
    int amount;
    printf("Enter amount to withdraw:\n");
    scanf("%d", &amount);

    if (amount > 0 && amount <= data->balance) {
        data->balance -= amount;
        printf("Balance withdrawn successfully.\n");
        printf("Updated balance: %d\n", data->balance);
    } else {
        printf("Withdrawal failed, Invalid amount\n");
    }
}

void checkBalance(const struct shared *data) {
    printf("Your current balance is: %d\n", data->balance);
}
