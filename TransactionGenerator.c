#include "TransacPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <semaphore.h>

int isCMDValid(int, unsigned int *, unsigned int *, char **);
void generateTransaction(int, pid_t, transaction *);
void sendTransaction(transaction *, int);

int main(int argc, char **argv)
{

    sem_t *poolSem = NULL;
    int poolLength = 0;
    poolSem = sem_open(POOL_SEMA, 0);

    if (poolSem == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    unsigned int reward = 0;
    unsigned int timeIntervalMs = 0;

    if (!isCMDValid(argc, &reward, &timeIntervalMs, argv))
    {
        return 0;
    }

    srand(time(NULL));

    transaction newTransaction;
    transactionPendingSet *pendingTransactions = NULL;

    key_t poolKey = createPoolKey();

    if (poolKey == -1)
    {
        perror("ftok");
        exit(1);
    }

    sem_wait(poolSem);
    atachToTrnsPool(&pendingTransactions, poolKey);
    poolLength = getPoolSize(poolKey) / sizeof(transactionPendingSet);

    pid_t pid;

    while (1)
    {

        pid = fork();
        if (pid == 0)
        {
            generateTransaction(reward, getpid(), &newTransaction);
            printf("ID:%s\nReward:%d\nValue:%d\nTimeStamp:%ld\n", newTransaction.id, newTransaction.reward, newTransaction.value, newTransaction.timeStamp);
            // send transaction

            exit(0);
        }
        else
        {
            usleep(timeIntervalMs * 1000);
            wait(NULL);
        }
    }

    sem_close(poolSem);

    return 0;
}

int isCMDValid(int argc, unsigned int *reward, unsigned int *timeIntervalMs, char **argv)
{

    char *endptr;

    if (argc < 3)
    {
        printf("%d arguments, inputed must have 3 arguments\n", argc);
        return 0;
    }
    *reward = strtol(argv[1], &endptr, 10);

    if (*reward < 1 || *reward > 3 || *endptr != '\0')
    {
        printf("Rewards must be between 1 and 3\n");
        return 0;
    }

    *timeIntervalMs = strtol(argv[2], &endptr, 10);

    if (*timeIntervalMs < 200 || *timeIntervalMs > 3000 || *endptr != '\0')
    {
        printf("Time interval must be between 200 and 3000\n");
        return 0;
    }

    return 1;
}

void generateTransaction(int reward, pid_t pid, transaction *newTransaction)
{

    newTransaction->reward = reward;
    newTransaction->timeStamp = time(NULL);
    newTransaction->value = rand() % 2000 + 1;
    snprintf(newTransaction->id, sizeof(newTransaction->id), "%d-%ld", pid, newTransaction->timeStamp);
}
