#include "multi.h"
#include "csapp.h"

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


int int_to_ascii(int n, char* buffer){
    int i = 0;
    while(n > 0){
        buffer[i] = n%10 + '0';
        n = n/10;
        i++;
    }
    buffer[i] = '\0';
    return i;
}




void * lancer_serveur(void* arg){
    server_partie_info_t* info = (server_partie_info_t*) arg;
    int nb_joueur = info->nb_joueur;
    int nb_herisson_par_joueur = info->nb_herisson_par_joueur;
    char* port = info->port;

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

        for(int herisson = 0; herisson < nb_herisson_par_joueur; herisson++){
            //on lit 2 coordonnées
            int x = read(joueurs[j], bufferX, 32);
            int y = read(joueurs[j], bufferY, 32);

            //on envoie les coordonnées à tout les joueurs
            char* player_number = malloc(sizeof(char)*(nb_joueur/10)); //on veut écrire le numéro du joueur donc on a besoin de nb_joueur/10 caractères
            for(int i = 0; i < nb_joueur; i++){
                int_to_ascii(j, player_number);
                write(joueurs[i], "coo", 3);
                write(joueurs[i], player_number, nb_joueur/10);
                write(joueurs[i], bufferX, x);
                write(joueurs[i], bufferY, y);
            }
            free(player_number);
        }
    }

    //on a tous les joueurs, on peut commencer la partie
    for(int i = 0; i < nb_joueur; i++){
        write(joueurs[i], "start", 5);
    }

    //on attend les coups des joueurs
    bool un_joueur_gagne = false;
    int gagnant = -1;
    while(!un_joueur_gagne){
        for(int i = 0; i < nb_joueur; i++){
            char* player_number = malloc(sizeof(char)*(nb_joueur/10)); //on veut écrire le numéro du joueur donc on a besoin de nb_joueur/10 caractères
            char bufferX[32];
            char bufferY[32];
            char handshake[32];
            write(joueurs[i], "play", 4);
            int x = read(joueurs[i], bufferX, 32);
            int y = read(joueurs[i], bufferY, 32);
            read(joueurs[i], handshake, 32);
            if(strcmp(handshake, "win") == 0){
                un_joueur_gagne = true;
                gagnant = i;
            }
            int_to_ascii(i, player_number);
            for(int j = 0; j < nb_joueur; j++){
                write(joueurs[j], "move", 4);
                write(joueurs[j], player_number, nb_joueur/10);
                write(joueurs[j], bufferX, x);
                write(joueurs[j], bufferY, y);
            }
            free(player_number);
        }
    }
    
    //on a un gagnant
    char *gagnant_str = malloc(sizeof(char)*nb_joueur/10);
    int_to_ascii(gagnant, gagnant_str);
    for(int i = 0; i < nb_joueur; i++){
        write(joueurs[i], "win", 4);
        write(joueurs[i], gagnant_str, 1);
    }
    free(gagnant_str);
    return NULL;
}


void* client(void* arg){
    server_partie_info_t* info = (server_partie_info_t*) arg;
    int nb_joueur = info->nb_joueur;
    int nb_herisson_par_joueur = info->nb_herisson_par_joueur;
    char* port = info->port;
    char* hostname = info->hostname;

    printf("Client lancé\n");
    int clientfd = open_clientfd(hostname, port);
    if(clientfd < 0){
        printf("Erreur lors de la connexion au serveur\n");
        return NULL;
    }
    char buffer[64];
    int n = read(clientfd, buffer, 64);
    printf("Client lit: %s\n", buffer);
    //on repond au serveur STOP
    write(clientfd, "STOP", 4);
    close(clientfd);
    return NULL;
}

int main(){

    pthread_t c, serv;
    //printf("On lance le thread serveur: \n");
    //pthread_create(&serv, NULL, serveur, NULL);
    //printf("On lance le thread client: \n");
    //pthread_create(&c, NULL, client, NULL);
    //pthread_join(serv, NULL);
    //close(c);
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
- les clients placent leur pion sous forme de coordonnées en envoyant 2 write: "[x]" "[y]"
- le serveur envoie "coo" suivit de 3 write: "[joueur]"" "[x]" "[y]" pour dire à tout les joueurs où le joueur [joueur] a placé son pion en [x] [y]
- le serveur envoie "start" pour commencer la partie
- le serveur envoie "play" pour demander au joueur de jouer
- le joueur envoie son coup "[x]" "[y]"
- le joueur envoie "next" pour dire qu'il a fini de jouer ou "win" pour dire qu'il a gagné
- le serveur envoie le coup à tout les joueurs "move" "joueur" "[x]" "[y]"
...
- le serveur envoie "win" "[numero]" à tout les joueurs
- le serveur se ferme
*/