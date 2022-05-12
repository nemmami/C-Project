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
		printf("%s\n", "Usage argv[0] ServerIP ServerPort Num Delay, en gros d abord 127.0.0.1, ensuite 9500, ensuite NumCompte et ensuite le delay si t'en a envie mon reuf");
		exit(1);
	}

	int tailleLogique = 0;
	int taillePhysique = 10;

	// creer la liste des virement pour les virement recurent
	Virement *vList = (Virement *)malloc(sizeof(Virement) * taillePhysique);
	if (vList == NULL)
	{
		perror("Out of memory\n");
		exit(EXIT_FAILURE);
	}

	// gerer pipe et enfants
	pid_t filsID1, filsID2;
	int fd[2]; // fd[0] == READ, fd[1] WRITE
	spipe(fd);

	filsID1 = fork(); // fils 1 minuterie

	if (filsID1 == 0) // minuterie
	{
		sclose(fd[0]); // cloture du descripteur de lecture, peut plus lire dans le pipe
		MessagePipe msgpipe;
		msgpipe.type = TYPE_ENVOI_VIREMENT;
		while (1)
		{
			// le delay
			int delay = atoi(argv[4]);
			sleep(delay);
			swrite(fd[1], &msgpipe, sizeof(MessagePipe)); // on écrit dans le pipe
		}
	}
	else // virement recurent
	{
		filsID2 = fork();

		if (filsID2 == 0)
		{
			sclose(fd[1]); // cloture du descripteur d'écritue, peut plus ecrire dans le pipe

			while (1)
			{
				MessagePipe msgpipe;
				int ret = sread(fd[0], &msgpipe, sizeof(MessagePipe));
				if (ret == 0)
				{
					break;
				}

				if (msgpipe.type == TYPE_AJOUT_VIREMENT) // on ajoute les virements dans la tableau
				{
					if (tailleLogique == taillePhysique) // si plus de place dans le tableau
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
				else // on envoie le tableau au serveur
				{
					if (tailleLogique != 0)
					{
						int sockfd = initSocketClient(argv[1], atoi(argv[2]));
						swrite(sockfd, &tailleLogique, sizeof(int));
						swrite(sockfd, vList, sizeof(Virement) * tailleLogique);
						sclose(sockfd);
					}
				}
			}
		}
		else // le pere
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
				
				if (msg[0] == '+') // cas +
				{
					// printf("msg 0: + ");
					Virement virement;
					virement.compteSource = atoi(argv[3]);
					virement.compteDestination = atoi(phr[1]);
					virement.montant = atoi(phr[2]);
					int tailleLogique = 1;
					int sockfd = initSocketClient(argv[1], atoi(argv[2]));
					swrite(sockfd, &tailleLogique, sizeof(int));
					swrite(sockfd, &virement, sizeof(Virement));
					sclose(sockfd);
				}
				else if (msg[0] == '*') // cas *
				{
					// sclose(fd[0]);
					MessagePipe msgpipe;
					msgpipe.type = TYPE_AJOUT_VIREMENT;
					msgpipe.virement.compteSource = atoi(argv[3]);
					msgpipe.virement.compteDestination = atoi(phr[1]);
					msgpipe.virement.montant = atoi(phr[2]);
					swrite(fd[1], &msgpipe, sizeof(MessagePipe));
				}
				else if (msg[0] == 'q') // cas q
				{
					onContinue = false;
					sclose(fd[1]);
					kill(filsID1, SIGUSR1);
					printf("\nVous êtes déconnecté! \n");
					break;
				}
				printf("\n");
			}
		}
	}
	return 0;
}
