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



int lookup(char* str, char delim){
    for(int i = 0; i < strlen(str); i++){
        if(str[i] == delim){
            return i;
        }
    }
    return -1;
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
        //printf("Erreur, commande mal formée:\n");
        //printf("buffer: %s\n", buffer);
        //printf("========\n");
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
commande_t* handle_cmd(plateau_t *p, int player, int* player_scores, commande_t* cmd ){
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
        info_placement_herisson_t * placement_herisson = demander_placement_herisson(player, nb_herisson_par_joueur, nb_lignes);
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
        commande_t* cmd_play = send_cmd("moved", player, true, 6, resBuff, deBuff, vertBuff, colBuff, vertL, vertC);
        //TODO FREE :c
        return cmd_play;
    }

    if(strcmp(cmd->cmd, "moved")==0){
        printf("Joueur %d traite moved\n", player);
        int joueur_qui_bouge = cmd->id;
        int res = atoi(cmd->args[0])-2;
        int de = atoi(cmd->args[1]);
        int vert = atoi(cmd->args[2]);
        int col = atoi(cmd->args[3])-1;
        int vertL = atoi(cmd->args[4]);
        int vertC = atoi(cmd->args[5]);


        //on joue le coup pour le joueur_qui_bouge
        if(res >= 0){
            player_scores[joueur_qui_bouge]++; //TODO verifier win
        }
    
        if(vert){ //1 si haut, 2 si bas
            int offset = vert == 1 ? -1 : 1;
            printf("On pop %d %d", vertL, vertC);
            char h = board_pop(p, vertL, vertC);
            board_push(p, vertL+offset, vertC, h);
        }

        if(res == -2){
            printf("Le joueur %d ne peut pas bouger et passe son tours\n", joueur_qui_bouge);
            return NULL;
        }


        //res==-1 ou player on fait un deplacement 
        char h = board_pop(p, de, col);
        board_push(p, de, col+1, h);

        board_print(p);

        return NULL;
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
    int* player_scores = calloc(nb_joueur, sizeof(int));

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
        if(n==0){
            break;
        }
        commande_t* cmd = str_to_cmd(buffer);
        printf("Client %d recoit commande:\n", joueur);
        afficher_cmd(cmd);
        printf("fin recv.\n");
        commande_t* res = handle_cmd(p, joueur, player_scores, cmd);
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

        //printf("FIN DE LA BOUCLE\n");

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
    usleep(500*1000);

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

        usleep(500*1000);


    commande_t* start = send_cmd("start", -1, true, 0);
    char* start_str = cmd_to_str(start);
    for(int i = 0; i<nb_joueur; i++){
        write(joueurs[i], start_str, strlen(start_str));
    }
    free(start_str);
    free_cmd(start);

bool is_fini = false;
int* joueurs_score=calloc(nb_joueur, sizeof(int));
while(!is_fini){
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
            if(atoi(res->args[0])-2>=0){
                joueurs_score[atoi(res->args[0])-2]++;
                if(joueurs_score[atoi(res->args[0])-2] >= nb_herisson_par_joueur-1){
                    is_fini = true;
                }
            }
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
    }

    for(int i = 0; i<nb_joueur; i++){
        close(joueurs[i]);
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