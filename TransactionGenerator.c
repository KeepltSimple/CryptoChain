#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/shm.h>
#include <fcntl.h>

typedef struct transaction
{
    char id[20];
    unsigned int reward;
    unsigned int value;
    time_t timeStamp;
} transaction;

typedef struct transactionPendingSet
{
    int empty;
    int age;
    transaction currTransaction;
} transactionPendingSet;

int isCMDValid(int, unsigned int *, unsigned int *, char **);
void generateTransaction(int, pid_t, transaction *);
void sendTransaction(transaction *);
void atachToTrnsPool(transactionPendingSet **, key_t);
key_t createKey();

int main(int argc, char **argv)
{
    unsigned int reward = 0;
    unsigned int timeIntervalMs = 0;

    if (!isCMDValid(argc, &reward, &timeIntervalMs, argv))
    {
        return 0;
    }

    srand(time(NULL));
    transaction newTransaction;
    transactionPendingSet *pendingTransactions = NULL;

    key_t poolKey = createKey();

    if (poolKey == -1)
    {
        perror("ftok");
        exit(1);
    }

    atachToTrnsPool(&pendingTransactions, poolKey);
    pid_t pid;

    while (1)
    {

        pid = fork();
        if (pid == 0)
        {
            generateTransaction(reward, getpid(), &newTransaction);
            printf("ID:%s\nReward:%d\nValue:%d\nTimeStamp:%ld\n", newTransaction.id, newTransaction.reward, newTransaction.value, newTransaction.timeStamp);

            exit(0);
        }
        else
        {
            usleep(timeIntervalMs * 1000);
            wait(NULL);
        }
    }

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

key_t createKey()
{
    const char *path = "/tmp/myproject.ipc";
    int fd = open(path, O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        perror("open");
        exit(1);
    }
    close(fd);
    return ftok(path, 'A');
}

void sendTransaction(transaction *newTransaction)
{
}

void atachToTrnsPool(transactionPendingSet **pendingTransactions, key_t poolKey)
{

    int shmidPool = shmget(poolKey, 0, 0666);
    if (shmidPool == -1)
    {
        perror("shmget");
        exit(1);
    }

    *pendingTransactions = (transactionPendingSet *)shmat(shmidPool, NULL, 0);
    if (*pendingTransactions == (transactionPendingSet *)-1)
    {
        perror("shmat");
        exit(1);
    }
}
