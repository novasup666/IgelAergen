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



/* fonction d'exemple pour  ce familiariser avec les sockets, je le delete press enter to retr enter to retrprès TODO DELETE ! 
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


void * serveur(void* arg){
    printf("Serveur lancé\n");
    server_partie_info_t* info = (server_partie_info_t*) arg;
    int nb_joueur = info->nb_joueur;
    int nb_herisson_par_joueur = info->nb_herisson_par_joueur;
    char* port = info->port;

    int listenfd = -1;
    int clientfd = -1;
    listenfd = open_listenfd(port);
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
        printf("[Serveur]: Nouvelle connection\n");
        //traitement de la connexion
        write_log(clientfd, "who\n", 4, true);
        char buffer[READ_SIZE];
        int n = read(clientfd, buffer, READ_SIZE); //on demande numéro du joueur
        int num_joueur = atoi(buffer);
        printf("[Serveur] recoit le numero du joueur: %d\n", num_joueur);
        joueurs[num_joueur] = clientfd;
        nb_joueur_connecte++;
    }

    for(int i = 0; i < nb_joueur; i++){
        char buffer_log[READ_SIZE];
        printf("LOG: Joueur %d connecté\n", i);
        char* msg = "Ton numéro de joueur est: ";
        write_log(joueurs[i], msg, strlen(msg), true);
        int s = int_to_ascii(i, buffer_log);
        write_log(joueurs[i], buffer_log, s, true);
    }

    usleep(500*1000); //TODO regler ce fix pas ouf, pour laisser le temps aux clients de recevoir le message

    printf("[Server]: Tous les joueurs sont connectés\n");
    //on a tous les joueurs, on peut commencer à demander les placements
    for(int i = 0; i < nb_joueur; i++){
        write_log(joueurs[i], "all_players_ok\n", 15, true);
    }

    //les joueurs placent leur pion
    for(int j = 0; j < nb_joueur; j++){
        write_log(joueurs[j], "place\n", 6, true);
        char buffer[READ_SIZE];
        clear_buffer(buffer, READ_SIZE);
        int n = read(joueurs[j], buffer, nb_herisson_par_joueur*2+2); //+2 car au plus 27 joueurs
        printf("[Serveur] recoit placement de %d de valeur '%s' et de taille %d\n", j, buffer, n);

        char* placed_info = calloc(READ_SIZE, sizeof(char));
        strcpy(placed_info, "placed\n");
        strcat(placed_info, buffer);
        for(int other_j = 0; other_j < nb_joueur; other_j++){
            if(other_j == j){
                continue;
            }
            printf("[Serveur] envoie placement à %d de valeur '%s'\n", other_j, buffer);
            write_log(joueurs[other_j], placed_info, nb_herisson_par_joueur*2+2, true);
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

    //on ferme les connections
    for(int i = 0; i < nb_joueur; i++){
        write_log(joueurs[i], "exit\n", 5, true);
        close(joueurs[i]);
    }
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

char* placement_herisson_to_serv_infoi(info_placement_herisson_t* placement_herisson){
    printf("INFO TO SERV nb herisson: %d\n", placement_herisson->nb_herissons);
    char buffer[32];
    clear_buffer(buffer, 32);
    int taille_num_joueur = int_to_ascii(placement_herisson->joueur, buffer);
    char * res = malloc(sizeof(char)*placement_herisson->nb_herissons*2 + taille_num_joueur + 1);
    strcpy(res, buffer);
    res[taille_num_joueur] = '&';
    int index = taille_num_joueur+1;
    for(int h = 0; h<placement_herisson->nb_herissons-1; h++){
        index += int_to_ascii(placement_herisson->lignes[h], res+index);
        res[index] = '&';
        index++;
    }   
    int_to_ascii(placement_herisson->lignes[placement_herisson->nb_herissons-1], res+index);
    return res;
}

//du type: joueur&x&y&z&...
info_placement_herisson_t* servinfo_to_placement_herisson(char* serv, int nb_herissons, int player_who_asked){
    info_placement_herisson_t* res = malloc(sizeof(info_placement_herisson_t));
    res->nb_herissons = nb_herissons;
    int* lignes = malloc(sizeof(int)*nb_herissons);
    int index = 0;
    bool fini = false;
    int index_placement = 0;

    printf("SERV TO INFO CO RECOIT %s\n", serv);
    int debut = lookup(serv, '&');
    char buffer[32];
    clear_buffer(buffer, 32);
    strncpy(buffer, serv, debut);
    int joueur = atoi(buffer);
    index += debut+1;
    while(!fini){
        int next = lookup(serv+index, '&');
        if(next == -1){
            next = strlen(serv);
            fini = true;
        }
        char* buffer = malloc(sizeof(char)*(next+1));
        strncpy(buffer, serv+index, next);
        buffer[next] = '\0';
        lignes[index_placement] = atoi(buffer);
        free(buffer);
        index += next+1;
        index_placement++;
    }
    printf("servtoco\n");
    printf("Joueur %d avec %d herissons\n", joueur, nb_herissons);
    for(int i = 0; i < nb_herissons; i++){
        printf("Ligne %d: %d\n",i, lignes[i]);
    }
    res->lignes = lignes;
    return res;
}

//Format d'un coup: [N|W]&[H|B|A]&player&de&x&y&c avec N next W win H/B/A pour haut/bas/aucun et (x,y) 
char* formatter_coup(info_coup_t *coup){
    if(coup == NULL){
        printf("Erreur, coup NULL\n");
        return NULL;
    }

    char* res = malloc(sizeof(char)*32);
    int index = 0;
    if(coup->result == -1){
        res[index] = 'N';
    }else{
        res[index] = 'W';
    }
    index++;
    res[index] = '&';
    index++;
    if(coup->deplacement_vertical == 0){
        res[index] = 'A';
    }else if(coup->deplacement_vertical == 1){
        res[index] = 'H';
    }else{
        res[index] = 'B';
    }
    index++;
    res[index] = '&';
    index++;
    index += int_to_ascii(coup->joueur, res+index);
    res[index] = '&';
    index++;
    index += int_to_ascii(coup->de, res+index);
    res[index] = '&';
    index++;
    index += int_to_ascii((coup->coo_vert)->ligne, res+index);
    res[index] = '&';
    index++;
    index += int_to_ascii((coup->coo_vert)->colonne, res+index);
    res[index] = '&';
    index++;
    index += int_to_ascii(coup->deplacement_colonne, res+index);
    return res;
}

info_coup_t* convertir_en_coup(char* serv){
    info_coup_t* res = malloc(sizeof(info_coup_t));
    res->result = -1;
    res->coo_vert = malloc(sizeof(coo_t));
    int index = 0;
    if(serv[0] == 'W'){
        res->result = serv[0];
    }
    index += 2;
    if(serv[index] == 'H'){
        res->deplacement_vertical = 1;
    }else if(serv[index] == 'B'){
        res->deplacement_vertical = 2;
    }else{
        res->deplacement_vertical = 0;
    }
    index += 2;
    char buffer[32];
    clear_buffer(buffer, 32);
    int next = lookup(serv+index, '&');
    strncpy(buffer, serv+index, next);
    res->joueur = atoi(buffer);
    index += next+1;
    clear_buffer(buffer, 32);
    next = lookup(serv+index, '&');
    strncpy(buffer, serv+index, next);
    res->de = atoi(buffer);
    index += next+1;
    clear_buffer(buffer, 32);
    next = lookup(serv+index, '&');
    strncpy(buffer, serv+index, next);
    res->coo_vert->ligne = atoi(buffer);
    index += next+1;
    clear_buffer(buffer, 32);
    next = lookup(serv+index, '&');
    strncpy(buffer, serv+index, next);
    res->coo_vert->colonne = atoi(buffer);
    index += next+1;
    clear_buffer(buffer, 32);
    next = lookup(serv+index, '&');
    strncpy(buffer, serv+index, next);
    res->deplacement_colonne = atoi(buffer);
    return res;
}

bool strstart(char* str, char* start){
    int i = 0;
    while(start[i] != '\0'){
        if(str[i] != start[i]){
            return false;
        }
        i++;
    }
    return true;
}

char* getarg(char* str, char sep){
    int index = lookup(str, sep);
    char* res = malloc(sizeof(char)*(index+1));
    strncpy(res, str, index+1);
    res[index] = '\0';
    return res;
}

void* client(void* arg){
    client_partie_info_t* info = (client_partie_info_t*) arg;
    int nb_joueur = info->nb_joueur;
    int nb_herisson_par_joueur = info->nb_herisson_par_joueur;
    int joueur = info->joueur;
    char* port = info->port;
    char* hostname = info->hostname;
    info_partie_t* info_partie = info->info;

    plateau_t * p = creer_plateau(NB_LIGNES, NB_COLONNES);
    int nb_lignes = NB_LIGNES; //ATTENTION: si on modifie la façon de créer le plateau, il faut changer ça
    int nb_colonnes = NB_COLONNES;

    bool buffer_vide_anonce = false;

    printf("Client lancé, se connete à %s:%s\n", hostname, port);
    int clientfd = open_clientfd(hostname, port);
    while(clientfd < 0){
        printf("Erreur lors de la connexion au serveur, press any key to retry\n");
        int c = getchar();
        clientfd = open_clientfd(hostname, port);
    }

    bool fini = false;
    char buffer[64];
    clear_buffer(buffer, 64);
    int n = 0;
    while(!fini){
        clear_buffer(buffer, 64);
        n = read(clientfd, buffer, 64);
        while(1){
        
        if(buffer == NULL){
            printf("Erreur lors de la lecture du buffer, CETTE ERREUR EST NORMALEMENT IMPOSSIBLE !\n");
            break;
            //continue;
        }

        if(buffer[0]=='\0' && !buffer_vide_anonce){
            printf("[Client] recoit un buffer vide !\n");
            buffer_vide_anonce = true;
            break;
        }
        
        if(strcmp(buffer, "who\n") == 0){
            printf("[Client] recoit who\n");
            char* player_number = malloc(sizeof(char)*(nb_joueur+1)); //on veut écrire le numéro du joueur donc on a besoin de  partie entière sup de nb_joueur/10 caractères mais pour simplifier on prend nb_joueur+1
            int s = int_to_ascii(joueur, player_number);
            player_number[s] = '\0';
            write_log(clientfd, player_number, s, false);
            free(player_number);
            //continue;
        }

        if(strcmp(buffer, "all_players_ok\n") == 0){
            printf("[Client] reçoit all_players_ok\n");
            //on a tous les joueurs, on peut commencer à placer les pions
            //continue;
        }   

        if(strstart(buffer, "place\n") == 0){
           

            printf("[Client] reçoit place\n");

            printf("on va demander le placement au joueur %d\n", joueur);
            fflush(stdout);
            sleep(1); //TODO regler ce fix pas ouf
            printf("Faite entrer pour commencer le placement de vos herissons:\n");
            info_placement_herisson_t * placement_herisson = demander_placement_herisson(joueur, nb_herisson_par_joueur, nb_lignes);
            
            
            //envoyer x1&x2&...&n pour placer les pions

            char * placement = placement_herisson_to_serv_infoi(placement_herisson);
            printf("On va envoyer '%s' car on a %d %d %d\n", placement, placement_herisson->joueur, placement_herisson->lignes[0], placement_herisson->lignes[1]);

            for(int h=0; h<placement_herisson->nb_herissons; h++){
                board_push(p, placement_herisson->lignes[h], 0, player_to_herisson(placement_herisson->joueur));
            }

            free(placement_herisson->lignes);
            free(placement_herisson);

            write_log(clientfd, placement, strlen(placement), true);
            free(placement);
            
            board_print(p);
            //continue;
        }

        if(strcmp(buffer, "placed\n") == 0){
            char* args = getarg(buffer, '\n');
            strcpy(buffer, args);
            free(args);

            printf("[Client] recoit placed:\n");
            //reçoit joueur&x1&x2&... et les place sur le plateau
            char buffer2[32];
            int n = read(clientfd, buffer2, 32);
            
            printf("[Client] placed arg: %s\n", buffer2);
            info_placement_herisson_t * placement_herisson = servinfo_to_placement_herisson(buffer2, nb_herisson_par_joueur, joueur);

            printf("On recoit %d %d %d\n", placement_herisson->joueur, placement_herisson->lignes[0], placement_herisson->lignes[1]);
            for(int h=0; h<placement_herisson->nb_herissons; h++){
                board_push(p, placement_herisson->lignes[h], 0, player_to_herisson(placement_herisson->joueur));
            }
            board_print(p);
            free(placement_herisson->lignes);
            free(placement_herisson);

            //continue;
        }


        if(strcmp(buffer, "start\n") == 0){
            printf("[Client] recoit start !\nDebut de la partie avec le plateau:\n");
            board_print(p);
            //on peut commencer à jouer
        }

        if(strcmp(buffer, "play\n") == 0){
            printf("[Client] recoit play\n");
            //on demande au joueur de jouer et on transforme le coup en "Nx&y&...&win" (next) ou "Wx&y&..." (win)
            //attention un "move gagnant" c'est juste le dernier move jouable, faut encore calculer les gagnants
            //Format d'un coup: [N|W]&[H|B|A]&player&x&y&c avec N next W win H/B/A pour haut/bas/aucun et (x,y) 
            //les coo du herisson à déplacer verticalement et c la colonne qui fait avancer 
            board_print(p);
            info_coup_t * info = jouer_coup(p,joueur);
            char* coup = formatter_coup(info);
            write(clientfd, coup, strlen(coup));
            free(coup);
            //continue;
        }

        if(strcmp(buffer, "move\n") == 0){
            //TODO on recoit le coup du joueur et on le joue
            //on a les coordonnées du joueur player_number faut les utiliser pour mettre à jour le plateau
            char buffer2[32];
            clear_buffer(buffer2, 32);
            int n = read(clientfd, buffer2, 32);
            printf("[Client] recoit move: %s\n", buffer2);
            info_coup_t* coup = convertir_en_coup(buffer2);
            if(coup->result == 'W'){
                printf("[Client] recoit win\n");
                fini = true;
                break;
            }

            //erreur de design, on aurait du faire une fonction jouer qui prend un coup en paramètre et qui le joue
            if(coup->deplacement_vertical == 1){
                board_push(p, coup->coo_vert->ligne-1, coup->coo_vert->colonne, player_to_herisson(coup->joueur));
                board_pop(p, coup->coo_vert->ligne, coup->coo_vert->colonne);
            }else if(coup->deplacement_vertical == 2){
                board_push(p, coup->coo_vert->ligne+1, coup->coo_vert->colonne, player_to_herisson(coup->joueur));
                board_pop(p, coup->coo_vert->ligne, coup->coo_vert->colonne);
            }else{
                board_push(p, coup->coo_vert->ligne, coup->deplacement_colonne+1, board_pop(p, coup->coo_vert->ligne, coup->deplacement_colonne));
            }


            board_print(p);
            free(coup->coo_vert);
            free(coup);

            //continue;
        }

        if(strcmp(buffer, "win\n") == 0){
            //TODO on a reçu un message de fin de partie, faut calculer les gagnants
            printf("[Client] recoit win\n");
            fini = true;
            break;
        }

        printf("[Client] recoit une commande non reconnu: '%s' \n", buffer);


        //on essaye de voir si on a pas reçu plusieurs commandes en même temps:
        int index = lookup(buffer, '\n');
        if(index == -1){
            printf("Erreur, buffer non reconnu: %s\n", buffer);
            break;
        }
        strncpy(buffer, buffer+index+1, 64-index);
        printf("nouveau buffer: %s\n", buffer);

        }
    //on relit une nouvelle commande
    }



    printf("Partie finie\n");    
    liberer_plateau(p);

    return NULL;
}


bool accepte_new_player(int* joueurs, int nb_joueur, int new_player){
    for(int i = 0; i < nb_joueur; i++){
        if(joueurs[i] == -1){
            joueurs[i] = new_player;
            return true;
        }
    }
    return false;
}

typedef struct requete_client{
    char* reponse;
    int joueur;
    int size;
} reponse_serveur_t;
typedef struct requete_client requete_client_t;


void* client2(void* arg){
    client_partie_info_t* info = (client_partie_info_t*) arg;
    int nb_joueur = info->nb_joueur;
    int nb_herisson_par_joueur = info->nb_herisson_par_joueur;
    int joueur = info->joueur;
    char* port = info->port;
    char* hostname = info->hostname;
    info_partie_t* info_partie = info->info;

    plateau_t * p = creer_plateau(NB_LIGNES, NB_COLONNES);
    int nb_lignes = NB_LIGNES; //ATTENTION: si on modifie la façon de créer le plateau, il faut changer ça
    int nb_colonnes = NB_COLONNES;
}

//cmd:[nom]\n[nb_args]\n[arg1]\n[arg2]\n...\nfin
void* serv2(void* args){
     printf("Serveur lancé\n");
    server_partie_info_t* info = (server_partie_info_t*) arg;
    int nb_joueur = info->nb_joueur;
    int nb_herisson_par_joueur = info->nb_herisson_par_joueur;
    char* port = info->port;

    int listenfd = -1;
    int clientfd = -1;
    listenfd = open_listenfd(port);
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
        printf("[Serveur]: Nouvelle connection\n");
        //traitement de la connexion
        write_log(clientfd, "who\n", 4, true);
        char buffer[READ_SIZE];
        int n = read(clientfd, buffer, READ_SIZE); //on demande numéro du joueur
        int num_joueur = atoi(buffer);
        printf("[Serveur] recoit le numero du joueur: %d\n", num_joueur);
        joueurs[num_joueur] = clientfd;
        nb_joueur_connecte++;
    }
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