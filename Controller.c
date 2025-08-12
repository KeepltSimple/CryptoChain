#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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

key_t createKey(char);

int main()
{

    key_t key = createKey('A');
    char *data = NULL;
    int shmid = 0;
    if (key == -1)
    {
        perror("ftok");
        exit(1);
    }
    shmid = shmget(key, 10, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }

    data = (char *)shmat(shmid, NULL, 0);
    if (data == (char *)-1)
    {
        perror("shmat child");
        exit(1);
    }

    if (shmdt(data) == -1)
    {
        perror("shmdt child");
        exit(1);
    }
    return 0;
}

key_t createKey(char projId)
{
    const char *path = "/tmp/myproject.ipc";
    int fd = open(path, O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        perror("open");
        exit(1);
    }

    return ftok(path, projId);
}
