#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "message.h"
#include "utils_v1.h"

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3)
    {
        printf(stderr, "Usage argv[0] type (opt)");
        exit(1);
    }

    int type = atoi(argv[1]);

    if (type == 1)
    {
        int shm_id = shmget(TRAIN_SHM_KEY, 2 * sizeof(pid_t), IPC_CREAT | IPC_EXCL | PERM);
        checkNeg(shm_id, "IPCs already created");

        sem_create(TRAIN_SEM_KEY, 1, PERM, 0);

        printf("IPCs created.\n");
    }
    else if (type == 2)
    {
        printf("Destroying IPCs...\n");
        int shm_id = shmget(TRAIN_SHM_KEY, 2 * sizeof(pid_t), 0);
        checkNeg(shm_id, "IPCs not existing");

        sshmdelete(shm_id);

        int sem_id = sem_get(TRAIN_SEM_KEY, 1);
        sem_delete(sem_id);

        printf("IPCs freed.\n");
    }
    else if (type == 3)
    {
    }
    else
    {
    }
}
