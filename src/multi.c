#include "multi.h"
#include "csapp.h"
#include <pthread.h>

//PARTIE CLIENT

#pragma region CLIENT

/*
D'après le cours:
Un client getaddrinfo(), il obtient un socket, il se connecte et utilise recv() et send() pour communiquer avec le serveur. Puis il ferme le socket.
*/


#pragma endregion CLIENT

int nb_joueur = 1;
char* port = "8080";
char* hostname = "localhost";
void* serveur(void* arg){
    printf("Serveur lancé\n");
    int listenfd = -1;
    int clientfd = -1;
    listenfd = open_listenfd(port);
    if(listenfd < 0){
        printf("Erreur lors de l'ouverture du socket d'écoute\n");
        return NULL;
    }
    printf("Serveur en attente de connexion\n");
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    while((clientfd = accept(listenfd, &clientaddr, &clientlen)) > 0){
        printf("Connexion établie\n");
        //traitement de la connexion
        write(clientfd, "Hello", 5);
        char buffer[1024];
        int n = read(clientfd, buffer, 1024);
        printf("Serveur lit: %s\n", buffer);
        if(strcmp(buffer, "STOP") == 0){
            printf("Serveur reçoit STOP\n");
            break;
        }
        close(clientfd);
    }
    close(listenfd);
    return NULL;
}

void* client(void* arg){
    printf("Client lancé\n");
    int clientfd = open_clientfd(hostname, port);
    if(clientfd < 0){
        printf("Erreur lors de la connexion au serveur\n");
        return NULL;
    }
    char buffer[1024];
    int n = read(clientfd, buffer, 1024);
    printf("Client lit: %s\n", buffer);
    //on repond au serveur STOP
    write(clientfd, "STOP", 4);
    close(clientfd);
    return NULL;
}

int main(){
    pthread_t c, serv;
    printf("On lance le thread serveur: \n");
    pthread_create(&serv, NULL, serveur, NULL);
    printf("On lance le thread client: \n");
    pthread_create(&c, NULL, client, NULL);
    pthread_join(serv, NULL);
    close(c);
    return 0;
}

/*
L'idée d'un serveur: attends les connection des joueurs, lance le signale de départ
Chaque joueur initialise le plateau de jeu et attend un signale pour jouer
Le joueur envoie son coup au serveur
Le serveur envoie le coup à tout les joueurs, qui mettent à jour leur plateau
...
Un joueur envoie un signale de fin de partie
Le serveur envoie un signale de fin de partie
Chaque joueur envoie un message de fin de partie
Le serveur ferme les connections

Ceci est une V1, (même si il n'y aura peut être pas de V2) car c'est très peu sécurisé:
Un joueur peut juste envoyer un message de fin de partie sans avoir gagné (cela pourrait se régler en ajoutant un système de vérification des coups, ou 
en faisant jouer uniquement le serveur, mais pour des raisons de simplicité: non)
*/