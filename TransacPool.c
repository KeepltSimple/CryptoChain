#include "TransacPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>

key_t createPoolKey(void)
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

size_t getPoolSize(key_t poolKey)
{
    struct shmid_ds shmInfo;
    int shmidPool = shmget(poolKey, 0, 0666);
    if (shmidPool == -1)
    {
        perror("shmget");
        exit(1);
    }
    if (shmctl(shmidPool, IPC_STAT, &shmInfo) == -1)
    {
        perror("shmctl");
        exit(1);
    }
    return shmInfo.shm_segsz;
}

void atachToTrnsPool(transactionPendingSet **pendingTransactions, key_t poolKey)
{
    int shmidPool = shmget(poolKey, 0, 0666);
    if (shmidPool == -1)
    {
        perror("shmget");
        exit(1);
    }
    *pendingTransactions = shmat(shmidPool, NULL, 0);
    if (*pendingTransactions == (transactionPendingSet *)-1)
    {
        perror("shmat");
        exit(1);
    }
}
