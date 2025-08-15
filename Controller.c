
#include <string.h>
#include <semaphore.h>
#include "TransacPool.h"

void createSharedTrnsPool(transactionPendingSet **, size_t, key_t, int *);

int main()
{
    transactionPendingSet *pendingTransactions = NULL;
    size_t shmPoolSize = sizeof(transactionPendingSet) * 10;
    key_t poolKey = createPoolKey();
    int shmidPool = 0;
    sem_t *poolSem;
    char *name = POOL_SEMA;
    poolSem = sem_open(name, O_CREAT, 0666, 1);

    if (poolKey == -1)
    {
        perror("ftok");
        exit(1);
    }

    createSharedTrnsPool(&pendingTransactions, shmPoolSize, poolKey, &shmidPool);
    sem_post(poolSem);
    sleep(5);
    sem_wait(poolSem);
    if (shmdt(pendingTransactions) == -1)
    {
        perror("shmdt");
    }
    shmctl(shmidPool, IPC_RMID, NULL);
    sem_close(poolSem);
    sem_unlink(name);
    return 0;
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
}
