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
    if (argc != 3)
    {
        printf("%s\n", "Usage argv[0] numeroCompte montant");
        exit(1);
    }

    int numClient = atoi(argv[1]);
    int montant = atoi(argv[2]);

    if (numClient > 1000 || numClient < 0)
    {
        printf("Numero de client inexistant");
        exit(1);
    }

    int id = sshmget(SHM_KEY, 1000 * sizeof(int), 0);
    int *tab = sshmat(id);

    int semId = sem_get(SEM_KEY, 1);

    sem_down0(semId);
    // debut zone critique
    tab[numClient] += montant;
    printf("Nouveau solde du compte %d : %d\n", numClient, tab[numClient]);
    // fin zone critique
    sem_up0(semId);

    sshmdt(tab);

    // sshmdelete(id);

    exit(EXIT_SUCCESS);
}