#include "multi.h"
#include "csapp.h"

#define READ_SIZE 32
//PARTIE CLIENT


/*
D'après le cours:
Un client getaddrinfo(), il obtient un socket, il se connecte et utilise recv() et send() pour communiquer avec le serveur. Puis il ferme le socket.
*/


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

//TODO réécrire cette fonction
int int_to_ascii(int n, char* buffer){
    if(n == 0){
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
    }

    int i = 0;
    while(n > 0){
        buffer[i] = n%10 + '0';
        n = n/10;
        i++;
    }
    buffer[i] = '\0';
    return i;
}

void clear_buffer(char* buffer, int size){
    for(int i = 0; i < size; i++){
        buffer[i] = '\0';
    }
}


/*
//TODO clear buffer après chaque read
void * server(void* arg){
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
    while(nb_joueur_connecte < nb_joueur && (clientfd = accept(listenfd, &clientaddr, &clientlen)) > 0){
        printf("Nouvelle connection\n");
        //traitement de la connexion
        printf("[Serveur] envoie who\n");
        write(clientfd, "who", 3);
        char buffer[32];
        int n = read(clientfd, buffer, 32); //on demande numéro du joueur
        int num_joueur = atoi(buffer);
        printf("[Serveur] recoit %d\n", num_joueur);
        joueurs[num_joueur] = clientfd;
        nb_joueur_connecte++;
    }

    printf("[Server]: Tous les joueurs sont connectés\n");
    //on a tous les joueurs, on peut commencer à demander les placements
    for(int i = 0; i < nb_joueur; i++){
        printf("[Serveur] envoie all_players_ok\n");
        write(joueurs[i], "all_players_ok", 14);
    }

    //les joueurs placent leur pion
    for(int j = 0; j < nb_joueur; j++){
        write(joueurs[j], "place", 5);
        char bufferX[32];
        char bufferY[32];



        for(int herisson = 0; herisson < nb_herisson_par_joueur; herisson++){
            clear_buffer(bufferX, 32);
            clear_buffer(bufferY, 32);

            //on lit 2 coordonnées
            int x = read(joueurs[j], bufferX, 32);
            int y = read(joueurs[j], bufferY, 32);

            printf("[Serveur] recoit '%s' '%s'\n", bufferX, bufferY);

            //on envoie les coordonnées à tout les joueurs
            char* player_number = malloc(sizeof(char)*(nb_joueur)); //TODO on veut écrire le numéro du joueur donc on a besoin de 1+nb_joueur/10 caractères
            clear_buffer(player_number, nb_joueur);
            for(int i = 0; i < nb_joueur; i++){
                int_to_ascii(j, player_number);
                printf("[Serveur] envoie 'coo', '%s', '%s', '%s'\n", player_number, bufferX, bufferY);
                printf("[SERVEUR] %d %d %d\n", nb_joueur, strlen(bufferX), strlen(bufferY));
                write(joueurs[i], "coo", 3);
                write(joueurs[i], player_number, nb_joueur);
                write(joueurs[i], bufferX, strlen(bufferX));
                write(joueurs[i], bufferY, strlen(bufferY));
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
            char* player_number = malloc(sizeof(char)*(nb_joueur)); //TODO on veut écrire le numéro du joueur donc on a besoin de nb_joueur/10 caractères
            char buffer_de[32];
            char bufferX[32];
            char bufferY[32];
            char handshake[32];
            clear_buffer(bufferX, 32);
            clear_buffer(bufferY, 32);
            clear_buffer(handshake, 32);
            clear_buffer(buffer_de, 32);
            clear_buffer(player_number, nb_joueur);

            write(joueurs[i], "play", 4);
            int n = read(joueurs[i], buffer_de, 32); //on lit le dès que le joueur a lancé


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
                write(joueurs[j], player_number, nb_joueur);
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

    //TODO on ferme les connections
    return NULL;
}*/


//TODO clear le buffer après chaque read
void* client(void* arg){
    client_partie_info_t* info = (client_partie_info_t*) arg;
    int nb_joueur = info->nb_joueur;
    int nb_herisson_par_joueur = info->nb_herisson_par_joueur;
    int joueur = info->joueur;
    char* port = info->port;
    char* hostname = info->hostname;

    plateau_t * p = creer_plateau(NB_LIGNES, NB_COLONNES);

    printf("Client lancé\n");
    int clientfd = open_clientfd(hostname, port);
    if(clientfd < 0){
        printf("Erreur lors de la connexion au serveur\n");
        return NULL;
    }

    bool init_fini = false;
    while(!init_fini){
        char buffer[64];
        clear_buffer(buffer, 64);
        int n = read(clientfd, buffer, 64);
        
        if(strcmp(buffer, "who\n") == 0){
            printf("[Client] recoit who\n");
            char* player_number = malloc(sizeof(char)*(nb_joueur+1)); //TODO on veut écrire le numéro du joueur donc on a besoin de nb_joueur/10 caractères
            int s = int_to_ascii(joueur, player_number);
            player_number[s] = '\0';
            printf("[Client] envoie %s\n", player_number);
            write(clientfd, player_number, s);
            free(player_number);
            continue;
        }

        if(strcmp(buffer, "all_players_ok\n") == 0){
            printf("[Client] recoit all_players_ok\n");
            //on a tous les joueurs, on peut commencer à placer les pions
            continue;
        }   

        if(strcmp(buffer, "place\n") == 0){
            printf("[Client] recoit place\n");
            //TODO envoyer x\ny\n... pour placer les pions
            continue;
        }
        if(strcmp(buffer, "placed\n") == 0){
            //TODO reçoit joueur\nx\ny\n... et les place sur le plateau
            continue;
        }

        if(strcmp(buffer, "coo") == 0){
            printf("[Client] recoit 'coo'\n");
            char player_number[32];
            char bufferX[32];
            char bufferY[32];
            clear_buffer(player_number, 32);
            clear_buffer(bufferX, 32);
            clear_buffer(bufferY, 32);

            printf("OMG\n");
            int n = read(clientfd, player_number, 32);
            printf("read one fini !\n");
            printf("%s\n", player_number);
            int x = read(clientfd, bufferX, 32);
            printf("read two fini !\n");
            int y = read(clientfd, bufferY, 32);
            printf("[Client] recoit '%s' '%s' '%s'\n", player_number, bufferX, bufferY);
            //TODO on a les coordonnées du joueur player_number faut les utiliser pour initialiser le plateau
            continue;
        }

        if(strcmp(buffer, "start") == 0){
            init_fini = true;
            //on peut commencer à jouer
            break;;
        }

        printf("[Client] recoit '%s' lors de l'initialisation qui sera ignoré !\n", buffer);
    }

    char gagnant[32];
    bool partie_finie = false;
    while(!partie_finie){
        //on attend de lire "play"
        char buffer[32];
        int n = read(clientfd, buffer, 32);
        if(strcmp(buffer, "play") == 0){
            //on demande au joueur de jouer
            int x = 2; //TODO: lire les coordonnées avec ask_user
            int y = 3;
            char bufferX[32];
            char bufferY[32];
            int_to_ascii(x, bufferX);
            int_to_ascii(y, bufferY);
            write(clientfd, bufferX, 32);
            write(clientfd, bufferY, 32);
            //TODO: si on a gagné on envoie "win" sinon "next"
            write(clientfd, "next", 4);
            continue;
        }

        if(strcmp(buffer, "move") == 0){
            char player_number[32];
            char bufferX[32];
            char bufferY[32];
            int n = read(clientfd, player_number, 32);
            int x = read(clientfd, bufferX, 32);
            int y = read(clientfd, bufferY, 32);
            //TODO on a les coordonnées du joueur player_number faut les utiliser pour mettre à jour le plateau
            continue;
        }

        if(strcmp(buffer, "win") == 0){
            int n = read(clientfd, gagnant, 32);
            partie_finie = true;
            break;
        }
        printf("[Client] recoit '%s' lors de la partie qui sera ignoré !\n", buffer);
    }

    printf("Partie finie\n");
    printf("le gagnant est: %s\n", gagnant);
    

    return NULL;
}


ssize_t write_log(int fd, const char *buf, size_t count, bool is_serv){
    char* txt = is_serv ? "[SERVEUR] " : "[CLIENT] ";
    printf("%s écrit: '%s'\n", txt, buf);
    return write(fd, buf, count);
}


void* serverv2(void* arg){
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
    while(nb_joueur_connecte < nb_joueur && (clientfd = accept(listenfd, &clientaddr, &clientlen)) > 0){
        printf("Nouvelle connection\n");
        //traitement de la connexion
        write_log(clientfd, "who\n", 4, true);
        char buffer[READ_SIZE];
        clear_buffer(buffer, READ_SIZE);
        int n = read(clientfd, buffer, READ_SIZE); //on demande numéro du joueur
        int num_joueur = atoi(buffer);
        printf("[Serveur] identifiant joueur %d: recoit %d\n",nb_joueur_connecte, num_joueur);
        joueurs[num_joueur] = clientfd;
        nb_joueur_connecte++;
    }

    printf("[Server]: Tous les joueurs sont connectés\n");
    for(int i = 0; i < nb_joueur; i++){
        printf("[Serveur] envoie all_players_ok\n");
        write_log(joueurs[i], "all_players_ok\n", 15, true);
    }

    for(int joueur = 0; joueur < nb_joueur; joueur++){
        write_log(joueurs[joueur], "place\n", 6, true);
        char buffer[READ_SIZE];
        clear_buffer(buffer, READ_SIZE);
        read(joueurs[joueur], buffer, READ_SIZE);
        for(int other_player = 0; other_player < nb_joueur; other_player++){
            if(other_player == joueur){
                continue;
            }
            write_log(joueurs[other_player], "placed\n", 6, true);
            write_log(joueurs[other_player], buffer, 4, true);
        }
    }
}

int main(){
    server_partie_info_t info;
    info.nb_joueur = 1;
    info.nb_herisson_par_joueur = 1;
    info.port = "8080";
    info.hostname = "localhost";

    client_partie_info_t info_client;
    info_client.nb_joueur = 1;
    info_client.nb_herisson_par_joueur = 1;
    info_client.joueur = 0;
    info_client.port = "8080";
    info_client.hostname = "localhost";

    pthread_t c, serv;
    printf("On lance le thread serveur: \n");
    pthread_create(&serv, NULL, serverv2, &info);
    printf("On lance le thread client: \n");
    client(&info_client);
    pthread_join(serv, NULL);
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
- le joueur envoie le dé qu'il a lancé
- si le joueur veut déplacer un pion verticalement il envoie "dep_vert" "[colonnes]" "vert"/"horiz"
- le joueur envoie son coup "[x]" "[y]"
- le joueur envoie "next" pour dire qu'il a fini de jouer ou "win" pour dire qu'il a gagné
- le serveur envoie le coup à tout les joueurs "move" "joueur" "[x]" "[y]"
...
- le serveur envoie "win" "[numero]" à tout les joueurs
- le serveur se ferme
*/