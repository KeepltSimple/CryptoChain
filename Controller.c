#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

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

key_t createKey();
void createSharedTrnsPool(transactionPendingSet **, size_t, key_t, int *);

int main()
{
    transactionPendingSet *pendingTransactions = NULL;
    size_t shmPoolSize = sizeof(transactionPendingSet) * 10;
    key_t poolKey = createKey();
    int shmidPool = 0;

    if (poolKey == -1)
    {
        perror("ftok");
        exit(1);
    }

    createSharedTrnsPool(&pendingTransactions, shmPoolSize, poolKey, &shmidPool);

    if (shmdt(pendingTransactions) == -1)
    {
        perror("shmdt");
    }

    return 0;
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

void createSharedTrnsPool(transactionPendingSet **pendingTransactions, size_t shmSize, key_t poolKey, int *shmidPool)
{
    *shmidPool = shmget(poolKey, shmSize, IPC_CREAT | 0666);
    if (*shmidPool == -1)
    {
        perror("shmget");
        exit(1);
    }

    *pendingTransactions = (transactionPendingSet *)shmat(*shmidPool, NULL, 0);
    if (*pendingTransactions == (transactionPendingSet *)-1)
    {
        perror("shmat");
        exit(1);
    }

    memset(*pendingTransactions, 0, shmSize);
    shmctl(*shmidPool, IPC_RMID, NULL);
}
