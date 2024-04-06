//***********************GAME**********************//
#ifndef SYSTEMES_ET_RESEAUX_PROJET_GAME_H
#define SYSTEMES_ET_RESEAUX_PROJET_GAME_H

#include <stdio.h>              
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>

#define MAX_JOUEURS 4
#define MIN_JOUEURS 2

#define MALLOC_SIZE 1024
#define MALLOC malloc(MALLOC_SIZE * sizeof (char ))


typedef struct Carte {
    unsigned char numero, Tete, is_picked, is_used;
} Carte;

typedef struct Joueur {
    char pseudo[64];
    unsigned short nb_tete, nb_defaite;
    Carte *carte[10];
    Carte *carte_choisie;
} Joueur;


typedef struct Jeu {
    Carte **plateau;
    Joueur *joueur[MAX_JOUEURS];
    Carte *liste_carte[104];
} Jeu;

unsigned int tour = 1, nb_partie = 0, nb_tete_max = 66, nb_manche_max = 10;
unsigned char isOver = 0, nb_joueur = 0, nb_bot = 0, nb_pret = 0;

//*****************FONCTIONS**************/
//Fonction qui permet d'initialiser le jeu
void init_jeu(Jeu *jeu);

//Fonction pour créer un plateau de carte
Carte **create_plateau();

//Fonction pour afficher le plateau ainsi que les cartes présentent dessus
void affiche_plateau(Jeu *jeu, char *buffer);

//Fonction pour libérer de la mémoire la matrice de carte
void free_jeu(Jeu *jeu);

//Fonction qui permet de créer une carte avec en paramètre son numéro
Carte *create_carte(unsigned short i);

//Fonction qui retourne une carte de la liste non utilisée et distribuée.
Carte *get_carte_liste(Jeu *jeu);

//Fonction qui distribue les cartes aux joueurs.
void distribution_carte(Jeu *jeu);

//Fonction qui affiche le nombre de cartes que l'utilisateur peut encore poser
unsigned short get_nb_card_joueur(Jeu *jeu, short idJoueur);

// Retourne sous forme de chaine de charactère les cartes du joueur en paramètre.
void _joueur(Joueur *joueur, char *buffer);

//Retourne le numéro de la dernière carte de la ligne mise en paramètre.
unsigned char get_pos_last_card(Jeu *jeu, int ligne);

//  Retourne la position de la carte qui est le plus proche du numéro en paramètre
char get_pos_card(Jeu *jeu, int numero);

//  Ajoute la carte en paramètre au plateau de jeu, si c'est possible l'ajoute et retourne 1, sinon retourne -1
char add_card(Jeu *jeu, Carte *carte);


//  Renvoie un tableau de joueurs classé par leur carte jouer en ordre croissant.
Joueur **get_ordre_joueur_tour(Jeu *jeu);

// Supprime toutes les cartes de la ligne en paramètre, ajoute la carte à la premiere
void put_carte_sur_ligne(Jeu *jeu, int ligne, Joueur *j);

// Fonction qui affiche le nombre de têtes des joueurs
void affiche_nb_tete_joueurs(Jeu *jeu, char *buffer);

//affiche les carte de joueur
void affiche_cardJ(Joueur *joueur, char *buffer);

//
unsigned char get_last_card(Jeu *jeu, int ligne);



#endif //SYSTEMES_ET_RESEAUX_PROJET_GAME_H
