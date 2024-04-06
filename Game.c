
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <bool_mat.h>
#include "Game.h"

void init_jeu(Jeu *jeu) {
     // Création du plateau de carte 4*6
    jeu->plateau =create_plateau (); 
    //Creation des 104 cartes avec numéro de tête
    for (int i = 0; i < 104; i++) 
        jeu->liste_carte[i] = create_carte(i + 1);

    //Nombre de têtes à 0 vu que le jeu commence et si jeu pas terminé on ne remet pas les têtes à 0.
    if (isOver == 0) {
        for (int i = 0; i < nb_joueur; i++) 
            jeu->joueur[i]->nb_tete = 0;
    }

    //On initialise le plateau à 0.
    Carte carte_p = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++) 
     for (int j = 0; j < 6; j++) 
       jeu->plateau[i][j] = carte_p;

    //Carte de la premiere colonne du plateau distribué
   Carte *cartes[4];

    for (int i = 0; i < 4; ++i) 
       cartes[i] = get_carte_liste(jeu);

    for (int j = 1; j <= 4; j++) {
        for (int i = 0; i < 3; i++) {
            if (cartes[i]->numero > cartes[i + 1]->numero) {
                Carte *c = cartes[i];
                cartes[i] = cartes[i + 1];
                cartes[i + 1] = c;
            }
        }
    }

    for (int i = 0; i < 4; i++) jeu->plateau[i][0] = *cartes[i];

  //distrubution de carte aux joueurs
    distribution_carte(jeu);
}

Carte **create_plateau() {
    Carte **plateau = (Carte **) malloc(6 * sizeof(Carte *));
    for (int i = 0; i < 6; i++) 
       plateau[i] = (Carte *) malloc(6 * sizeof(Carte));
    return plateau;
}
    
//l'ordre de joeueurs pour poser la carte
Joueur **get_ordre_joueur_tour(Jeu *jeu) {
    Joueur **joueurs = (Joueur **) malloc(nb_joueur * sizeof(Joueur *));

    for (int i = 0; i < nb_joueur; ++i) 
       joueurs[i] = jeu->joueur[i];

    for (int j = 1; j <= nb_joueur; j++) {
        for (int i = 0; i < nb_joueur - 1; i++) {
            if (joueurs[i]->carte_choisie->numero > joueurs[i + 1]->carte_choisie->numero) {
                Joueur *j = joueurs[i];
                joueurs[i] = joueurs[i + 1];
                joueurs[i + 1] = j;
            }
        }
    }

    return joueurs;
} 

void affiche_plateau(Jeu *jeu, char *buffer) { 
    char *res = buffer;//initialise un pointeur res pointant vers le début du tampon buffer.
     // Écrit dans res 
    snprintf(res, MALLOC_SIZE, "\t\t\t\t\t\tPLATEAU:\n");

    for (int i = 0; i < 4; i++) {
        snprintf(res + strlen(res), MALLOC_SIZE, "Rangée %u\t", i + 1); // Ajoute au tampon res le numéro de la rangée
        for (int j = 0; j < 6; j++)
            snprintf(res + strlen(res), MALLOC_SIZE, "\t[%03d-%d]\t", jeu->plateau[i][j].numero,
                     jeu->plateau[i][j].Tete);
        snprintf(res + strlen(res), MALLOC_SIZE, "\n");// Ajoute un saut de ligne après avoir parcouru une rangée
    }
    strcpy(buffer, res);// Copie le contenu du tampon res dans le tampon buffer
}

//recuperer la position de la derniere carte
unsigned char get_pos_last_card(Jeu *jeu, int ligne) {
    int pos_derniere_carte = -1; // Initialisation à une valeur impossible pour marquer l'absence de carte

    for (int i = 0; i < 6; ++i) {
        if (jeu->plateau[ligne][i].numero == 0) {
            break; // Si on trouve une carte vide, on arrête la recherche
        }
        pos_derniere_carte = i; // Met à jour la position de la dernière carte non vide
    }

    return (unsigned char)pos_derniere_carte;
}

//crée une carte
Carte *create_carte(unsigned short i) {
    Carte *c = (Carte *) malloc(sizeof(Carte));  // Alloue de la mémoire pour une structure Carte
    c->numero = i;  // Initialise le numéro de la carte avec la valeur passée en paramètre
    c->is_picked = 0;  // Initialise is_picked à 0, indiquant que la carte n'est pas encore sélectionnée
    c->is_used = 0;  // Initialise  is_used à 0, indiquant que la carte n'est pas encore utilisée
    c->Tete = (rand() % 7) + 1;  // Initialise les Tete avec un nombre aléatoire entre 1 et 7 inclus
    return c;  // Retourne un pointeur vers la carte 
}


//ajoute une carte au plateau
char  add_card(Jeu *jeu, Carte *carte) {
  int pos = get_pos_card(jeu, carte->numero);//recuperer la position de la carte sur le plateau
    if (pos == -1 || pos % 6 == 5) {
        return -1;
    } else {
        jeu->plateau[pos / 6][(pos % 6) + 1] = *carte;//ajouter la carte a cette position 
        return 1;
    }
}


//recuperer une carte aleatoire 
Carte *get_carte_liste(Jeu *jeu) {
    Carte *tmp[104];
    unsigned char cpt = 0;

    // Parcours de la liste pour trouver les cartes pas encore prises
    for (int i = 0; i < 104; ++i) {
        if (!(jeu->liste_carte[i]->is_picked)) { // Vérifie si la carte n'est pas encore prise
            tmp[cpt++] = jeu->liste_carte[i];// Stocke un pointeur vers la carte non prise dans tmp
        }
    }

    if (cpt == 0) {
        return NULL; // Aucune carte disponible
    }

    Carte *c = tmp[rand() % cpt];// Sélection aléatoire d'une carte parmi celles non prises
    c->is_picked = 1;// Marque la carte sélectionnée comme prise
    return c;
}

//place carte si trop petite ou derniere ligne
void put_carte_sur_ligne(Jeu *jeu, int ligne, Joueur *j) {
    int Tetes = 0; // pour stocker la somme des valeurs "Tete"
    for (int i = 0; i < 6; ++i) {
        if (jeu->plateau[ligne][i].numero == 0)// Vérifie si la case est vide
            break;
        Tetes += jeu->plateau[ligne][i].Tete; // Calcul du total des valeurs "Tete" des cartes déjà présentes sur la ligne
    }
   // Réinitialisation des cartes sur la ligne 
    memset(jeu->plateau[ligne], 0, 6 * sizeof(Carte));
    // Placement de la carte choisie par le joueur sur la première position de la ligne
    jeu->plateau[ligne][0] = *j->carte_choisie;
    // Mise à jour du nombre total de "Tete" du joueur 
    j->nb_tete += Tetes;
}

//determiner au placer la carte(get_pos_carte_mini)
char get_pos_card(Jeu *jeu, int numero) {
    char pos = -1;
    short diff = numero - get_last_card(jeu, 0);
    
    if (diff > 0)
        pos = get_pos_last_card(jeu, 0);
    for (int i = 1; i < 4; ++i) {
        short diff_i = numero - get_last_card(jeu, i);
         if (diff_i >= 0 && diff_i < diff) {
            diff = diff_i;
            pos = (i * 6) + get_pos_last_card(jeu, i);
        }
    }
    return pos;
}

//distribuer les cartes
void distribution_carte(Jeu *jeu) {
    for (int i = 0; i < nb_joueur; i++) {
       Joueur *j = jeu->joueur[i]; // Sélectionne un joueur du jeu
        for (int k = 0; k < 10; k++) { // Distribue 10 cartes à ce joueur
            j->carte[k] = get_carte_liste(jeu); // recuperer une carte non prise à partir de la liste 
        }
    }
}

//nombre de carte restantes
unsigned short get_nb_card_joueur(Jeu *jeu, short idJoueur) {
    unsigned short cpt = 0;
    for (int i = 0; i < 10; i++) 
      if (jeu->joueur[idJoueur]->carte[i]->is_used == 0) 
       cpt++;
    return cpt;
}


//affiche nombre de têtes de chaque joueur
void affiche_nb_tete_joueurs(Jeu *jeu, char *buffer) {
    char *tmp = MALLOC;
    snprintf(tmp, MALLOC_SIZE, "");

    for (short i = 0; i < nb_joueur; i++)
        snprintf(tmp + strlen(tmp), MALLOC_SIZE, "Le joueur %s possède %d têtes \n", jeu->joueur[i]->pseudo,
                 jeu->joueur[i]->nb_tete);

  

    strcpy(buffer, tmp);
    free(tmp);
}


//afficher les cartes de joueur
void affiche_cardJ(Joueur *joueur, char *buffer) {
    char *res = MALLOC;
    snprintf(res, MALLOC_SIZE, "");
    for (int i = 0; i < 10; i++) {
        if (joueur->carte[i]->is_used == 0)
            snprintf(res + strlen(res), MALLOC_SIZE, "Carte %02d > Numéro[%03d] Tete[%d]\n", i + 1,
                     joueur->carte[i]->numero,
                     joueur->carte[i]->Tete);
    }
    strcpy(buffer, res);
    free(res);
}

//recuperer la derniere carte de la ligne
unsigned char get_last_card(Jeu *jeu, int ligne) {
    for (int i = 0; i < 6; ++i) {//parcourir les cartes de la ligne
        if (jeu->plateau[ligne][i].numero == 0) {
            return jeu->plateau[ligne][i - 1].numero;//on recupere la valeur de la carte precedente
        }
    }
    return 0;
}

//liberer la memoire louée par jeu
void free_jeu(Jeu *jeu) {
    for (int i = 0; i < 6; i++)
      free(jeu->plateau[i]);
    free(jeu->plateau);
    for (int i = 0; i < 104; i++)
      free(jeu->liste_carte[i]);
}


