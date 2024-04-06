//**********//
// Server.h //
//**********//

#ifndef SYSTEMES_ET_RESEAUX_PROJET_SERVEUR_H
#define SYSTEMES_ET_RESEAUX_PROJET_SERVEUR_H


#include<stdlib.h>
#include<stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */

#include "Game.c"
#include "Robot.h"


int PORT = 8080;

#define BUF_SIZE 1024


typedef struct
{
    short numero_joueur;
    short socket;
    short pret;
    Joueur *joueur;
    short robot;

} client;


 //Fonction qui crée un nouveau pointeur de client et le renvoie.
 client *create_joueur();


// Fonction qui accepte ou non les clients qui se connectent au serveur.
void *connexion_players();

// Fonction qui envoie le message en paramêtre à tous les joueurs.
void send_players(client **clients, int nb_clients, char *message);

//Fonction qui écoute en continu le socket du joueur en paramètre, jusqu’à qu’il mette prêt.
void *Start(void *);

// Fonction qui retourne 1 si tous les joueurs sont prêts et 0 sinon.
int start_game();

//Ecoute le joueur, pour récupérer la carte choisie par le joueur.
void *player_card_choice(void *);

// Ecoute le bot, pour récupérer la carte choisie par le bot.
void *robot_card_choice(void *);

// Fonction appelé quand un joueur quitte la partie.
void client_quit(client *c);

// Ferme le socket de tous les clients et du serveur.
void closeServeurConnections();

// Fonction qui écoute le client en paramètre et retourne les données envoyer par le client.
void receive_from_client(client *c, char *buffer);

// Quand un joueur joue une carte qui est trop petite.
int get_user_choice(client *c);

// boucle du jeu
void play_game(Jeu *jeu);

// Met fin au serveur.
void close_serveur();

// Fonction de gestion des signaux du programme
void gestion_signaux_serveur(int signal_recu);

// Fonction qui test si le client en parametre un est bot ou pas.
int is_robot(char *nom, client *c);

// Ajoute un bot au jeu.
void add_robot();

//free les variables du serveur et du jeu.
 
void free_serveur();
void handle_error(const char *message);

#endif //SYSTEMES_ET_RESEAUX_PROJET_SERVEUR_H
