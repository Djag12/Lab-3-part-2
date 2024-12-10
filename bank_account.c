#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_ACCOUNT 100
#define MAX_WITHDRAW 50
#define NUM_ITERATIONS 25

struct SharedData {
    int BankAccount;
    int Turn;
};

int main() {
    int shmid = shmget(IPC_PRIVATE, sizeof(struct SharedData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    struct SharedData *data = (struct SharedData *)shmat(shmid, NULL, 0);
    if (data == (struct SharedData *)(-1)) {
        perror("shmat");
        exit(1);
    }

    data->BankAccount = 0;
    data->Turn = 0;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    srand(time(NULL));

    if (pid > 0) {
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            sleep(rand() % 6);
            int account = data->BankAccount;

            while (data->Turn != 0) {}

            if (account <= MAX_ACCOUNT) {
                int balance = rand() % 101;
                if (balance % 2 == 0) {
                    data->BankAccount += balance;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, data->BankAccount);
                } else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }
            } else {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
            }

            data->Turn = 1;
        }
    } else {
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            sleep(rand() % 6);
            int account = data->BankAccount;

            while (data->Turn != 1) {}

            int balance_needed = rand() % (MAX_WITHDRAW + 1);
            printf("Poor Student needs $%d\n", balance_needed);

            if (balance_needed <= account) {
                data->BankAccount -= balance_needed;
                printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance_needed, data->BankAccount);
            } else {
                printf("Poor Student: Not Enough Cash ($%d)\n", account);
            }

            data->Turn = 0;
        }
    }

    shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
