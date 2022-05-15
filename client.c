#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils_v1.h"
#include "message.h"

#define MESSAGE_SIZE 8192

// PRE: ServierIP : a valid IP address
//      ServerPort: a valid port number
// POST: on success connects a client socket to ServerIP:port
//       return socket file descriptor
//       on failure, displays error cause and quits the program
int initSocketClient(char ServerIP[16], int Serverport)
{
	int sockfd = ssocket();
	sconnect(ServerIP, Serverport, sockfd);
	return sockfd;
}

int main(int argc, char **argv)
{
	if (argc != 5)
	{
		printf("%s\n", "Usage ./client ServerIP ServerPort Num Delay");
		exit(1);
	}

	int tailleLogique = 0;
	int taillePhysique = 10;

	// on crée le tableau de virement recurrent
	Virement *vList = (Virement *)malloc(sizeof(Virement) * taillePhysique);
	if (vList == NULL)
	{
		perror("Out of memory\n");
		exit(EXIT_FAILURE);
	}

	// on crée les deux fils ainsi que le pipe
	pid_t filsID1, filsID2;
	int fd[2]; // fd[0] == lecture, fd[1] == écriture
	spipe(fd);

	filsID1 = fork(); // fils 1 : minuterie

	if (filsID1 == 0)
	{
		sclose(fd[0]); // cloture du descripteur de lecture
		MessagePipe msgpipe;
		msgpipe.type = TYPE_ENVOI_VIREMENT;
		while (1)
		{
			int delay = atoi(argv[4]);
			sleep(delay);
			swrite(fd[1], &msgpipe, sizeof(MessagePipe)); // on écrit dans le pipe
		}
	}
	else // fils 2 : virement recurrent
	{
		filsID2 = fork();

		if (filsID2 == 0)
		{
			sclose(fd[1]); // cloture du descripteur d'écriture

			while (1)
			{
				MessagePipe msgpipe;
				int ret = sread(fd[0], &msgpipe, sizeof(MessagePipe));
				if (ret == 0) // lorsque l'utilisateur entre "q"
				{
					break;
				}

				if (msgpipe.type == TYPE_AJOUT_VIREMENT) // on ajoute le virement au tableau
				{
					if (tailleLogique == taillePhysique) // cas tableau plein
					{
						taillePhysique *= 2;
						if ((vList = (Virement *)realloc(vList, sizeof(Virement) * (taillePhysique))) == NULL)
						{
							perror("Erreur realloc\n");
							exit(EXIT_FAILURE);
						}
					}

					Virement *v = &(vList)[tailleLogique];
					v->compteSource = msgpipe.virement.compteSource;
					v->compteDestination = msgpipe.virement.compteDestination;
					v->montant = msgpipe.virement.montant;
					(tailleLogique)++;
				}
				else if (msgpipe.type == TYPE_ENVOI_VIREMENT) // on envoie le tableau au serveur
				{
					if (tailleLogique != 0)
					{
						bool estRecurrent = true;
						int sockfd = initSocketClient(argv[1], atoi(argv[2]));
						swrite(sockfd, &tailleLogique, sizeof(int));
						swrite(sockfd, vList, sizeof(Virement) * tailleLogique);
						swrite(sockfd, &estRecurrent, sizeof(bool)); // on previent le serveur qu'il s'agit d'un lot de virements recurrents
						sclose(sockfd);
					}
				}
			}
		}
		else // pere
		{
			bool onContinue = true;

			printf("Bienvenue sur votre banque !\n");
			printf("\n");

			while (onContinue)
			{
				printf("Veuillez entrez une de ces 3 commandes : \n");
				printf("\tEffectuer un virement : + n2 somme\n");
				printf("\tAjouter un nouveau virement réccurent :  * n2 somme\n");
				printf("\tQuitter votre espace client :  q\n");

				char msg[MESSAGE_SIZE];
				sread(0, msg, 256);

				/* on sépare chacunes des données de la commande */
				char *phr[3]; // phr[0] == +/*  phr[1] == n2  phr[2] == somme
				int i = 0;
				char d[] = " ";
				char *p = strtok(msg, d);
				while (p != NULL)
				{
					phr[i] = p;
					p = strtok(NULL, d);
					i++;
				}
				/* fin de traitement */

				int compteSource = atoi(argv[3]);
				int compteDestination = atoi(phr[1]);
				int montant = atoi(phr[2]);

				if (msg[0] == '+') // cas +
				{
					Virement virement;
					virement.compteSource = compteSource;
					virement.compteDestination = compteDestination;
					virement.montant = montant;
					int tailleLogique = 1;
					bool estRecurrent = false;
					int sockfd = initSocketClient(argv[1], atoi(argv[2]));
					swrite(sockfd, &tailleLogique, sizeof(int));
					swrite(sockfd, &virement, sizeof(Virement));
					swrite(sockfd, &estRecurrent, sizeof(bool)); // on previent le serveur qu'il s'agit d'un virement simple
					sclose(sockfd);
				}
				else if (msg[0] == '*') // cas *
				{
					MessagePipe msgpipe;
					msgpipe.type = TYPE_AJOUT_VIREMENT;
					msgpipe.virement.compteSource = compteSource;
					msgpipe.virement.compteDestination = compteDestination;
					msgpipe.virement.montant = montant;
					swrite(fd[1], &msgpipe, sizeof(MessagePipe));
				}
				else if (msg[0] == 'q') // cas q
				{
					onContinue = false;
					sclose(fd[1]); // cloture du descripteur d'écriture, on ne peux plus écrire sur le pipe
					kill(filsID1, SIGUSR1); // on tue notre fils minuterie
					printf("\nVous êtes déconnecté!");
				}

				printf("\n");
			}
		}
	}
	return 0;
}
