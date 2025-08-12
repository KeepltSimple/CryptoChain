#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ipc.h>

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
    if (key == 1)
    {
        return 0;
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
        return 1;
    }

    return ftok(path, projId);
}
