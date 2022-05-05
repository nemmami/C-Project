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
        printf("Usage argv[0] type (opt)");
        exit(1);
    }

    int type = atoi(argv[1]);

    if (type == 1)
    {
        // creation memoire partagé
        sshmget(SHM_KEY, 1000 * sizeof(int), IPC_CREAT | IPC_EXCL | PERM);

        // creation semapgore
        sem_create(SEM_KEY, 1, PERM, 1);

        printf("IPCs created.");
    }
    else if (type == 2)
    {
        printf("Destroying IPCs...\n");
        // get identifiant de la memoire partagé
        int shm_id = sshmget(SHM_KEY, 1000 * sizeof(int), 0);
        // get id du sem
        int sem_id = sem_get(SEM_KEY, 1);
        // suprime la memoire partagé
        sshmdelete(shm_id);
        // supprime le semaphore
        sem_delete(sem_id);

        printf("IPCs freed.\n");
    }
    else if (type == 3)
    {
        if (argc != 3)
        {
            printf("La duree n'est pas donnée");
            exit(1);
        }
        int opt = atoi(argv[2]);
        int sem_id = sem_get(SEM_KEY, 1);

        sem_down0(sem_id);
        sleep(opt);
        sem_up0(sem_id);
    }
    else
    {
        printf("Type incorrect !");
        exit(0);
    }

    return 0;
}
