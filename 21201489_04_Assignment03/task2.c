#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define MAX_LEN 10

struct message {
    long type;
    char content[MAX_LEN];
};

void run_otp_gen(int qid);
void run_mail_proc(int qid, const char* otp);

int main() {
    key_t key = ftok("otpfile", 65);
    int qid = msgget(key, 0666 | IPC_CREAT);

    char input[50];
    printf("Please enter the workspace name:\n");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    if (strcmp(input, "cse321") != 0) {
        printf("Invalid workspace name\n");
        msgctl(qid, IPC_RMID, NULL);
        return 0;
    }

    struct message msg;
    msg.type = 100;
    strncpy(msg.content, input, MAX_LEN);
    msgsnd(qid, &msg, sizeof(msg.content), 0);
    printf("Workspace name sent to OTP generator from log in: %s\n\n", msg.content);

    pid_t otp_pid = fork();
    if (otp_pid == 0) {
        run_otp_gen(qid);
        exit(0);
    }

    wait(NULL);

    struct message recv1, recv2;
    msgrcv(qid, &recv1, sizeof(recv1.content), 200, 0);
    printf("Log in received OTP from OTP generator: %s\n", recv1.content);

    msgrcv(qid, &recv2, sizeof(recv2.content), 300, 0);
    printf("Log in received OTP from mail: %s\n", recv2.content);

    if (strcmp(recv1.content, recv2.content) == 0) {
        printf("OTP Verified\n");
    } else {
        printf("OTP Incorrect\n");
    }

    msgctl(qid, IPC_RMID, NULL);
    return 0;
}

void run_otp_gen(int qid) {
    struct message incoming;
    msgrcv(qid, &incoming, sizeof(incoming.content), 100, 0);
    printf("OTP generator received workspace name from log in: %s\n\n", incoming.content);

    char otp_str[MAX_LEN];
    snprintf(otp_str, MAX_LEN, "%d", getpid());

    struct message otp_msg;
    otp_msg.type = 200;
    strncpy(otp_msg.content, otp_str, MAX_LEN);
    msgsnd(qid, &otp_msg, sizeof(otp_msg.content), 0);
    printf("OTP sent to log in from OTP generator: %s\n", otp_msg.content);

    otp_msg.type = 150;
    msgsnd(qid, &otp_msg, sizeof(otp_msg.content), 0);
    printf("OTP sent to mail from OTP generator: %s\n\n", otp_msg.content);

    pid_t mail_pid = fork();
    if (mail_pid == 0) {
        run_mail_proc(qid, otp_str);
        exit(0);
    } else {
        wait(NULL);
    }
}

void run_mail_proc(int qid, const char* otp) {
    struct message mail_in;
    msgrcv(qid, &mail_in, sizeof(mail_in.content), 150, 0);
    printf("Mail received OTP from OTP generator: %s\n", mail_in.content);

    struct message mail_out;
    mail_out.type = 300;
    strncpy(mail_out.content, otp, MAX_LEN);
    msgsnd(qid, &mail_out, sizeof(mail_out.content), 0);
    printf("OTP sent to log in from mail: %s\n", mail_out.content);
}
