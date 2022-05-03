
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

#define MESSAGE_SIZE 8192
#define BACKLOG 5

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
	struct sockaddr_in addr;
	/* 1024 client connections MAX */
	struct pollfd fds[1024];
	bool fds_invalid[1024];
	int nbSockfd = 0;
	int i;
	char msg[MESSAGE_SIZE];

	srand(time(NULL));

	if (argc != 2)
	{
		printf("%s\n", "Usage argv[0] ServerPort");
		exit(1);
	}

	// addr_size = sizeof(struct sockaddr_in);
	sockfd = initSocketServer(atoi(argv[1]));
	printf("Le serveur est à l'écoute sur le port : %i \n", atoi(argv[1]));

	fds[nbSockfd].fd = sockfd;
	fds[nbSockfd].events = POLLIN;
	fds_invalid[nbSockfd] = false;
	nbSockfd++;

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
			if (fds[i].revents & POLLIN && !fds_invalid[i])
			{

				sread(fds[i].fd, msg, sizeof(msg));

				printf("MESSAGE RECU DE : %s - ADRESSE IP CLIENT : %s\n", msg, inet_ntoa(addr.sin_addr));

				sleep(2);
				sclose(fds[i].fd);
				fds_invalid[i] = true;
			}
		}
	}
}
