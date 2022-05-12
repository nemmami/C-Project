
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include "utils_v1.h"
#include "message.h"

/* return sockfd */
int initSocketServer(int port)
{
	int sockfd = ssocket();
	/* no socket error */

	sbind(port, sockfd);
	/* no bind error */

	slisten(sockfd, BACKLOG);
	/* no listen error */

	return sockfd;
}

int main(int argc, char **argv)
{
	int sockfd, newsockfd;
	// struct sockaddr_in addr;
	/* 1024 client connections MAX */
	struct pollfd fds[1024];
	bool fds_invalid[1024];
	int nbSockfd = 0;
	int i;
	// char msg[MESSAGE_SIZE];
	// Virement virement;

	srand(time(NULL));

	if (argc != 2)
	{
		printf("%s\n", "Usage argv[0] ServerPort");
		exit(1);
	}

	// addr_size = sizeof(struct sockaddr_in);
	sockfd = initSocketServer(atoi(argv[1]));
	printf("Le serveur est à l'écoute sur le port : %i \n", atoi(argv[1]));
	printf("\n");

	fds[nbSockfd].fd = sockfd;
	fds[nbSockfd].events = POLLIN;
	nbSockfd++;
	fds_invalid[nbSockfd] = false;

	while (1)
	{
		spoll(fds, nbSockfd, 0);

		// trt accept fds[0] == socket d'écoute sur le port passé en argument
		if (fds[0].revents & POLLIN & !fds_invalid[0])
		{
			/* client trt */
			newsockfd = saccept(sockfd);
			// add sock to fds poll
			fds[nbSockfd].fd = newsockfd;
			fds[nbSockfd].events = POLLIN;
			nbSockfd++;
			fds_invalid[nbSockfd] = false;
		}

		// trt messages clients
		for (i = 1; i < nbSockfd; i++)
		{
			if (fds[i].revents & POLLIN & !fds_invalid[i])
			{
				int tailleLogique;
				recv(fds[i].fd, &tailleLogique, sizeof(int), 0); // on recoit la taille

				Virement *vList = (Virement *)malloc(sizeof(Virement) * tailleLogique);
				if (vList == NULL)
				{
					perror("Out of memory\n");
					exit(EXIT_FAILURE);
				}

				recv(fds[i].fd, vList, sizeof(Virement) * tailleLogique, 0); // on recoit le tableau

				// tout recup
				int idShm = sshmget(SHM_KEY, 1000 * sizeof(int), 0);
				int *tab = sshmat(idShm);
				int semId = sem_get(SEM_KEY, 1);

				for (int i = 0; i < tailleLogique; i++)
				{
					Virement virement = vList[i];

					printf("Virement de %d euro du compte %d vers le compte %d\n", virement.montant, virement.compteSource, virement.compteDestination);

					sem_down0(semId);
					// debut zone critique
					tab[virement.compteSource] -= virement.montant;
					printf("Nouveau solde du compte %d : %d\n", virement.compteSource, tab[virement.compteSource]);
					tab[virement.compteDestination] += virement.montant;
					printf("Nouveau solde du compte %d : %d\n", virement.compteDestination, tab[virement.compteDestination]);
					printf("\n");
					// fin zone critique
					sem_up0(semId);
				}

				sshmdt(tab);

				sclose(fds[i].fd);
				fds_invalid[i] = true;
			}
		}
	}
}
