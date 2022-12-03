//  shm_processes2.c
//  Created by Mary Nwosu and Melanie White on 11/25/22.

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define BankAccount 0
#define Turn 1

void  ChildProcess(int []);

int  main(int  argc, char *argv[]) {
     int  ShmID, *ShmPTR, status;
     int  i, account, deposit;
     pid_t  pid;
     srandom(time(NULL));

     ShmID = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
     printf("Parent has received a shared memory of two integers...\n");

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }
     printf("Parent has attached the shared memory...\n");

     ShmPTR[BankAccount] = 0; // Initialize BankAccount to 0
     ShmPTR[Turn] = 0;        // Initialize Turn to 0
     printf("Original Bank Account Balance = %d and Turn = %d.\n",
               ShmPTR[BankAccount], ShmPTR[Turn]);

     printf("Parent is about to fork a child process...\n");
     pid = fork();
     if (pid < 0) {
          printf("*** fork error (server) ***\n");
          exit(1);
     }
     else if (pid == 0) {     // Child Process
          ChildProcess(ShmPTR);
          exit(0);
     } else {                 // Parent Process
        for (i = 0; i < 25; i++) {
               sleep(random() % 6);
               while (ShmPTR[Turn] != 0);
               account = ShmPTR[BankAccount];
               if (account <= 100) {              // if Account Balance <= 100
                    deposit = random()% 101;      // random deposit between 0-100
                    if (deposit % 2 == 0) {       // if deposit is even
                         account += deposit;
                         printf("Dear Old Dad: Deposits $%d / Balance = $%d\n",
                                   deposit, account);
                    } else {                      // if deposit is odd
                         printf("Dear Old Dad: Doesn't have any money to give\n");
                    }
               } else {                           // if Account Balance > 100
                    printf("Dear Old Dad: Thinks Student has enough Cash ($%d)\n",
                              account);
            }
            ShmPTR[BankAccount] = account;
            ShmPTR[Turn] = 1;
          }
          wait(&status);
          printf("Parent has detected the completion of its child...\n");
          shmdt((void *) ShmPTR);
          printf("Parent has detached its shared memory...\n");
          shmctl(ShmID, IPC_RMID, NULL);
          printf("Parent has removed its shared memory...\n");
          printf("Parent exits...\n");
          exit(0);
     }
}

void ChildProcess(int SharedMem[]) {
    int i, account, withdrawal;
    srandom(time(NULL));

    for (i = 0; i < 25; i++) {
        sleep(random() % 6);
        while (SharedMem[Turn] != 1);
        account = SharedMem[BankAccount];
        withdrawal = random()% 51;        // random withdrawal between 0-50
        printf("    Poor Student needs $%d\n", withdrawal);
        if (withdrawal <= account) {       // if withdrawal <= Account Balance
            account -= withdrawal;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n",
                         withdrawal, account);
        } else {                           // if withdrawal > Account Balance
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }
        SharedMem[BankAccount] = account;
        SharedMem[Turn] = 0;
    }
}
