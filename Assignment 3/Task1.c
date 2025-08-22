#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>

struct shared {
    char sel[100];
    int b;
};

int main() {
    int pipe_file_descriptors[2];
    pipe(pipe_file_descriptors);

    int shared_memory_id = shmget((key_t)101, sizeof(struct shared), 0666 | IPC_CREAT);
    void *shared_memory_pointer = shmat(shared_memory_id, NULL, 0);
    struct shared shared_data = *(struct shared *)shared_memory_pointer;
   
    shared_data.b = 1000;

    printf("Provide Your Input From Given Options:\n");
    printf("1. Type a to Add Money\n");
    printf("2. Type w to Withdraw Money\n");
    printf("3. Type c to Check Balance\n");

    char user_input;
    scanf(" %c", &user_input);
    strcpy(shared_data.sel, &user_input);
    printf("Your selection: %c\n", user_input);


    *(struct shared *)shared_memory_pointer = shared_data;

    pid_t child_process_id = fork();
   
    if (child_process_id == 0) {
        close(pipe_file_descriptors[0]);
       
        // Read from shared memory again
        struct shared child_shared_data = *(struct shared *)shared_memory_pointer;
       
        if (strcmp(child_shared_data.sel, "a") == 0) {
            printf("Enter amount to be added:\n");
            int amount_to_add;
            scanf("%d", &amount_to_add);
           
            if (amount_to_add > 0) {
                child_shared_data.b += amount_to_add;
                printf("Balance added successfully\n");
                printf("Updated balance after addition:\n%d\n", child_shared_data.b);
            } else {
                printf("Adding failed, Invalid amount\n");
            }
        }
        else if (strcmp(child_shared_data.sel, "w") == 0) {
            printf("Enter amount to be withdrawn:\n");
            int amount_to_withdraw;
            scanf("%d", &amount_to_withdraw);
           
            if (amount_to_withdraw > 0 && amount_to_withdraw <= child_shared_data.b) {
                child_shared_data.b -= amount_to_withdraw;
                printf("Balance withdrawn successfully\n");
                printf("Updated balance after withdrawal:\n%d\n", child_shared_data.b);
            } else {
                printf("Withdrawal failed, Invalid amount\n");
            }
        }
        else if (strcmp(child_shared_data.sel, "c") == 0) {
            printf("Your current balance is:\n%d\n", child_shared_data.b);
        }
        else {
            printf("Invalid selection\n");
        }
       
        
        *(struct shared *)shared_memory_pointer = child_shared_data;
       
        char thank_you_message[] = "Thank you for using";
        write(pipe_file_descriptors[1], thank_you_message, sizeof(thank_you_message));
        close(pipe_file_descriptors[1]);
        shmdt(shared_memory_pointer);
        exit(0);
    }
    else if (child_process_id > 0) {
        close(pipe_file_descriptors[1]);
        wait(NULL);
       
        char received_message[100];
        read(pipe_file_descriptors[0], received_message, sizeof(received_message));
        printf("%s\n", received_message);
       
        close(pipe_file_descriptors[0]);
        shmdt(shared_memory_pointer);
        shmctl(shared_memory_id, IPC_RMID, NULL);
    }
    else {
        perror("fork failed");
        exit(1);
    }
   
    return 0;
}