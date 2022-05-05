#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#define SERVER_PORT 	9501
#define SERVER_IP		"127.0.0.1"  /* localhost */
#define MAX_PSEUDO 		256

#define PERM 0666
#define TYPE_AJOUT_VIREMENT 44
#define TYPE_ENVOI_VIREMENT 55

#define INSCRIPTION_REQUEST 10
#define INSCRIPTION_OK		11
#define INSCRIPTION_KO 		12

#define SHM_KEY 1234
#define SEM_KEY 4567

/* struct message used between server and client */
typedef struct {
  char messageText[MAX_PSEUDO];
  int code;
} StructMessage;

typedef struct Virement {
  int compteSource;
  int compteDestination;
  int montant;
} Virement;

typedef struct MessagePipe {
  int type;
  Virement virement;
} MessagePiep;


#endif
