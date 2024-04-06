////********************************Client**************************************///////


#ifndef SYSTEMES_ET_RESEAUX_PROJET_CLIENT_H
#define SYSTEMES_ET_RESEAUX_PROJET_CLIENT_H


#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<signal.h>
#include <time.h>
#include <string.h>


char *hostname = "172.31.18.103";

int PORT = 8080;


// Fonction qui écoute à l'infini sur le socket du serveur.
void *listen_serv(void *);


// Fonction de gestion des signaux du programme
void gestion_signaux_client(int signal_recu);

#endif //SYSTEMES_ET_RESEAUX_PROJET_CLIENT_H
