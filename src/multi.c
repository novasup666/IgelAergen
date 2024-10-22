#include "multi.h"
#include "csapp.h"

#define READ_SIZE 64
//PARTIE CLIENT


/*
D'après le cours:
Un client getaddrinfo(), il obtient un socket, il se connecte et utilise recv() et send() pour communiquer avec le serveur. Puis il ferme le socket.
*/


//int nb_joueur = 2;
//char* port = "8080";
char* hostname = "localhost";
FILE* log_file = NULL;





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

        if(strcmp(buffer, "place\n") == 0){
            printf("[Client] %d recoit place\n", joueur);
            printf("ON DEMANDE AU JOUEUR %d DE PLACER SA BESTIOLE\n", joueur);
            usleep(500*1000); //TODO regler ce fix pas ouf
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




commande_t* str_to_cmd(char* buffer){
    //on lit la commande de la forme cmd:[nom]\nid:[-1 pour server/num_joueur]\n[nb_args]\n[arg1]\n[arg2]\n...\nfin
    commande_t* res = malloc(sizeof(commande_t));
    if(buffer == NULL){
        return NULL;
    }
    printf("str to cmd: '%s'\n", buffer);
    int index = 0;
    if(strlen(buffer)<4 || buffer[index] != 'c' || buffer[index+1] != 'm' || buffer[index+2] != 'd' || buffer[index+3] != ':'){
        printf("Erreur, commande mal formée:\n");
        printf("buffer: %s\n", buffer);
        printf("========\n");
        return NULL;
    }
    index += 4;
    int name_size = lookup(buffer+index, '\n');
    char* name = malloc(sizeof(char)*(name_size+1));
    strncpy(name, buffer+index, name_size);
    name[name_size] = '\0';
    res->cmd = name;


    index += name_size+1;
    if(buffer[index] != 'i' || buffer[index+1] != 'd' || buffer[index+2] != ':'){
        printf("Erreur, id mal formé\n");
        return NULL;
    }
    index += 3;
    int id_size = lookup(buffer+index, '\n');
    char* id = malloc(sizeof(char)*(id_size+1));
    strncpy(id, buffer+index, id_size);
    id[id_size] = '\0';
    res->id = atoi(id);
    free(id);

    index += id_size;
    if(buffer[index] != '\n'){
        printf("Erreur, nb_args mal formé\n");
        return NULL;
    }
    index++;
    int nb_args_size = lookup(buffer+index, '\n');
    char* nb_args = malloc(sizeof(char)*(nb_args_size+1));
    strncpy(nb_args, buffer+index, nb_args_size);
    nb_args[nb_args_size] = '\0';
    res->nb_args = atoi(nb_args);
    free(nb_args);

    index += nb_args_size+1;
    res->args = malloc(sizeof(char*)*res->nb_args);
    for(int i = 0; i < res->nb_args; i++){
        int arg_size = lookup(buffer+index, '\n');
        char* arg = malloc(sizeof(char)*(arg_size+1));
        strncpy(arg, buffer+index, arg_size);
        arg[arg_size] = '\0';
        res->args[i] = arg;
        index += arg_size+1;
    }

    res->is_cmd = true;
    res->auto_instancie = false;
    return res;
}

//TODO regler le pb des commande auto instancié qui ne sont pas free
//Pour l'instant on ignore la memleak
void free_cmd(commande_t* cmd){
    if(cmd == NULL){
        return;
    }
    if(!cmd->auto_instancie){
        free(cmd->cmd);
        for(int i = 0; i < cmd->nb_args; i++){
        free(cmd->args[i]);
        }
    }
    free(cmd->args);
    free(cmd);
}

char* cmd_to_str(commande_t *cmd){
    char* res = calloc(128, sizeof(char));
    int index = 0;
    strcpy(res, "cmd:");
    index += 4;
    strcpy(res+index, cmd->cmd);
    int taille_cmd = strlen(cmd->cmd);
    strcpy(res+index+taille_cmd, "\n");
    index += taille_cmd+1;
    strcpy(res+index, "id:");
    index += 3;
    int signe = 1;


    if(cmd->id == -1){
        strcpy(res+index, "-1");
        index += 2;
    }else{
        //l'id est positif
        char idbuff[32];
        int taille_id = int_to_ascii(cmd->id, idbuff);
        strcpy(res+index, idbuff);
        index += taille_id;
    }
    strcpy(res+index, "\n");
    index++;

    char numerical_buff[32];
    clear_buffer(numerical_buff, 32);
    int nb_arg_size = int_to_ascii(cmd->nb_args, numerical_buff);
    strcpy(res+index, numerical_buff);
    index += nb_arg_size;
    strcpy(res+index, "\n");
    index++;

    for(int i = 0; i < cmd->nb_args; i++){
        strcpy(res+index, cmd->args[i]);
        index += strlen(cmd->args[i]);
        strcpy(res+index, "\n");
        index++;
    }
    strcpy(res+index, "fin\0");
    return res;
}


commande_t* answer_who(plateau_t*p, int player, commande_t *c){
    commande_t* res = malloc(sizeof(commande_t));
    res->cmd = "im";
    res->id = player;
    res->nb_args = 0;
    res->args = NULL;
    res->is_cmd = true;
    return res;
}



//source pour l'utilisation de vargs https://medium.com/swlh/variadic-function-in-c-programming-d3632315a48e
//permet de transformer des arguments en commande_t
commande_t* send_cmd(char* cmd, int id, bool is_cmd, int nb_args, ...){
    va_list pargs;
    va_start(pargs, nb_args);
    commande_t* res = malloc(sizeof(commande_t));
    res->cmd = cmd;
    res->nb_args = nb_args;
    res->args = malloc(sizeof(char*)*nb_args);
    for(int i = 0; i < nb_args; i++){
        res->args[i] = va_arg(pargs, char*);
    }
    va_end(pargs);
    res->id = id;
    res->is_cmd = is_cmd;
    res->auto_instancie = true;
    return res;

}


//utiliser par le serveur pour traiter les commandes
commande_t* handle_cmd(plateau_t *p, int player, commande_t* cmd ){
    if(cmd == NULL){
        printf("Handle commande NULL\n");
        return NULL;
    }
    printf("Le joueur %d recoit la commande %s\n", player, cmd->cmd);
    //TODO utiliser une hashtbl et des pointeurs de fonctions pour éviter les if
    if(strcmp(cmd->cmd, "who") == 0){
        printf("On recoit who\n");
        return send_cmd("im", player, true, 0);
    }
    if(strcmp(cmd->cmd, "all_players_ok") == 0){
        printf("On recoit all_players_ok\n");
        //on va demander le placement des herissons
        return NULL;
    }

    if(strcmp(cmd->cmd, "place")==0){
        int nb_herisson_par_joueur = atoi(cmd->args[0]);
        int nb_lignes = atoi(cmd->args[1]);

        if(PLACEMENT_ALEATOIRE){
            info_placement_herisson_t * placement_herisson = 
        }
        else{
            info_placement_herisson_t * placement_herisson = demander_placement_herisson(player, nb_herisson_par_joueur, nb_lignes);
        }
        commande_t* res = malloc(sizeof(commande_t));
        res->cmd = "placed";
        res->id = player;
        res->nb_args = nb_herisson_par_joueur;
        res->args = malloc(sizeof(char*)*nb_herisson_par_joueur);
        for(int i = 0; i < nb_herisson_par_joueur; i++){
            res->args[i] = malloc(sizeof(char)*3);
            sprintf(res->args[i], "%d", placement_herisson->lignes[i]);
        }
        for(int h=0; h<placement_herisson->nb_herissons; h++){
                board_push(p, placement_herisson->lignes[h], 0, player_to_herisson(placement_herisson->joueur));
        }

        board_print(p);
        res->is_cmd = true;
        free(placement_herisson);
        return res;
    }

    if(strcmp(cmd->cmd, "placed")==0){
        printf("On reçoit placed    \n");
        for(int h = 0; h<cmd->nb_args; h++){
            board_push(p, atoi(cmd->args[h]), 0, player_to_herisson(cmd->id));
        }
        board_print(p);
        return NULL;
        //TODO utiliser l'argument pour placer le herisson du joueur
    }

    if(strcmp(cmd->cmd, "start")==0){
        printf("Tous les joueurs sont pret !\n");
        return NULL;
    }

    if(strcmp(cmd->cmd, "play")==0){
        info_coup_t * info = jouer_coup(p,player);
        char* resBuff=calloc(16,1);;
        char* deBuff=calloc(16,1);;
        char* vertBuff=calloc(16,1);;
        char* colBuff=calloc(16,1);;
        char* vertL=calloc(16,1);;
        char* vertC=calloc(16,1);;

        int_to_ascii(info->result+2,resBuff); //car jouer_coup peut renvoyer -2
        int_to_ascii(info->de,deBuff);
        int_to_ascii(info->deplacement_vertical,vertBuff);
        int_to_ascii(info->deplacement_colonne+1, colBuff); //+1 car peut être -1
        if(info->deplacement_vertical == 0){
            int_to_ascii(0, vertL);
            int_to_ascii(0, vertC);

        }else{
            int_to_ascii(info->coo_vert->ligne, vertL);
            int_to_ascii(info->coo_vert->colonne, vertC);
        }
        printf("On a comme info: res: %d avec comme '%s' '%s'\n", info->result, vertL, vertC);
        commande_t* cmd_play = send_cmd("move", player, true, 6, resBuff, deBuff, vertBuff, colBuff, vertL, vertC);
        //TODO FREE :c
        return cmd_play;
    }





    printf("Commande non reconnue: '%s'\n", cmd->cmd); 

    return NULL;

    

}

void afficher_cmd(commande_t* cmd){
    if(cmd == NULL){
        printf("Commande NULL\n");
        return;
    }
    printf("Commande: %s\n", cmd->cmd);
    printf("Id: %d\n", cmd->id);
    printf("Nb args: %d\n", cmd->nb_args);
    for(int i = 0; i < cmd->nb_args; i++){
        printf("Arg %d: %s\n", i, cmd->args[i]);
    }
}

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
    int clientfd = open_clientfd(hostname, port);

    while(clientfd < 0){
        printf("Erreur lors de la connexion au serveur, press enter key to retry\n");
        int c = getchar();
        clientfd = open_clientfd(hostname, port);
    }
    char buffer[READ_SIZE];
    clear_buffer(buffer, READ_SIZE);

    int n = 0;
    bool fini = false;
    while(!fini){
        clear_buffer(buffer, READ_SIZE);
        int n = read(clientfd, buffer, 64);
        commande_t* cmd = str_to_cmd(buffer);
        printf("Client %d recoit commande:\n", joueur);
        afficher_cmd(cmd);
        printf("fin recv.\n");
        commande_t* res = handle_cmd(p, joueur, cmd);
        printf("Client %d envoie commande:\n", joueur);
        afficher_cmd(res);
        printf("fin envoie.\n");
        fflush(stdout);
        if(res != NULL && res->is_cmd){
            char* str = cmd_to_str(res);
            int size = strlen(str);
            int size_writed = write(clientfd, str, size);
            while(size_writed < size){
                //eviter les short count error
                printf("Erreur écriture incomplète");
                size_writed += write(clientfd, str+size_writed, size-size_writed);
            }
            free(str);
        }

        //TODO fix la memleak
       //free_cmd(cmd);
       //free_cmd(res);

        printf("FIN DE LA BOUCLE\n");

    }
    return NULL;

}

//cmd:[nom]id:[-1 pour server/num_joueur]\n[nb_args]\n[arg1]\n[arg2]\n...\nfin
void* serv2(void* arg){
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
    
    char buffer[READ_SIZE];
    //on attend les connections des joueurs
    while(nb_joueur_connecte < nb_joueur && (clientfd = accept(listenfd, &clientaddr, &clientlen)) > 0){
        clear_buffer(buffer, READ_SIZE);
        printf("[Serveur]: Nouvelle connection\n");
        //traitement de la connexion
        commande_t* cmd = send_cmd("who", -1, true, 0);
        char* str = cmd_to_str(cmd);
        int size = strlen(str);
        int size_writed = write(clientfd, str, size);
        while(size_writed < size){
            printf("Erreur: écriture incomplète pour le serveur");
            size_writed += write(clientfd, str+size_writed, size-size_writed);
        }
        read(clientfd, buffer, READ_SIZE);
        commande_t* res = str_to_cmd(buffer);

        if(res == NULL){
            continue;
        }

        if(strcmp(res->cmd, "im") == 0 && res->id >= -1 && res->id < nb_joueur){
            joueurs[res->id] = clientfd;
            nb_joueur_connecte++;
            printf("Joueur %d connecté\n", res->id);
        }
        free_cmd(res);
        free(str);
        free_cmd(cmd);
    }

    printf("[Serveur]: Tous les joueurs sont connectés\n");

    for(int i=0; i < nb_joueur; i++){
        char buffer_nb_herisson[32];
        clear_buffer(buffer_nb_herisson, 32);
        int_to_ascii(nb_herisson_par_joueur, buffer_nb_herisson);
        char buffer_nb_lignes[32];
        clear_buffer(buffer_nb_lignes, 32);
        int_to_ascii(NB_LIGNES, buffer_nb_lignes);

        commande_t* cmd = send_cmd("all_players_ok", -1, true, 0);
        char* str = cmd_to_str(cmd);
        write(joueurs[i], str, strlen(str)); //TODO VERIFIER FULL WRITE
        free_cmd(cmd);
        free(str);
    }

    for(int j=0; j<nb_joueur; j++){

        char buffer_nb_herisson[32];
        clear_buffer(buffer_nb_herisson, 32);
        int_to_ascii(nb_herisson_par_joueur, buffer_nb_herisson);
        char buffer_nb_lignes[32];
        clear_buffer(buffer_nb_lignes, 32);
        int_to_ascii(NB_LIGNES, buffer_nb_lignes);

        printf("Commande place envoye par le serveur au joueur %d\n", j);
        commande_t* cmd = send_cmd("place", -1, true, 2, buffer_nb_herisson, buffer_nb_lignes);
        char* str = cmd_to_str(cmd);
        int size = strlen(str);
        int size_writed = write(joueurs[j], str, size);
        while(size_writed < size){
            printf("Erreur: écriture incomplète pour le serveur");
            size_writed += write(joueurs[j], str+size_writed, size-size_writed);
        }
        free(str);
        free(cmd);
        char* reponse=calloc(READ_SIZE, sizeof(char));
        int taille_lu = 0;
        re_read: //pour eviter un j--; contiune
        taille_lu += read(joueurs[j], reponse+taille_lu, READ_SIZE-taille_lu);
        commande_t* res = str_to_cmd(reponse);
        printf("Serveur reçoit réponse: '%s'\n", reponse);
        if(res != NULL && strcmp(res->cmd, "placed") == 0){
            printf("On recoit placed et on transmet aux autres: '%s'\n", reponse);
            for(int other_j = 0; other_j < nb_joueur; other_j++){
                if(j == other_j){
                    continue;
                }
                write(joueurs[other_j], reponse, strlen(reponse));
            }
        }else{
            goto re_read;
        }
        free_cmd(res);
        free(reponse);

    }

    commande_t* start = send_cmd("start", -1, true, 0);
    char* start_str = cmd_to_str(start);
    for(int i = 0; i<nb_joueur; i++){
        write(joueurs[i], start_str, strlen(start_str));
    }
    free(start_str);
    free_cmd(start);


for(int j=0; j<nb_joueur; j++){

        printf("Commande play envoye par le serveur au joueur %d\n", j);
        commande_t* cmd = send_cmd("play", -1, true, 0);
        char* str = cmd_to_str(cmd);
        int size = strlen(str);
        int size_writed = write(joueurs[j], str, size);
        while(size_writed < size){
            printf("Erreur: écriture incomplète pour le serveur");
            size_writed += write(joueurs[j], str+size_writed, size-size_writed);
        }
        free(str);
        free(cmd);

        char* reponse=calloc(READ_SIZE, sizeof(char));
        int taille_lu = 0;
        re_read_mvd: //pour eviter un j--; contiune
        taille_lu += read(joueurs[j], reponse+taille_lu, READ_SIZE-taille_lu);
        commande_t* res = str_to_cmd(reponse);
        printf("Serveur reçoit réponse: '%s'\n", reponse);
        if(res != NULL && strcmp(res->cmd, "moved") == 0){
            printf("On recoit moved et on transmet aux autres: '%s'\n", reponse);
            for(int other_j = 0; other_j < nb_joueur; other_j++){
                if(j == other_j){
                    continue;
                }
                write(joueurs[other_j], reponse, strlen(reponse));
            }
        }else{
            goto re_read_mvd;
        }
        free_cmd(res);
        free(reponse);
    }   

    
    free(joueurs);



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