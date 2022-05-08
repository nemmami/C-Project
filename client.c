#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

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
		printf("%s\n", "Usage argv[0] ServerIP ServerPort, en gros d abord 127.0.0.1, ensuite 9500, ensuite NumCompte et ensuite le delay si t'en a envie mon reuf");
		exit(1);
	}

	// gerer pipe et enfants
	int fd[2]; // fd[0] == READ, fd[1] WRITE
	spipe(fd);
	pid_t filsID1 = sfork(); // fils 1 minuterie

	// creer la liste des virement pour les virement recurent
	int tailleLogique = 0;
	Virement *vList = (Virement *)malloc(sizeof(Virement) * 10);

	if (vList == NULL)
	{
		perror("Out of memory\n");
		exit(EXIT_FAILURE);
	}

	if (filsID1 == 0) // minuterie
	{
		// cloture du descripteur de lecture, peut plus lire dans le pipe
		sclose(fd[0]);
		MessagePipe msgpipe;
		msgpipe.type = 0;
		while (TRUE)
		{
			// le delay
			sleep(atoi(argv[4]))
				swrite(fd[1], &msgpipe, sizeof(MessagePipe));
		}
	}
	else // virement recurent
	{
		pid_t filsID2 = sfork();
		if (filsID2 == 0)
		{
			// cloture du descripteur d'écritue, peut plus ecrire dans le pipe
			sclose(fd[1]);
			while (TRUE)
			{
				MessagePipe msgpipe;
				int a = sread(fd[1], &msgpipe, sizeof(MessagePipe));

				if (msgpipe.type) // type et 1 sa continue
				{
					// ajouter les virement dans la liste
					Virement *v = &vList[*tailleLogique];
					v->compteSource = atoi(argv[3]);
					v->compteDestination = msgpipe.virement.compteDestination;
					v->montant = msgpipe.virement.montant;
					(*tailleLogique)++;
				}
			}
		}
	}

	// initClient before read keyboard ... not a good idea
	// int sockfd = initSocketClient(argv[1], atoi(argv[2]));
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
			printf("'%s'\n", p);
			p = strtok(NULL, d);
			i++;
		}
		/* fin de traitement */

		if (msg[0] == '+')
		{
			Virement virement;
			virement.compteSource = atoi(argv[3]);
			virement.compteDestination = atoi(phr[1]);
			virement.montant = atoi(phr[2]);

			int sockfd = initSocketClient(argv[1], atoi(argv[2]));
			send(sockfd, &virement, sizeof(Virement), 0);
			sclose(sockfd);
		}
		else if (msg[0] == '*')
		{
			/*
			MessagePipe msgpipe;
			msgpipe.type = 1;
			msgpipe.virement.compteDestination = atoi(phr[1]);
			msgpipe.virement.montant = atoi(phr[2]);
			swrite(sockfd, &msgpipe, sizeof(MessagePipe));
			*/
		}
		else if (msg[0] == 'q')
		{
			onContinue = false;
			// sclose(sockfd);
			printf("\nVous êtes déconnecté! \n");
			break;
		}
		else
		{
			printf("tu t'es trompé akhi");
		}

		// swrite(sockfd, msg, sizeof(msg));

		/* wait server response */
		/*
		sread(sockfd, msg, sizeof(msg));

		printf("Réponse du serveur : \n");
		printf("%s\n", msg);
		*/
		printf("\n");
	}

	// sclose(sockfd);
	return 0;
}
