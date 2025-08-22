#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msg {
    long int type;
    char txt[6];
};

int main() {
    int msg_id = msgget((key_t)102, 0666 | IPC_CREAT);

    printf("Please enter the workspace name:\n");
    char workspace[10];
    scanf("%s", workspace);

    if (strcmp(workspace, "cse321") != 0) {
        printf("Invalid workspace name\n");
        msgctl(msg_id, IPC_RMID, 0);
        return 0;
    }

    struct msg workspace_msg;
    workspace_msg.type = 1;
    strcpy(workspace_msg.txt, workspace);
    msgsnd(msg_id, &workspace_msg, sizeof(workspace_msg.txt), 0);
    printf("Workspace name sent to otp generator from log in: %s\n", workspace);

    pid_t otp_pid = fork();
   
    if (otp_pid == 0) {
        struct msg received_workspace;

        msgrcv(msg_id, &received_workspace, sizeof(received_workspace.txt), 1, 0);


        printf("OTP generator received workspace name from log in: %s\n", received_workspace.txt);

        int otp = getpid();
        char otp_str[6];
        snprintf(otp_str, sizeof(otp_str), "%d", otp);

        struct msg otp_login_msg;
        otp_login_msg.type = 2;
        strcpy(otp_login_msg.txt, otp_str);

        msgsnd(msg_id, &otp_login_msg, sizeof(otp_login_msg.txt), 0);

        printf("OTP sent to log in from OTP generator: %s\n", otp_str);

        struct msg otp_mail_msg;
        otp_mail_msg.type = 3;


        strcpy(otp_mail_msg.txt, otp_str);
        msgsnd(msg_id, &otp_mail_msg, sizeof(otp_mail_msg.txt), 0);
        printf("OTP sent to mail from OTP generator: %s\n", otp_str);

        pid_t mail_pid = fork();
       
        if (mail_pid == 0) {
            struct msg received_otp;
            msgrcv(msg_id, &received_otp, sizeof(received_otp.txt), 3, 0);

            printf("Mail received OTP from OTP generator: %s\n", received_otp.txt);

            struct msg mail_login_msg;
            mail_login_msg.type = 4;
            strcpy(mail_login_msg.txt, received_otp.txt);
            msgsnd(msg_id, &mail_login_msg, sizeof(mail_login_msg.txt), 0);
            printf("OTP sent to log in from mail: %s\n", received_otp.txt);
           
            exit(0);
        }
        else {
            wait(NULL);
            exit(0);
        }
    }
    else {
        wait(NULL);

        struct msg otp_from_generator;
        msgrcv(msg_id, &otp_from_generator, sizeof(otp_from_generator.txt), 2, 0);


        printf("Log in received OTP from OTP generator: %s\n", otp_from_generator.txt);

        struct msg otp_from_mail;
        msgrcv(msg_id, &otp_from_mail, sizeof(otp_from_mail.txt), 4, 0);
        
        printf("Log in received OTP from mail: %s\n", otp_from_mail.txt);

        if (strcmp(otp_from_generator.txt, otp_from_mail.txt) == 0) {
            printf("OTP Verified\n");
        } else {
            printf("OTP Incorrect\n");
        }

        msgctl(msg_id, IPC_RMID, 0);
    }

    return 0;
}
