#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include "main.h"
#include "multi.h"

int PIEGEES [6]  = {2,6,4,5,3,7};

//définit la taille du buffer pour les entiers

//TODO remplacer ca par la lecture des arguments du main


//gère la logique du jeu 

// /!\ ON STOCKERA DES MINUSCULES !!!!!!

case_t* creer_case(int ligne, int colonne, int nb_herissons, char* herissons, bool piegee){

    rca_t * h = creer_rca(1); 
    for(int i=0; i<nb_herissons; i++){
        push_rca(h,herissons[i]);
    }

    case_t* c = malloc(sizeof(case_t));
    c->ligne = ligne;
    c->colonne = colonne;
    c->herissons = h;
    c->is_piege = piegee;
    return c;
}

plateau_t* creer_plateau(int nb_lignes, int nb_colonnes){
    plateau_t *p = calloc(1, sizeof(plateau_t));
    p->nb_lignes = nb_lignes;
    p->nb_colonnes = nb_colonnes;
    p->cases = calloc(nb_lignes*nb_colonnes, sizeof(case_t));
    for(int i = 0; i< nb_lignes*nb_colonnes; i++){
        int ligne = i/nb_colonnes;
        int colonne = i%nb_colonnes;
        p->cases[i] = creer_case(ligne, colonne, 0, NULL, (PIEGEES[ligne] == colonne)); //TODO changer Null en le bon argument. NULL est le bon argument pour l'initialisation
    }

    return p;
}

int lancer_de(void){
    return rand()%NB_LIGNES ;
}


void board_push(plateau_t* p, int line, int col, char ctn){
    push_rca((p->cases[line* (p->nb_colonnes) +col])->herissons,ctn); 
}

char board_pop(plateau_t* p, int line, int col){
    return pop_rca((p->cases[line* (p->nb_colonnes) +col])->herissons);
}

int board_height(plateau_t*p, int line, int col){
    return((p->cases[line* (p->nb_colonnes) +col])->herissons)->size;
}

//j'ai changé la spec, ça renvoie 0 si la case est vide
char board_top (plateau_t*p, int line, int col){
    rca_t* h = (p->cases[line* (p->nb_colonnes) +col])->herissons;
    return is_empty_rca(h) ? 0 : peek_rca(h, 0);
}

char board_peek(plateau_t*p, int line, int col, int pos){
    return peek_rca((p->cases[line* (p->nb_colonnes) +col])->herissons,pos);
}

bool board_is_empty(plateau_t*p, int line, int col){
    return board_top(p, line, col) == 0;
}




void cell_print(plateau_t* p, int line, int col, int slice){
    char top;
    char lb;
    char rb;
    char bot;
    if((p->cases[line* (p->nb_colonnes) +col])->is_piege){
        top = 'V';
        lb = '>';
        rb = '<';
        bot= '^';
    }else{
        top = '_';
        lb = '|';
        rb = '|';
        bot= '-';
    }
    if (slice == 0){
        printf("  %c%c%c  ", top,top,top);
    }
    if (slice == 1){
        if (board_height(p,line,col) == 0){
            printf(" %c   %c ",lb,rb);
        }
        else{
            char team = board_top(p,line,col);
            printf(" %c%c%c%c%c ",lb,(team  - 0x20),(team  - 0x20),(team  - 0x20),rb); //attention c'est pas un + mais un - (a-0x20=A)

        }
    }
    if (slice == 2){
        if (board_height(p,line,col) == 0){
            printf(" %c   %c ",lb,rb);
        }
        if(board_height(p,line,col) == 1){
            char team = board_top(p,line,col);
            printf(" %c%c%c%c%c ",lb,(team - 0x20),(team - 0x20),(team- 0x20),rb);
        }    
        if(board_height(p,line,col) == 2){
            char team = board_peek(p,line,col,1);
            printf(" %c %c %c ",lb,(team ),rb);
        }  
        if(board_height(p,line,col) == 3){
            char team1 = board_peek(p,line,col,1);
            char team2 = board_peek(p,line,col,2);
            printf(" %c%c %c%c ",lb,(team1),(team2),rb);
        }  
        if(board_height(p,line,col) >= 4){
            char team1 = board_peek(p,line,col,1);
            char team2 = board_peek(p,line,col,2);
            char team3 = board_peek(p,line,col,3);
            printf(" %c%c%c%c%c ",lb,(team1),(team2), (team3),rb);
        } 
    }
    if (slice == 3){
        int n = board_height(p,line,col);
        if (n<=1){
            printf("  %c%c%c  ",bot,bot,bot);
        }
        else if(n<10){
            printf("  %c%d%c  ",bot,n,bot);
        }
        else if (n<100){
            printf("  %d   ",n);
        }
        else if (n<1000){
            printf("  %d  ",n);
        }
        else{
            printf("  bcp  ");
        }
    }
    
}

void board_print(plateau_t * p){
   
    printf("        DÉBUT ");
    for(int i = 0; i<(p->nb_colonnes)-2; i++){
        printf("       ");
    }
    printf("  FIN \n");
    printf("Adil   ");
    for(int i = 0; i<(p->nb_colonnes); i++){
        printf("  col  ");
    }
    printf("\n");
    printf("& Noé  ");
    for(int i = 0; i<(p->nb_colonnes); i++){
        printf("   %d   ", (i));
    }   
    printf("\n");
    for(int i = 0;i<(p->nb_lignes); i++){
        for(int s = 0; s<4; s++){
            if(s == 1){printf(" ligne ");}
            else{
            if(s == 2){printf("   %d   ",i);}
            else{printf("       ");}}
            for(int j = 0; j<(p->nb_colonnes); j++){
                cell_print(p,i,j,s);
            }
            printf("\n");
        }

    }
    printf("\n");
}

//un joueur est représenté par un entier entre 0 et 26 et un hérisson par une lettre minuscule entre a et z
char player_to_herisson(int joueur){
    return joueur + 'a';
}

int herisson_to_player(char herisson){
    return herisson - 'a';
}

plateau_t* initialiser_partie(int lignes, int colonnes, info_partie_t* info){
    plateau_t *p = creer_plateau(lignes, colonnes);
    p->nb_joueurs = info->nb_joueurs;
    p->nb_herissons_par_joueurs = info->nb_herissons_par_joueurs;
    //place les herissons correctements sur la 1er ligne
    for(int joueur = 0; joueur < info->nb_joueurs; joueur++){
        for(int herisson = 0; herisson < info->nb_herissons_par_joueurs; herisson++){
            board_push(p, info->placement_herissons[joueur][herisson] , 0, player_to_herisson(joueur));

        }
    }
    return p;
}



void liberer_case(case_t* c){
    liberer_rca(c->herissons); 
    free(c);
}

void liberer_plateau(plateau_t* p){
    for(int i = 0; i<p->nb_lignes; i++){
        for(int j = 0; j<p->nb_colonnes; j++){
            liberer_case(p->cases[i*p->nb_colonnes+j]);
        }
    }
    free(p->cases);
    free(p);
}

bool is_herisson_on_case(plateau_t* p, int joueur, int ligne, int colonne){
    return board_top(p, ligne, colonne) == player_to_herisson(joueur);
}

//TODO utiliser une carte des pièges
bool is_herisson_trapped(plateau_t* p, int joueur, int ligne, int colonne){
    if (!(p->cases[ligne*(p->nb_colonnes)+colonne]->is_piege)){
        return false;
    }
    for (int j = 0; j < colonne; j++){
        if(!(board_is_empty(p,ligne,j))){
            return true;
        }
    }
    return !(is_herisson_on_case(p,joueur,ligne,colonne));
}

bool is_herisson_last(plateau_t* p, int joueur, int ligne, int colonne){
    if(!is_herisson_on_case(p, joueur, ligne, colonne)){
        return false;
    }
    for(int i=0; i<ligne; i++){
        if(!board_is_empty(p, i, colonne)){
            return false;
        }
    }
    return true;
}

bool is_coup_possible(plateau_t* p, int ligne){
    for(int i=0; i<p->nb_colonnes; i++){
        if(!board_is_empty(p, ligne, i)){
            return true;
        }
    }
    return false;
}

//-2 si il peut pas jouer
//-1 si il joue et que aucun herisson arrive sur la dernière colonne
//player si le herisson de player arrive sur la dernière colonne
info_coup_t* jouer_coup(plateau_t *p, int joueur){
    printf("DEBUG: joueur %d\n", joueur);
    int de = lancer_de();
    //TODO remove ce fix hideux
    while(de >= p->nb_lignes){
        printf("Erreur, le dé a donné un nombre trop grand (%d) pour le nombre de lignes (%d)\n\n", de, p->nb_lignes);
        de=lancer_de();
    }

    info_coup_t* info = malloc(sizeof(info_coup_t));
    info->joueur = joueur;
    info->result = -1;
    info->deplacement_vertical = -1;
    info->deplacement_colonne = -1;
    info->coo_vert = NULL;
    info->de = de;


    printf("Le joueur %d a lancé un %d et peu donc faire avancer un herisson de la ligne %d\n\n", joueur, de, de);
    
    //on laisse le joueur deplacer un herisson verticalement si il le désire
    bool choix_vertical_valide = false;
    int return_value = -1;
    while(!choix_vertical_valide){
        printf("Selectionnez un herisson à déplacer verticalement (optionel):\n");
        coo_t* c = demander_coo(p, joueur, true, false);
        if(c==NULL){
            printf("Joueur %d, vous avez choisi de ne pas déplacer de herisson verticalement\n", joueur);
            choix_vertical_valide = true;
            info->deplacement_vertical = 0;
        }
        else{
            int deplacement = demander_deplacement(p, joueur); //1=haut 2=bas
            info->deplacement_vertical = deplacement;
            info->coo_vert = c;
            if(deplacement == 1){
                if(c->ligne <= 0){
                    printf("Erreur, vous ne pouvez pas déplacer le herisson %d vers le haut\n", joueur);
                }
                else{
                    if(is_herisson_trapped(p,joueur,de,c->colonne)){
                        printf("Malheureusement la case (%d, %d) est piégée, l'hérisson est bloqué...\n\n", de, c->colonne);
                    }
                    else{
                    board_push(p, c->ligne - 1, c->colonne, player_to_herisson(joueur));
                    board_pop(p, c->ligne, c->colonne);
                    choix_vertical_valide = true;}
                }
            }
            else{
                if(c->ligne >= p->nb_lignes){
                    printf("Erreur, vous ne pouvez pas déplacer le herisson %d vers le bas\n", joueur);
                }
                else{
                    if(is_herisson_trapped(p,joueur,de,c->colonne)){
                        printf("Malheureusement la case (%d, %d) est piégée, l'hérisson est bloqué...\n", de, c->colonne);
                    }
                    else{
                    board_push(p, c->ligne + 1, c->colonne, player_to_herisson(joueur));
                    board_pop(p, c->ligne, c->colonne);
                        choix_vertical_valide = true;
                    }
                }
            }
        }
        free(c);
    }

        board_print(p);

    if(!is_coup_possible(p, de)){
        printf("Joueur %d: vous ne pouvez pas jouer de coup, vous passez votre tour !\n", joueur);
        info->result = -2;
        return info;
    }

    //le joueur doit donc déplacer un herisson horizontalement de la ligne déterminée par le dé
    bool choix_horizontal_valide = false;
    while(!choix_horizontal_valide){
        printf("Joueur %d: Selectionnez un herisson à déplacer horizontalement sur la ligne %d (ceci est obligatoire pour les ects):\n", joueur, de); //TODO remove la vanne claqué sur les ects
        int colonne = demander_coo_plateau(joueur, p->nb_colonnes-1, false, true); //on ne veut pas bouger un herisson sur la dernière colonne
        info->deplacement_colonne = colonne;
        if(board_is_empty(p, de, colonne)){
            printf("Erreur, la case (%d, %d) est vide\n", de, colonne);
        }
        else if(is_herisson_trapped(p,joueur,de,colonne)){
            printf("Malheureusement la case (%d, %d) est piégée, l'hérisson est bloqué...\n", de, colonne);
        }
        else{
            //on déplace le herisson
            board_push(p, de, colonne+1, board_pop(p, de, colonne));
            if(colonne+1 == p->nb_colonnes -1){
                return_value = herisson_to_player(board_top(p, de, colonne+1));
                info->result = return_value;
            }
            choix_horizontal_valide = true;
        }
    }


    printf("PUTIN DE COUILLASE DE CONNARD DE MERDE %d\n", info->result);
    
    return info;
}




void game_loop(plateau_t *p){
    bool partie_finie = false;
    int* joueurs_score = calloc(p->nb_joueurs, sizeof(int));
    int current_player = 0;
    while(!partie_finie){
        //on fait jouer un coup à current_player
        //on ajoute le score
        //on verifie qu'on a pas fini
        //current_player ++ %= nbjoueur
        printf("\n\n\n\n\n\n\n");
        printf("\n\n<<<<<<< < < <  <  Nouveau Tour  >  > > > >>>>>>>\n\n\n");
        board_print(p);
        info_coup_t * info = jouer_coup(p,current_player);
        int r = info->result;

        if (r >= 0 ){
            joueurs_score[r]++;
            if (joueurs_score[r] >= p->nb_herissons_par_joueurs -1){
                partie_finie = true;
            }
        }

        current_player ++;
        current_player %= p->nb_joueurs;

        free(info);
    }

    board_print(p);
    printf("\n\n<<<<<<< < < <  <  Victoire de Joueur %d  >  > > > >>>>>>>\n\n",current_player);
    free(joueurs_score);
}



int __main(){
    commande_t* c = str_to_cmd("cmd:who\nid:0\n0\nfin");
    if(c == NULL){
        printf("Erreur\n");
        return 1;
    }
    printf("cmd: '%s'\n", c->cmd);
    printf("id: %d\n", c->id);
    printf("nb_args: %d\n", c->nb_args);
    for(int i = 0; i < c->nb_args; i++){
        printf("arg %d: %s\n", i, c->args[i]);
    }

    char* str = cmd_to_str(c);
    printf("str: %s\n", str);
    free(str);

    free_cmd(c);

    return 0;
}

int main(int argc, char** argv){
    srand(time(NULL)); //initialise le générateur de nombre aléatoire, à appeler une seule fois !
    
    printf("\n\n<<<<<<< < < <  <  Bienvenue dans Igel Aergen  >  > > > >>>>>>>\n\n");
    printf("Ce programme permet de jouer à ce jeu de 3 façons différentes\n- En mode classique: tout les joueurs jouent sur la machine\n- En mode multi:\n    - un joueur est le serveur\n    - les autres sont les clients\n");

    int mode;
    if (argc == 2){
        mode = atoi(argv[1]);
    }
    else{
        printf("Entrez le mode de jeu désiré : \n 1: classique    | 2: serveur    | 3: client\n>");
        mode = readInt(2);
    }

    if (mode == 1){
        printf("Vous avez choisi le mode classique !\n");
        info_partie_t* info = demander_info_partie(NB_LIGNES, NB_COLONNES);
        plateau_t* p = initialiser_partie(NB_LIGNES, NB_COLONNES, info);
        printf("\n\n<<<<<<< < < <  <  Plateau initialisé !  >  > > > >>>>>>>\n\n");
        game_loop(p);
        liberer_plateau(p);
        liberer_info_partie(info);
    }
    else{
        printf("\n\n<<<<<<< < < <  <  Vous avez choisi le mode multi  >  > > > >>>>>>>\n\n");
        printf("Entrez le nombre de joueurs:\n>");
        int nb_joueurs = readInt(2);
        printf("Entrez le nombre d'hérissons par joueurs:\n>");
        int nb_herissons_par_joueurs = readInt(2);
        printf("Entrez le port de communication utilisé:\n>");
        int port_int = readInt(5);
        printf("Le port est %d\n", port_int);
        char port[5];
        int_to_ascii(port_int, port);
        printf("int to ascii: %s\n", port);

        printf("Entrez votre numéro de joueur:\n>");
        int joueur = readInt(3);
        
        client_partie_info_t * cinfo = malloc(sizeof(client_partie_info_t));
        cinfo->nb_joueur = nb_joueurs;
        cinfo->nb_herisson_par_joueur = nb_herissons_par_joueurs;
        cinfo->joueur = joueur;
        cinfo->port = port;

        if (mode == 2){
            /*Un joueur dont la machine prends le rôle de serveur va ainsi executer en
            parallèle la fonction de serveur permettant de synchroniser le multijoueur
            ainsi que la fonction de client permettant au joueur de jouer aussi.
            */
            server_partie_info_t * sinfo = malloc(sizeof(server_partie_info_t));
            sinfo->nb_joueur = nb_joueurs;
            sinfo->nb_herisson_par_joueur = nb_herissons_par_joueurs;
            sinfo->port = port;


            cinfo->hostname = "localhost";

            pthread_t tid;
            printf("On lance le thread serveur sur le port %s: \n", sinfo->port);
            pthread_create(&tid,NULL,serveur,sinfo);
            printf("On lance le client: \n");
            client(cinfo);
            pthread_join(tid,NULL);

            free(sinfo);
        }
    
        if (mode == 3){
            printf("Entrez l'adresse (IPV4) du serveur':\n>");
            char host [17] ;
            scanf("%s",host); //c'est déjà un ptr
            cinfo->hostname = host;

            client(cinfo);
        }

        free(cinfo);
    }

    
    return 0;
}


int main_test(){
    client_partie_info_t info_client;
    info_client.nb_joueur = 2;
    info_client.nb_herisson_par_joueur = 1;
    info_client.joueur = 1;
    info_client.port = "8080";
    info_client.hostname = "localhost";
    client2(&info_client);
}

int _main(int argc, char** argv){
    int local = atoi(argv[1]);
    if(local){
        return main_test();
    }
    server_partie_info_t info;
    info.nb_joueur = 2;
    info.nb_herisson_par_joueur = 1;
    info.port = "8080";

    client_partie_info_t info_client;
    info_client.nb_joueur = 2;
    info_client.nb_herisson_par_joueur = 1;
    info_client.joueur = 0;
    info_client.port = "8080";
    info_client.hostname = "localhost";

    pthread_t c, serv;
    printf("On lance le thread serveur: \n");
    pthread_create(&serv, NULL, serv2, &info);
    printf("On lance le thread client: \n");
    client2(&info_client);
    pthread_join(serv, NULL);
    //close(c);
    return 0;
}