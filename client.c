#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

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

	// initClient before read keyboard ... not a good idea
	int sockfd = initSocketClient(argv[1], atoi(argv[2]));
	bool onContinue = true;

	printf("Bienvenue votre banque !\n");
	printf("\n");
	while(onContinue) {
		
		printf("Veuillez entrez une de ces 3 commandes : \n");
		printf("1) Effectuer un virement : + n2 somme\n");
		printf("2) Ajouter un nouveau virement réccurent :  * n2 somme\n");
		printf("3) Quitter votre espace client :  q\n");

		char msg[MESSAGE_SIZE];
		sread(0, msg, 256);

		/* on sépare chacunes des données de la commande */
		char* phr[3]; //phr[0] == +/*  phr[1] == n2  phr[2] == somme
		int i = 0;
		char d[] = " ";
		char *p = strtok(msg, d);
		while(p != NULL) {
			phr[i] = p;
			//printf("'%s'\n", p);
			p = strtok(NULL, d);
			i++;
		}
		/* fin de traitement */

		if(msg[0] == '+') {
			Virement virement;
			virement.compteSource = atoi(argv[3]);
			virement.compteDestination = atoi(phr[1]);
			virement.montant = atoi(phr[2]);
			send(sockfd, &virement, sizeof(Virement), 0);
		} else if(msg[0] == '*') {

		} else if(msg[0] == 'q') {
			onContinue = false;
		} else {
			printf("tu t'es trompé akhi");
		}
		

		//swrite(sockfd, msg, sizeof(msg));

		
		/* wait server response */
		/*
		sread(sockfd, msg, sizeof(msg));

		printf("Réponse du serveur : \n");
		printf("%s\n", msg);
		*/
		printf("\n");	
	}

	sclose(sockfd);
	return 0;
}
