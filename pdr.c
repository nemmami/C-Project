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
        printf("%s\n", "Usage argv[0] clientID monttant");
        exit(1);
    }

    int numClient = atoi(argv[1]);
    int montant = atoi(argv[2]);

    if (numClient > 1000 || numClient < 0)
    {
        printf("Numéro de client n'existe pas");
        exit(1);
    }

    // semaphore
    sshmget(LLN_SHM_KEY, 2 * sizeof(int), IPC_CREAT | PERM);

    
    printf("%d", montant);
}