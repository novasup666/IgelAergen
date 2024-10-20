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

//int nb_joueur = 2;
//char* port = "8080";
char* hostname = "localhost";
FILE* log_file = NULL;

/* fonction d'exemple pour  ce familiariser avec les sockets, je le delete après TODO DELETE !
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
    int nb_connect = 0;
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
}*/

struct server_partie_info {
    int nb_joueur;
    int nb_herisson_par_joueur;
    const char* port;
};
typedef struct partie_info partie_info_t;


void * lancer_serveur(void* arg){
    partie_info_t* info = (partie_info_t*) arg;
    int nb_joueur = info->nb_joueur;
    int nb_herisson_par_joueur = info->nb_herisson_par_joueur;
    const char* port = info->port;

    int listenfd = -1;
    int clientfd = -1;
    listenfd = open_listenfd(port);
    if(listenfd < 0){
        printf("Erreur lors de l'ouverture du socket d'écoute\n");
        return NULL;
    }
    printf("Serveur en attente de connexion\n");
    int nb_connect = 0;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    int nb_joueur_connecte = 0;
    int* joueurs = malloc(sizeof(int) * nb_joueur);
    for(int i = 0; i < nb_joueur; i++){
        joueurs[i] = -1;
    }

    //on attend les connections des joueurs
    while((clientfd = accept(listenfd, &clientaddr, &clientlen)) > 0 && nb_joueur_connecte <= nb_joueur){
        printf("Nouvelle connection\n");
        //traitement de la connexion
        write(clientfd, "who", 3);
        char buffer[32];
        int n = read(clientfd, buffer, 32); //on demande numéro du joueur
        int num_joueur = atoi(buffer);
        if(num_joueur < 0 || num_joueur >= nb_joueur){
            write(clientfd, "Error: joueur non reconnu\n", 27);
            close(clientfd);
            continue;
        }
        joueurs[num_joueur] = clientfd;
        nb_joueur_connecte++;
    }

    //on a tous les joueurs, on peut commencer à demander les placements
    for(int i = 0; i < nb_joueur; i++){
        write(joueurs[i], "all_players_ok", 2);
    }

    //les joueurs placent leur pion
    for(int j = 0; j < nb_joueur; j++){
        write(joueurs[j], "place", 5);
        char bufferX[32];
        char bufferY[32];

        int nb_herisson_correctement_place = 0;
        while(nb_herisson_correctement_place <= nb_herisson_par_joueur){
            //on lit 2 coordonnées
            int x = read(joueurs[j], bufferX, 32);
            int y = read(joueurs[j], bufferY, 32);
            if(x < 0 || y < 0){
                write(joueurs[j], "Error: coordonnées invalides\n", 30);
                continue;
            }
            write(joueurs[j], "coo_ok", 2);
            //on envoie les coordonnées à tout les joueurs
            char* player_number = malloc(sizeof(char)*(nb_joueur/10)); //on veut écrire le numéro du joueur donc on a besoin de nb_joueur/10 caractères
            for(int i = 0; i < nb_joueur; i++){
                itoa(j, player_number, 10);
                write(joueurs[i], "coo", 3);
                write(joueurs[i], player_number, x);
                write(joueurs[i], bufferX, x);
                write(joueurs[i], bufferY, y);
            }
        }
    }

    //on a tous les joueurs, on peut commencer la partie
    for(int i = 0; i < nb_joueur; i++){
        write(joueurs[i], "start", 5);
    }

    //TODO finir ici !

    //on attend les coups des joueurs
    bool un_joueur_gagne = false;
    while(!un_joueur_gagne){
        for(int i = 0; i < nb_joueur; i++){
            write(joueurs[i], "play", 4);
            char buffer[32];
            int n = read(joueurs[i], buffer, 32);
            for(int j = 0; j < nb_joueur; j++){
                write(joueurs[j], buffer, n);
            }
        }
    }
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

/*
protocole de communication:
- le serveur envoie "who" pour demander au client de s'identifier
- le client envoie son numéro de joueur
- le serveur envoie "all_players_ok" pour dire que tout les joueurs sont connectés
- le serveur envoie "place" pour demander au joueur de placer son pion
- les clients placent leur pion sous forme de coordonnées et recoivent "coo_ok" si les coordonnées sont valides
- le serveur envoie "coo" suivit de 3 write: "[joueur]"" "[x]" "[y]" pour dire à tout les joueurs où le joueur [joueur] a placé son pion en [x] [y]
- le serveur envoie "start" pour commencer la partie
- le serveur envoie "play" pour demander au joueur de jouer
- le joueur envoie son coup
- le joueur envoie "next" pour dire qu'il a fini de jouer ou "win" pour dire qu'il a gagné
- le serveur envoie le coup à tout les joueurs
...
- le serveur envoie "win [numero]" à tout les joueurs
- le serveur se ferme
*/