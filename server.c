
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
#include <signal.h>

#include "utils_v1.h"
#include "message.h"

volatile sig_atomic_t end = 0;

void endServerHandler(int sig)
{
	end = 1;
}

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
	/* 1024 client connections MAX */
	struct pollfd fds[1024];
	bool fds_invalid[1024];
	int nbSockfd = 0;
	int i;
	Virement *vList;

	// gestion du cas Ctrl-C
	sigset_t set;
	ssigemptyset(&set);
	sigaddset(&set, SIGINT);
	ssigaction(SIGINT, endServerHandler);

	if (argc != 2)
	{
		printf("%s\n", "Usage ./server ServerPort");
		exit(1);
	}

	sockfd = initSocketServer(atoi(argv[1]));
	printf("Le serveur est à l'écoute sur le port : %i \n", atoi(argv[1]));
	printf("\n");

	fds[nbSockfd].fd = sockfd;
	fds[nbSockfd].events = POLLIN;
	nbSockfd++;
	fds_invalid[nbSockfd] = false;

	while (!end) // tant qu'on a pas effectuer de Ctrl-C
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
			if (fds[i].revents & POLLIN && !fds_invalid[i])
			{
				int tailleLogique;
				read(fds[i].fd, &tailleLogique, sizeof(int)); // on recoit la taille, sread pourrait poser probleme avec le handler

				vList = (Virement *)malloc(sizeof(Virement) * tailleLogique);
				if (vList == NULL)
				{
					perror("Out of memory\n");
					exit(EXIT_FAILURE);
				}

				read(fds[i].fd, vList, sizeof(Virement) * tailleLogique); // on recoit le tableau

				// accès à la mémoire partagé et au sémaphores
				int idShm = sshmget(SHM_KEY, 1000 * sizeof(int), 0);
				int *tab = sshmat(idShm);
				int semId = sem_get(SEM_KEY, 1);

				for (int i = 0; i < tailleLogique; i++) // on effectue chacun des virements du tableau
				{
					Virement virement = vList[i];

					printf("Virement de %d euro du compte %d vers le compte %d\n", virement.montant, virement.compteSource, virement.compteDestination);

					sem_down0(semId);
					// debut zone critique
					// enlever argent compte source ("num")
					tab[virement.compteSource] -= virement.montant;
					if (tailleLogique == 1) // si + n2 somme => Le nouveau solde du compte émetteur (“num”) est affiché.
					{
						printf("Nouveau solde de votre compte %d : %d\n", virement.compteSource, tab[virement.compteSource]);
					} // si * n2 somme =>  Le nouveau solde du compte émetteur (“num”) n’est pas affiché.

					// ajouter argent au destinateur ("n2")
					tab[virement.compteDestination] += virement.montant;

					// fin zone critique
					sem_up0(semId);
				}

				printf("\n");
				sshmdt(tab);

				close(fds[i].fd);
				fds_invalid[i] = true;
			}
		}
	}
}
