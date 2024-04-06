//**********//
// Client.c //
//**********//


#include "Client.h"
#include "Game.h"
#include <pthread.h>

int sock;
char pseudo[64];



//   MAIN   //
// ******** //
int main(int argc, char **argv)
{

    if (argc == 3)
    {
        PORT = atoi(argv[1]);
        strcpy(hostname, argv[2]);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("Impossible de créer le socket");
        exit(errno);
    }

    struct sockaddr_in sin = {0}; /* initialise la structure avec des 0 */

    sin.sin_port = htons(PORT); /* on utilise htons pour le port */
    sin.sin_family = AF_INET;


    if (connect(sock, (struct sockaddr *) &sin, sizeof(struct sockaddr)) == -1)
    {
        perror("Connexion impossible");
        exit(errno);
    }

    //GESTION DES SIGNAUX pour fermer le programme correctement
    signal(SIGINT, gestion_signaux_client);
    signal(SIGTERM, gestion_signaux_client);


    printf("BIENVENUE SUR LE JEU 6 QUI PREND \n\n");


    printf("Choisissez votre pseudo : ");

    scanf("%s", pseudo);
    fflush(stdin);
    fflush(stdout);
    send(sock, pseudo, strlen(pseudo), 0);

    pthread_t pthread;
    //lancer le tread pour ecouter en continue les msg du serv
    pthread_create(&pthread, NULL, &listen_serv, NULL);

    // Boucle pour envoyer des messages au serveur
    while (1)
    {
        char *message = (char *) malloc(512);
        scanf("%s", message);
        send(sock, message, strlen(message), 0);
        fflush(stdin);
        fflush(stdout);
        free(message);
    }

    return EXIT_SUCCESS;
}



// ********* //
// FONCTIONS //


//ecoute et recoie les messages du serveur 
void *listen_serv(void *argv)
{
    while (1)
    {
        char buffer[2048];
        int n = 0;

        if ((n = recv(sock, buffer, sizeof buffer - 1, 0)) == 0)
        {
            perror("Erreur ");
            exit(errno);
        }
        buffer[n] = '\0';
        printf("%s\n", buffer);
        fflush(stdout);
    }
}

void gestion_signaux_client(int signal_recu)
{

    switch (signal_recu)
    {

        //SIGNAL CTRL + C
        case SIGINT:
            printf("\nVous avez quitté la partie\n");
            exit(0);

        //SIGNAL POUR ARRETER PROGRAMME
        case SIGTERM:
            printf("\nSIGNAL SIGTERM RECU\n");
            printf("\nVous avez quitté la partie\nAu revoir\n");
            exit(0);

        default:
            printf("\nSIGNAL RECU > %d\n", signal_recu);
            break;
    }
}
