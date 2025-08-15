#ifndef TRANSACPOOL_H
#define TRANSACPOOL_H

#include <time.h>
#include <sys/ipc.h>

#define POOL_SEMA "poolCreated"

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

key_t createPoolKey(void);
size_t getPoolSize(key_t poolKey);
void atachToTrnsPool(transactionPendingSet **pendingTransactions, key_t poolKey);

#endif
