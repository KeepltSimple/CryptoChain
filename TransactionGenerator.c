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
#include <signal.h>
#include <semaphore.h>

volatile sig_atomic_t sigintReceived = 0;

int isCMDValid(int, unsigned int *, unsigned int *, char **);
void generateTransaction(int, pid_t, transaction *);
void sendTransaction(transaction *, int, transactionPendingSet *);
void sigintHandler(int);

int main(int argc, char **argv)
{
    sigset_t blockAllset;
    sigfillset(&blockAllset);
    sigprocmask(SIG_SETMASK, &blockAllset, NULL);

    sigset_t sigintSet;
    sigemptyset(&sigintSet);
    sigaddset(&sigintSet, SIGINT);

    struct sigaction sa;
    sa.sa_handler = sigintHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);

    sigprocmask(SIG_UNBLOCK, &sigintSet, NULL);

    pid_t pid;
    int poolLength = 0;
    unsigned int reward = 0;
    unsigned int timeIntervalMs = 0;
    transaction newTransaction;
    transactionPendingSet *pendingTransactions = NULL;
    sem_t *poolSem = NULL;

    key_t poolKey = 0;

    poolKey = createPoolKey();
    if (poolKey == -1)
    {
        perror("ftok");
        exit(1);
    }

    poolSem = sem_open(POOL_SEMA, 0);

    if (poolSem == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    if (!isCMDValid(argc, &reward, &timeIntervalMs, argv))
    {
        return 0;
    }

    srand(time(NULL));

    atachToTrnsPool(&pendingTransactions, poolKey);
    poolLength = getPoolSize(poolKey) / sizeof(transactionPendingSet);

    while (!sigintReceived)
    {

        pid = fork();
        if (pid == 0)
        {

            generateTransaction(reward, getpid(), &newTransaction);

            sigprocmask(SIG_BLOCK, &sigintSet, NULL);
            sendTransaction(&newTransaction, poolLength, pendingTransactions);
            sigprocmask(SIG_UNBLOCK, &sigintSet, NULL);
            exit(0);
        }
        else
        {

            usleep(timeIntervalMs * 1000);
            wait(NULL);
        }
    }

    if (shmdt(pendingTransactions) == -1)
    {
        perror("shmdt");
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

void sendTransaction(transaction *newTransaction, int poolLimit, transactionPendingSet *pendingTransactions)
{

    for (int i = 0; i < poolLimit; i++)
    {
        if (!pendingTransactions[i].empty)
        {
            strcpy(pendingTransactions[i].currTransaction.id, newTransaction->id);
            pendingTransactions[i].currTransaction.reward = newTransaction->reward;
            pendingTransactions[i].currTransaction.timeStamp = newTransaction->timeStamp;
            pendingTransactions[i].currTransaction.value = newTransaction->value;
            pendingTransactions[i].empty = 1;

            break;
        }
    }
}

void sigintHandler(int sig)
{
    sigintReceived = 1;
}
