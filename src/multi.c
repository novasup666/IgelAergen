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
void* serveur_test(void* arg){
    printf("Serveur lancé\n");
    int listenfd = -1;
    int clientfd = -1;
    listenfd = open_listenfd("8080");
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

int calc_size(int n){
    int size = 0;
    while(n > 0){
        n = n/10;
        size++;
    }
    return size;
}

int int_to_ascii(int n, char* buffer){
    if(n == 0){
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
    }

    int size = calc_size(n);
    for(int i=calc_size(n)-1; i>=0; i--){
        buffer[i] = (n%10) + '0';
        n = n/10;
    }
    return size;
}

void clear_buffer(char* buffer, int size){
    for(int i = 0; i < size; i++){
        buffer[i] = '\0';
    }
}

ssize_t write_log(int fd, const char *buf, size_t count, bool is_serv){
    char* txt = is_serv ? "[SERVEUR] " : "[CLIENT] ";
    printf("%s écrit: '%s'\n", txt, buf);
    return write(fd, buf, count);
}


//TODO clear buffer après chaque read
void * serveur(void* arg){
    printf("Serveur lancé\n");
    server_partie_info_t* info = (server_partie_info_t*) arg;
    int nb_joueur = info->nb_joueur;
    int nb_herisson_par_joueur = info->nb_herisson_par_joueur;
    char* port = info->port;

    int listenfd = -1;
    int clientfd = -1;
    printf("OPEN LISTE on %s:\n", port);
    listenfd = open_listenfd(port);
    printf("LISTENFD: %d\n", listenfd);
    while(listenfd < 0){
        printf("[SERVER] FATAL ! Erreur lors de l'ouverture du socket d'écoute, press enter to retry\n");
        getchar();
        listenfd = open_listenfd(port);
        //exit(-1);
        //return NULL;
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
        write_log(clientfd, "who\n", 4, true);
        char buffer[READ_SIZE];
        int n = read(clientfd, buffer, READ_SIZE); //on demande numéro du joueur
        int num_joueur = atoi(buffer);
        printf("[Serveur] recoit %d\n", num_joueur);
        joueurs[num_joueur] = clientfd;
        nb_joueur_connecte++;
    }

    printf("[Server]: Tous les joueurs sont connectés\n");
    //on a tous les joueurs, on peut commencer à demander les placements
    for(int i = 0; i < nb_joueur; i++){
        printf("[Serveur] envoie all_players_ok\n");
        write_log(joueurs[i], "all_players_ok\n", 15, true);
    }

    //les joueurs placent leur pion
    for(int j = 0; j < nb_joueur; j++){
        write_log(joueurs[j], "place\n", 6, true);
        char buffer[READ_SIZE];
        clear_buffer(buffer, READ_SIZE);
        int n = read(joueurs[j], buffer, READ_SIZE);
        for(int other_j = 0; other_j < nb_joueur; other_j++){
                write_log(joueurs[other_j], "placed\n", 7, true);
                write_log(joueurs[other_j], buffer, n, true);
        }
    }

    //on a tous les joueurs, on peut commencer la partie
    for(int i = 0; i < nb_joueur; i++){
        write_log(joueurs[i], "start\n", 6, true);
    }

    usleep(500*1000); //TODO regler ce fix pas ouf, pour laisser le temps aux clients de recevoir le message

    //on attend les coups des joueurs
    bool un_joueur_gagne = false;
    int gagnant = -1;
    while(!un_joueur_gagne){
        for(int i = 0; i < nb_joueur; i++){
            write_log(joueurs[i], "play\n", 5, true);
            char buffer[READ_SIZE];
            clear_buffer(buffer, READ_SIZE);
            int n = read(joueurs[i], buffer, READ_SIZE);
            //on lit un coup du joueur et on l'envoie à tous les autres
            for(int j = 0; j < nb_joueur; j++){
                write(joueurs[j], "move\n", 5);
                write(joueurs[j], buffer, n);
            }

            //on vérifie si le joueur a gagné
            if(buffer[0] == 'W'){
                printf("[Serveur] recoit un mov final\n");
                un_joueur_gagne = true;
                break;
            }
        }
    }
    
    usleep(500*1000); //TODO encore le fix pas ouf

    //on a fini la partie, on envoie ça à tout le monde
    for(int i = 0; i < nb_joueur; i++){
        write_log(joueurs[i], "win\n", 4, true);
    }

    //TODO on ferme les connections
    free(joueurs);
    return NULL;
}


int lookup(char* str, char delim){
    for(int i = 0; i < strlen(str); i++){
        if(str[i] == delim){
            return i;
        }
    }
    return -1;
}   

char** cut(char* str, char delim){
    char** res = malloc(sizeof(char*)*2);
    int index = lookup(str, delim);
    if(index == -1){
        res[0] = NULL;
        res[1] = str;
        return res;
    }
    res[0] = malloc(sizeof(char)*(index+1));
    res[1] = malloc(sizeof(char)*(strlen(str)-index));
    for(int i = 0; i < index; i++){
        res[0][i] = str[i];
    }
    res[0][index] = '\0';
    for(int i = index+1; i < strlen(str); i++){
        res[1][i-index-1] = str[i];
    }
    res[1][strlen(str)-index-1] = '\0';
    return res;
}


//TODO clear le buffer après chaque read
void* client(void* arg){
    client_partie_info_t* info = (client_partie_info_t*) arg;
    int nb_joueur = info->nb_joueur;
    int nb_herisson_par_joueur = info->nb_herisson_par_joueur;
    int joueur = info->joueur;
    char* port = info->port;
    char* hostname = info->hostname;
    info_partie_t* info_partie = info->info;

    plateau_t * p = creer_plateau(NB_LIGNES, NB_COLONNES);

    printf("Client lancé, se connete à %s:%s\n", hostname, port);
    int clientfd = open_clientfd(hostname, port);
    while(clientfd < 0){
        printf("Erreur lors de la connexion au serveur, press any key to retry\n");
        int c = getchar();
        clientfd = open_clientfd(hostname, port);
    }

    bool init_fini = false;
    while(!init_fini){
        char buffer[64];
        clear_buffer(buffer, 64);
        int n = read(clientfd, buffer, 64);

        
        
        if(strcmp(buffer, "who\n") == 0){
            printf("[Client] recoit who\n");
            char* player_number = malloc(sizeof(char)*(nb_joueur+1)); //on veut écrire le numéro du joueur donc on a besoin de  partie entière sup de nb_joueur/10 caractères mais pour simplifier on prend nb_joueur+1
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
            coo_t * coo = demander_coo(p, joueur, true, true);
            //TODO envoyer x1&x2&... pour placer les pions
            printf("Indiquez la colonne où vous souhaitez placer l'hérisson\n")
            char *placement;
            printf("=========================================================\n");
            printf(" Vous allez placer vos herissons de depart !\n");
            for(int herisson = 0; herisson<nb_herrisons_par_joueurs; herisson++){
                printf("Placer le herisson %d:\n>", herisson);
                int c = -1;
                while((c=readInt(TAILLE_MAX_ENTIER)) <0 || c >= nb_lignes){
                    printf("Erreur, joueur %d, veuillez indiquer un nombre entre 0 et %d !\n", joueur, nb_lignes-1);
                    printf("Placer le herisson %d:\n>", herisson);
                }
                if (herisson == 0){
                    asprintf(&placement,"%d",c);
                }
                else{
                    asprintf(&placement,"%s&%d", placement,c);  
                }
            }            

            write(clientfd, placement, 4);
            continue;
        }

        if(strcmp(buffer, "placed\n") == 0){
            //TODO reçoit joueur&x1&x2&... et les place sur le plateau
            char buffer2[32];
            int n = read(clientfd, buffer2, 32);
            printf("[Client] recoit placed: %s\n", buffer2);
            int placeur = buffer2[0];
            int i = 1;
            while (buffer2[i]!='\0'){
                if (buffer2[i]!='&'){
                    board_push(p,0,(int)(buffer2[i]-'0'), player_to_herisson(placeur));
                }
            }
            continue;
        }


        if(strcmp(buffer, "start\n") == 0){
            printf("[Client] recoit start !\n");
            init_fini = true;
            //on peut commencer à jouer
            break;;
        }

        if(strcmp(buffer, "play\n") == 0){
            printf("[Client] recoit play\n");
            //on demande au joueur de jouer et on transforme le coup en "Nx&y&...&win" (next) ou "Wx&y&..." (win)
            //attention un "move gagnant" c'est juste le dernier move jouable, faut encore calculer les gagnants
            //TODO: formater le coup en "Nx&y&...&win" ou "Wx&y&..." 
            //Format d'un coup: [N|W]&[H|B|A]&x&y&c avec N next W win H/B/A pour haut/bas/aucun et (x,y) 
            //les coo du herisson à déplacer verticalement et c la colonne qui fait avancer 
<<<<<<< HEAD
            
            int *infos_coup = calloc(4,sizeof(int));
            jouer()
=======
            info_coup_t * info = jouer_coup(p,current_player);
            //TODO fonction pour formater le coup
            char* coup = "W&1&2";
>>>>>>> f887103 (some fix main/serv)
            write(clientfd, coup, strlen(coup));
            continue;
        }

        if(strcmp(buffer, "move\n") == 0){
            //TODO on recoit le coup du joueur et on le joue
            //TODO on a les coordonnées du joueur player_number faut les utiliser pour mettre à jour le plateau
            char buffer2[32];
            clear_buffer(buffer2, 32);
            int n = read(clientfd, buffer2, 32);
            printf("[Client] recoit move: %s\n", buffer2);
            continue;
        }

        if(strcmp(buffer, "win\n") == 0){
            //TODO on a reçu un message de fin de partie, faut calculer les gagnants
            printf("[Client] recoit win\n");
            break;
        }

        printf("[Client] recoit '%s' qui sera ignoré !\n", buffer);
    }



    printf("Partie finie\n");    
    liberer_plateau(p);

    return NULL;
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