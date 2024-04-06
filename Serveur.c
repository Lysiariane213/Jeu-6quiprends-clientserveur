//**********//
// Server.c //
//**********//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Serveur.h"

#define MAX_BUFFER_SIZE 1024
#define MAX_PLAYER_MESSAGE_SIZE 1024

//********VARIABLES******* //
client *clients[MAX_JOUEURS];
struct sockaddr_in serveur_addr = {0};
Jeu jeu;
int nb_client = 0, serveur_socket;
unsigned short paquet_vide = 0;
unsigned char nb_r = 1;

//**********************main********************/
int main(int argc, char **argv) {
    srand(time(NULL));
    printf("BIENVENUE SUR LE JEU 6 QUI PREND ");
    //GESTION DES SIGNAUX pour fermer le programme correctement
    signal(SIGINT, gestion_signaux_serveur);
    signal(SIGTERM, gestion_signaux_serveur);

    if (argc == 2) {
        PORT = atoi(argv[1]);
    }

    serveur_socket = socket(AF_INET, SOCK_STREAM, 0);

     if (serveur_socket == -1) {
         handle_error("Erreur création socket");
      }

    printf("Création du socket...\n");
    serveur_addr.sin_family = AF_INET;
    serveur_addr.sin_addr.s_addr = INADDR_ANY;
    serveur_addr.sin_port = htons(PORT);

    if (bind(serveur_socket, (struct sockaddr *) &serveur_addr, sizeof(struct sockaddr)) == -1) {
        perror("Erreur de la liaison des sockets");
        close_serveur();
        exit(errno);
    }

    printf("Socket lié avec succès sur le port [%i]\n", PORT);
        listen(serveur_socket, 5);
    //Lancement du thread qui gere les connections des joueurs.
    pthread_t thread;
    pthread_create(&thread, NULL, &connexion_players, NULL);
    //Tant que conditions pour lancer le jeu sont pas bonnes, on attends
    while (start_game() != 1) { usleep(100000); }

    send_players(clients, nb_client, "\nTous les joueurs sont prêt la partie va commencer.\n");
    for (int i = 0; i < nb_client; i++) jeu.joueur[i] = clients[i]->joueur;
    init_jeu(&jeu);
    char plateau[1024];
    affiche_plateau(&jeu, plateau);
    send_players(clients, nb_client, plateau);
    play_game(&jeu);
    close_serveur();
}

//************************FONCTIONS*******************************/
//rejoindre la partie
void *Start(void *argv) {
    client *c = (client *) argv;
    char message[1024];
    char buffer[1024];
    if (isOver == 0) {
        while (1) {
            strcpy(message, "Envoyer  'ok' pour commencer la partie.\n");
            send(c->socket, message, strlen(message), 0);
            receive_from_client(c, buffer);
            if (strcmp(buffer, "ok") == 0) {
                c->pret = 1;
                nb_pret++;
                printf("Le joueur %s a rejoint la partie[%d]\n", c->joueur->pseudo,  nb_joueur);
                //Nombre de joueur pas encore prêt
                if (nb_pret != nb_joueur) 
                send_players(clients, nb_client, "En attente d'autres joueurs\n");               
                }
                while (nb_pret < MIN_JOUEURS) {
                    strcpy(message, "Envoyer  'oui' pour rajouter un robot a la partie.\n");
                    send(c->socket, message, strlen(message), 0);
                    receive_from_client(c, buffer);          
                        if (nb_client < MAX_JOUEURS) {
                             if (strcmp(buffer, "oui") == 0) 
                               add_robot();     
                        }
                }             
            } 
        }
    return 0;
}

//les connexions de clients  
void *connexion_players() {
    while (1) {
        printf("\nEn attente de connexion de client\n");

        struct sockaddr_in client_addr = {0};
        int taille = (int) sizeof(serveur_addr);
        int client_socket = accept(serveur_socket, (struct sockaddr *) &serveur_addr, (socklen_t *) &taille);

        if (client_socket == -1) {
            perror("Erreur lors de l'acceptation de la connexion du client");
          continue;
        }

        if (start_game()) {
            char mes[MAX_PLAYER_MESSAGE_SIZE];
            if (nb_client == MAX_JOUEURS)
               strcpy(mes, "Le nombre de joueurs maximum a été atteint.");
            else strcpy(mes, "La partie a déjà commencé.");
            send(client_socket, mes, strlen(mes), 0);
            close(client_socket);
            continue;
        }

        client *new_client = create_joueur();
        if (new_client == NULL) {
            close(client_socket);
            continue;
        }

        new_client->socket = client_socket;

        char message[MAX_PLAYER_MESSAGE_SIZE];
        char buffer[MAX_BUFFER_SIZE];

        receive_from_client(new_client, buffer);

        int is_robot_result = is_robot(buffer, new_client);
        if (is_robot_result) {
            printf("Un robot s'est connecté.\n");
            continue;
        }

        strcpy(new_client->joueur->pseudo, buffer);

        printf("Connexion réalisée avec le joueur %s\n", new_client->joueur->pseudo);
        snprintf(message, MAX_PLAYER_MESSAGE_SIZE, "Vous êtes le joueur n°%u\n", nb_client + 1);
        send(client_socket, message, strlen(message), 0);

        snprintf(message, MAX_PLAYER_MESSAGE_SIZE, "Le joueur : %s vient de se connecter.", new_client->joueur->pseudo);
        send_players(clients, nb_client, message);

        clients[nb_client] = new_client;

        nb_client++;
        nb_joueur++;
        //gererer les connex clients simult
        pthread_t new_thread;
        pthread_create(&new_thread, NULL, &Start, (void *) new_client);
    }
}

//envoyer des message à tout les joueurs
void send_players(client **clients, int nb_client, char *message) {
    for (int i = 0; i < nb_client; ++i) {
        if (clients[i]->robot == 0) {
            int robot = clients[i]->robot;
            int socket = clients[i]->socket;
            if (robot == 0 && socket != -1) {
                send(socket, message, strlen(message), 0);
            }
        }
    }
}

//créer un joueur
client *create_joueur() {
    client *c = (client *) malloc(sizeof(client));
    c->pret = 0;
    c->robot = 0;
    c->joueur = (Joueur *) malloc(sizeof(Joueur));
    c->numero_joueur = nb_client;
    return c;
}

//les joueurs sont pret pour commancer la partie
int start_game() {
    //Si aucun client
    if (nb_client == 0) 
      return 0;
    unsigned char nbr = 0;
    for (int i = 0; i < nb_client; i++) {
        if (clients[i]->pret == 1) nbr++;
    }
    //Si tous les joueurs sont prets et que le nombre est bon le jeu commence
    if (nbr == nb_client && nbr >= MIN_JOUEURS) return 1;
    return 0;
}

//choisir une carte(joueur)
void *player_card_choice(void *argv) {
    client *c = (client *) argv;
    if (isOver == 0 && get_nb_card_joueur(&jeu, c->numero_joueur) > 0) {
        char message[MAX_PLAYER_MESSAGE_SIZE];
        char buffer[MAX_BUFFER_SIZE];
        //AFFICHAGE DES INFOS DU JOUEUR
        snprintf(message, MALLOC_SIZE, "\n\t*** ROUND [%d] ***\n", tour);
        snprintf(message + strlen(message), MALLOC_SIZE, "Joueur %s, il vous reste %d cartes:\n",c->joueur->pseudo,
                 get_nb_card_joueur(&jeu, (unsigned short) c->numero_joueur));
        snprintf(message + strlen(message), MALLOC_SIZE, "Vous avez %d tetes\n",
                 c->joueur[c->numero_joueur].nb_tete);
        send(c->socket, message, strlen(message), 0);
        char ch[MAX_BUFFER_SIZE];
        affiche_cardJ(c->joueur, ch);
        strcpy(message, ch);
        send(c->socket, message, strlen(message), 0);
        //BOUCLE DE SAISIE DE LA CARTE
        while (1) {
            receive_from_client(c, buffer);
            int nb = atoi(buffer);
            if (nb == 0 || (nb > 10 || nb < 1)) {//on verifie si la carte est valide 
                strcpy(message, "La carte indiquée n’existe pas\n");
                send(c->socket, message, strlen(message), 0);
                continue;
            } else {
                if (c->joueur->carte[nb - 1]->is_used == 1) {
                    strcpy(message, "Cette carte a deja été utilisée, veuillez en choisir une autre\n");
                    send(c->socket, message, strlen(message), 0);
                } else {
                    c->joueur->carte[nb - 1]->is_used = 1;
                    c->joueur->carte_choisie = c->joueur->carte[nb - 1];
                    fflush(stdout);
                    printf("Le joueur %s place sa carte %d > [%d - %d]\n", c->joueur->pseudo, nb - 1,
                           c->joueur->carte[nb - 1]->numero, c->joueur->carte[nb - 1]->Tete);
                    break;
                }
            }
        }
        //Si joueur n'a pas de carte, on incremente le nombre de joueur n'ayant pas de carte
        if (get_nb_card_joueur(&jeu, c->numero_joueur) == 0) {
            paquet_vide++;
        }
    }
}

//choisir une carte(robot)
void *robot_card_choice(void *argv) {
    client *c = (client *) argv;
    if (isOver == 0 && get_nb_card_joueur(&jeu, c->numero_joueur) > 0) {
        char *buffer = (char *) malloc(1024 * sizeof(char));
        while (1) {
            char message[MAX_BUFFER_SIZE];
            int pos = 0; 
            for (int i = 0; i < 10; ++i) {
                if (c->joueur->carte[i]->is_used == 0) {
                    message[pos++] ='0' + i;//le numero de carte choisis 
                
                }
            }

            message[pos] = '\0'; // Ajouter le caractère de fin de chaîne
            send(c->socket, message, strlen(message), 0);       
            //recupere le choix du robot 
            receive_from_client(c, buffer);
            int nb = atoi(buffer);
            if (nb < 10 || nb > 0) {
                if (c->joueur->carte[nb]->is_used == 0) {
                    c->joueur->carte[nb]->is_used = 1;
                    c->joueur->carte_choisie = c->joueur->carte[nb];
                    fflush(stdout);
                    printf("Le robot %s place sa carte %d > [%d - %d]\n\n", c->joueur->pseudo, nb,
                           c->joueur->carte[nb]->numero, c->joueur->carte[nb]->Tete);                
                    break;
                }  
            }            
        }
        //Si bot n'a pas de carte, on incremente le nombre de joueur n'ayant pas de carte
        if (get_nb_card_joueur(&jeu, c->numero_joueur) == 0)
            paquet_vide++;
        free(buffer);
    }
}


//quitter la partie
void client_quit(client *c) {
    char *mess = (char *) malloc(128 * sizeof(char));
    snprintf(mess, strlen(mess), "\nLe client %s a quitté la partie\n", c->joueur->pseudo);
    printf("%s\n", mess);
    send_players(clients, nb_client, mess);
    free(mess);
    close_serveur();
}



//fermer les sockets de tout les clients
void closeServerConnections() {
    for (int i = 0; i < nb_client; ++i) {
        if (clients[i]->socket != -1) {
            close(clients[i]->socket);
            clients[i]->socket = -1;  // Marquer le socket comme fermé
        }
    }
    if (serveur_socket != -1) 
        close(serveur_socket);
      
}


//recevoir les message de client
void receive_from_client(client *c, char *buffer) {
    memset(buffer, 0, MAX_BUFFER_SIZE); // Efface complètement le tampon
    int received = recv(c->socket, buffer, MAX_BUFFER_SIZE - 1, 0);
    if (received <= 0) {
        client_quit(c);
    }
    buffer[received] = '\0';
}
//choisir la ligne à ramasser
int get_user_choice(client *c) {
    char message[MAX_PLAYER_MESSAGE_SIZE];
    char buffer[MAX_BUFFER_SIZE];
    int nbr;
    
    if (c->robot == 1) {
        
         if (c->robot == 1) {
          const char *robotchoice = "1234";
          send(c->socket, robotchoice, strlen(robotchoice), 0);
          receive_from_client(c, buffer);
         return atoi(buffer)-1 ;
       }
    }

        snprintf(message, MAX_PLAYER_MESSAGE_SIZE, "Vous avez choisi la carte la plus petite du plateau\nQuelle rangée souhaitez-vous prendre [1-4] ?\n");
        send(c->socket, message, strlen(message), 0);

    while (1) {
        char buffer[MAX_BUFFER_SIZE];
        receive_from_client(c, buffer);
        nbr = atoi(buffer);
        
        if (nbr < 1 || nbr > 4) {
            snprintf(message, MAX_PLAYER_MESSAGE_SIZE, "Veuillez choisir une rangée entre 1 et 4\n");
            send(c->socket, message, strlen(message), 0);
        } else {
            printf("Le joueur %s choisit de prendre la rangée %d\n", c->joueur->pseudo, nbr - 1); 
            break;
        }
    }
    return nbr - 1;
}

//gérer les signaux
void gestion_signaux_serveur(int signal_recu) {
    switch (signal_recu) {
        //SIGNAL CTRL + C
        case SIGINT:
            close_serveur();
            break;
            //SIGNAL POUR ARRETER PROGRAMME
        case SIGTERM:
            printf("\nSIGNAL SIGTERM RECU\n");
            close_serveur();
            break;
        default:
            printf("\nSIGNAL RECU > %d\n", signal_recu);
            break;
    }
}

//Fermer le serveur
void close_serveur() {
    printf("\nLA PARTIE A ÉTÉ ARRETER\n");
    closeServerConnections();  
    fflush(stdout);
    fflush(stdin);
    free_serveur();
    exit(EXIT_SUCCESS);
}

//robot
int is_robot(char *nom, client *c) {
    if (atoi(nom) == robot_id) {
        c->pret = 1;
        nb_pret++;
        c->robot = 1;
        snprintf(c->joueur->pseudo, 64, "robot n°%d", nb_r);
        nb_r++;
        clients[nb_client] = c;
        send_players(clients, nb_client,"Un robot a été ajouté\n"); 
        nb_client++;
        nb_joueur++;
        return 1;
    }
    return 0;
}

//libere la memoire pour serveur
void free_serveur() {
    for (int i = 0; i < nb_client; ++i) {
        if (clients[i] != NULL)
            free(clients[i]);
        if (jeu.joueur[i] != NULL)
            free(jeu.joueur[i]);
    }
    if (jeu.plateau != NULL)
            free_jeu(&jeu);
}

//gérer le jeu
void play_game(Jeu *jeu) {
    while (1) {
        pthread_t threads[nb_client];
        for (int i = 0; i < nb_client; ++i) {
            if (clients[i]->robot == 0)
                pthread_create(&threads[i], NULL, player_card_choice, (void *) clients[i]);
            else
                pthread_create(&threads[i], NULL, robot_card_choice, (void *) clients[i]);
        }
        //synchroniser les threads
        for (int i = 0; i < nb_client; i++)
            pthread_join(threads[i], NULL);
        //recuperation d'un tableau de pointeur de joueurs trié en fonction de la carte qu'ils ont joué.
        Joueur **joueurs = get_ordre_joueur_tour(jeu);
        //SI PARTIE N'EST PAS TERMINE
        if (isOver == 0) {
            for (int i = 0; i < nb_client; ++i) {
                int pos = add_card(jeu, joueurs[i]->carte_choisie);
                if (pos == 0 || pos == -1) {
                    client *c;
                    for (int j = 0; j < nb_client; ++j) {
                        if (clients[j]->joueur == joueurs[i])
                            c = clients[j];
                    }
                    int ligne = get_pos_card(jeu, joueurs[i]->carte_choisie->numero) / 6;
                    //si on a pas trouve une pos pour la carte
                    if (pos == -1)
                        ligne = get_user_choice(c);
                        //ramasse les tetes de la ligne 
                    put_carte_sur_ligne(jeu, ligne, c->joueur);
                    printf("joueur : %s tete : %i\n\n", c->joueur->pseudo, c->joueur->nb_tete);
                    fflush(stdout);
                }
                if (jeu->joueur[i]->nb_tete >= nb_tete_max && !isOver) {
                    //Au Serveur
                    printf("\n****FIN DE LA PARTIE****\n");
                    printf("****NOMBRE DE TETES MAXIMAL ATTEINT****\n");
                    printf("Le joueur  %s a perdu\n",clients[i]->joueur->pseudo);
                    //Aux joueurs
                    send_players(clients, nb_client,"\n****FIN DE LA PARTIE****\n");
                    send_players(clients, nb_client, "****NOMBRE DE TETES MAXIMAL ATTEINT****\n");
                    char message[MAX_PLAYER_MESSAGE_SIZE];
                    snprintf(message, MALLOC_SIZE, "Le joueur %s a perdu",clients[i]->joueur->pseudo);
                
                    isOver = 1; //Valeur qui nous fait sortir du WHILE
                    break;
                }
            }
            char plateau[MAX_PLAYER_MESSAGE_SIZE];
            affiche_plateau(jeu, plateau);
            send_players(clients, nb_client, plateau);
            //Cas où tous les joueurs n'ont plus de carte, on redonne des cartes et on change le tour
            if (paquet_vide == nb_joueur && !isOver) {
                isOver = 3;
                send_players(clients, nb_client, "\n***ROUND TERMINE***\n");
                send_players(clients, nb_client, "\n***LA PARTIE CONTINUE***\n");
                char tete[MAX_PLAYER_MESSAGE_SIZE];
                affiche_nb_tete_joueurs(jeu, tete);
                send_players(clients, nb_client, tete);
                free_jeu(jeu);
                init_jeu(jeu);
                char plateau[MAX_PLAYER_MESSAGE_SIZE];
                affiche_plateau(jeu, plateau);
                send_players(clients, nb_client, plateau);
                isOver = 0;
                paquet_vide= 0;
                tour++; // Incrementation du tour des joueurs
            }
            //Cas ou nb de tour max atteint
            if (tour >= nb_manche_max && !isOver) {
                //Au serveur
                printf("\n***FIN DE LA PARTIE***\n");
                printf("***NOMBRE DE TOURS MAXIMAL ATTEINT***\n");
                //Aux joueurs
                send_players(clients, nb_client, "\n***FIN DE LA PARTIE***\n");
                send_players(clients, nb_client, "***NOMBRE DE TOURS MAXIMAL ATTEINT***\n");         
                isOver = 1; //Valeur qui nous fait sortir du WHILE
            }
        } // FIN IF JEU TOURNE
            //Cas ou partie est finie
        else {
            printf("\n voulez vous jouer une autre partie ? [O]\n>");
            char rep;
            scanf(" %c", &rep);
            if (rep == 'O') {         
                free_jeu(jeu);
                isOver = 0;
                nb_partie++;
                init_jeu(jeu);
                send_players(clients, nb_client, "La partie va commencer\n");
                printf("La partie va commencer\n");
                char plateau[1024];
                affiche_plateau(jeu, plateau);
                send_players(clients, nb_client, plateau);             
            } else if (rep == 'X') {
                send_players(clients, nb_client, "Le jeu est terminé\n");
                free(joueurs);
                break;
            }
        }
        free(joueurs);
    }
}

//ajouter un robot
void add_robot() {
    char command[1024];
    snprintf(command, 1024, "./Robot %d 127.0.0.1", PORT);
    system(command);
}
void handle_error(const char *message) {
    perror(message);
    close_serveur(); // Fonction pour arrêter proprement le serveur
    exit(EXIT_FAILURE);
}
