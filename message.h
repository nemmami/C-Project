#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#define SERVER_PORT 9501
#define SERVER_IP "127.0.0.1" /* localhost */
#define MAX_PSEUDO 256

#define PERM 0666
#define TYPE_AJOUT_VIREMENT 44 
#define TYPE_ENVOI_VIREMENT 55

#define SHM_KEY 1234
#define SEM_KEY 4567

#define MESSAGE_SIZE 8192
#define BACKLOG 5

/* structure d'un virment */
typedef struct Virement
{
  int compteSource;
  int compteDestination;
  int montant;
} Virement;

/* structure du message qu'on envoie à travers du pipe */
typedef struct MessagePipe
{
  int type;
  Virement virement;
} MessagePipe;

#endif
