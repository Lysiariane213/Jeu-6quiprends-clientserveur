//
// ****************************************robot
//

#include "Robot.h"
#include <string.h>
#include <time.h>

int sock;
int main(int argc, char *argv[])
{
    srand(time(NULL));

    if (argc == 3)
    {
        PORT = atoi(argv[1]);
        strcpy(argv[2], hostname);
    }
    printf("%d - %s\n", PORT, hostname);
    fflush(stdout);


    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("erreur bot");
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

    char buff[1024];

    snprintf(buff, 1024, "%d", robot_id);

    send(sock, buff, strlen(buff), 0);


    while (1)
    {
        fflush(stdout);

        char buffer[2048];
        int n = 0;


        if ((n = recv(sock, buffer, sizeof buffer -1, 0)) == 0)
        {
            perror("Erreur reception ");
            exit(errno);
        }
        buffer[n] = '\0';

        char num;
        do
        {
            num = buffer[rand()%n];
       
        } while (strcmp(&num, "") ==0);

        send(sock, &num, 1, 0);

    }

    exit(EXIT_SUCCESS);
}