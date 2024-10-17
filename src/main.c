#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "resizableCharArray.h"
#include "util.h"

//définit la taille du buffer pour les entiers
#define TAILLE_MAX_ENTIER 3 //pour eviter les depassements de capacité

//TODO remplacer ca par la lecture des arguments du main
#define NB_LIGNES 3
#define NB_COLONNES 3
//gère la logique du jeu 

// /!\ ON STOCKERA DES MINUSCULES !!!!!!

case_t* creer_case(int ligne, int colonne, int nb_herissons, char* herissons){

    rca_t * h = creer_rca(nb_herissons); 
    for(int i=0; i<nb_herissons; i++){
        push_rca(h,herissons[i]);
    }

    case_t* c = malloc(sizeof(case_t));
    c->ligne = ligne;
    c->colonne = colonne;
    c->herissons = h;
    return c;
}

plateau_t* creer_plateau(int nb_lignes, int nb_colonnes){
    plateau_t *p = malloc(sizeof(plateau_t));
    p->nb_lignes = nb_lignes;
    p->nb_colonnes = nb_colonnes;
    p->cases = malloc(sizeof(case_t)*nb_lignes*nb_colonnes);
    for(int i = 0; i< nb_lignes*nb_colonnes; i++){
        int ligne = i/nb_colonnes;
        int colonne = i%nb_lignes;
        p->cases[i] = creer_case(ligne, colonne, 0, NULL); //TODO changer Null en le bon argument
    }

    return p;
}

int lancer_de(){
    return rand()%6 + 1;
}


//ATTENTION, valgrind kiff pas du tout ces fonctions !

void board_push(plateau_t* p, int line, int row, char ctn){
    push_rca((p->cases[line][row]).herissons,ctn);
}

char board_pop(plateau_t* p, int line, int row){
    return pop_rca((p->cases[line][row]).herissons);
}

int board_height(plateau_t*p, int line, int row){
    return((p->cases[line][row]).herissons)->size;
}

char board_top (plateau_t*p, int line, int row){
    return peek_rca((p->cases[line][row]).herissons,0);
}

char board_peek(plateau_t*p, int line, int row, int pos){
    return peek_rca((p->cases[line][row]).herissons,pos);
}




//attention faut faire gaffe aux "\n", mais on peu laisser ça à print_plateau
void cell_print(plateau_t* p, int line, int row, int slice){
    char top;
    char lb;
    char rb;
    char bot;
    if((p->cases[line][row]).is_piege){
        top = 'V';
        lb = '>';
        rb = '<';
        bot= '^';
    }else{
        top = '_';
        lb = '|';
        rb = '|';
        bot= '_';
    }
    if (slice == 0){
        printf("  %c%c%c  ", top,top,top);
    }
    if (slice == 1){
        if (board_height(p,line,row) == 0){
            printf(" %c   %c ",lb,rb);
        }
        else{
            char team = board_top(p,line,row);
            printf(" %c%c%c%c%c ",lb,(team  - 0x20),(team  - 0x20),(team  - 0x20),rb); //attention c'est pas un + mais un - (a-0x20=A)
        }
    }
    if (slice == 2){
        if (board_height(p,line,row) == 0){
            printf(" %c   %c ",lb,rb);
        }
        if(board_height(p,line,row) == 1){
            char team = board_top(p,line,row);
            printf(" %c%c%c%c%c ",lb,(team),(team),(team),rb);
        }    
        if(board_height(p,line,row) == 2){
            char team = board_peek(p,line,row,1);
            printf(" %c %c %c ",lb,(team ),rb);
        }  
        if(board_height(p,line,row) == 3){
            char team1 = board_peek(p,line,row,1);
            char team2 = board_peek(p,line,row,2);
            printf(" %c%c %c%c ",lb,(team1),(team2),rb);
        }  
        if(board_height(p,line,row) == 4){
            char team1 = board_peek(p,line,row,1);
            char team2 = board_peek(p,line,row,2);
            char team3 = board_peek(p,line,row,3);
            printf(" %c%c%c%c%c ",lb,(team1),(team2), (team3),rb);
        } 
    }
    if (slice == 3){
        int n = board_height(p,line,row);
        if (n>1){
            printf("  %c%d%c  ",bot,n,bot);
        }
        else{
        printf("  %c%c%c  ",bot,bot,bot);}
    }
    
}

void board_print(plateau_t * p){
    printf("       START ");
    for(int i = 0; i<(p->nb_colonnes)-2; i++){
        printf("       ");
    }
    printf("FINISH \n\n");
    printf("       ");
    for(int i = 0; i<(p->nb_colonnes); i++){
        printf("  col  ");
    }
    printf("\n");
    printf("       ");
    for(int i = 0; i<(p->nb_colonnes); i++){
        printf("   %c   ", (97+i));
    }   
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
    p->nb_herrisons_par_joueurs = info->nb_herissons_par_joueurs;
    //place les herissons correctements sur la 1er ligne
    for(int joueur = 0; joueur < info->nb_joueurs; joueur++){
        for(int herisson = 0; herisson < info->nb_herissons_par_joueurs; herisson++){
            //board_push(p, 0, info->placement_herissons[joueur][herisson], player_to_herisson(joueur));
        }
    }
    return p;
}


//todo couper ça en 3 fonctions ou bien utiliser une struct
info_partie_t* demander_info_partie(){
    info_partie_t* info = malloc(sizeof(info_partie_t));
    
    int nb_joueurs = -1;
    printf("Nombre de joueurs (max 27):\n>");
    while((nb_joueurs = readInt(2)) <= 0 || nb_joueurs > 27){ //ici on utilise 2 car on veut un nombre entre 0 et 27
        printf("Erreur, veuillez indiquer un nombre entre 1 et 27!\n");
        printf("Nombre de joueurs (max 27):\n>");
    }
    printf("Nombre de joueurs: %d\n", nb_joueurs);

    int nb_herrisons_par_joueurs;
    printf("Nombre d'herissons par joueurs (max 999):\n>");
    while((nb_herrisons_par_joueurs = readInt(TAILLE_MAX_ENTIER)) <= 0){
        printf("Erreur, veuillez indiquer un nombre positif!\n");
        printf("Nombre d'herissons par joueurs (max 999):\n>");

    }
    printf("Nombre d'herissons par joueurs: %d\n", nb_herrisons_par_joueurs);

    //info va être un tableau 2D qui contient à la case i les positions des hérissons du joueurs i
    int **placement_herisson = malloc(sizeof(int*)*nb_joueurs); //DEBUG merci valgrind
    for(int j = 0; j<nb_joueurs; j++){
        placement_herisson[j] = malloc(sizeof(int)*nb_herrisons_par_joueurs);
    }
    
    for(int joueur = 0; joueur < nb_joueurs; joueur++){
        printf("=========================================================\n");
        printf("Joueurs %d: Vous allez placer vos herissons de depart !\n", joueur);
        for(int herisson = 0; herisson<nb_herrisons_par_joueurs; herisson++){
            printf("Placer le herisson %d:\n>", herisson);
            int c;
            while((c=readInt(TAILLE_MAX_ENTIER)) <0){
                printf("Erreur, joueur %d, veuillez indiquer un nombre positif !\n", joueur);
            }
            placement_herisson[joueur][herisson]=c;
        }
    }
    info->nb_joueurs = nb_joueurs;
    info->nb_herissons_par_joueurs = nb_herrisons_par_joueurs;
    info->placement_herissons = placement_herisson;
    return info;
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

bool is_herisson_traped(plateau_t* p, int joueur, int ligne, int colonne){

}

bool is_herisson_on_case(plateau_t* p, int joueur, int ligne, int colonne){
    return board_top(p, ligne, colonne) == player_to_herisson(joueur);
}

int demander_coo_plateau(int joueur, int max, bool is_ligne){
    int r;
    bool choix_valide = false;
    const char* type = is_ligne ? "ligne" : "colonne";

    while(!choix_valide){
        printf("Choisissez une %s entre 0 et %d:\n>", type, max-1);
        r = readInt(TAILLE_MAX_ENTIER);
        if(r <0){
            printf("Erreur, veuillez indiquer un nombre positif !\n");
        }
        else if(r >= max){
            printf("Erreur, veuillez indiquer un nombre inférieur à %d !\n", max-1);
        }else{
            choix_valide = true;
        }
    }
    return r;
}

coo_t* demander_coo(plateau_t *p, int joueur, bool must_have_herisson){
    int ligne;
    int colonne;
    bool choix_valide = false;
    printf("=========================================================\n");
    while(!choix_valide){
        printf("Le joueur %d: Choisissez un herisson:\n>", joueur);
        ligne = demander_coo_plateau(joueur, p->nb_lignes, true);
        colonne = demander_coo_plateau(joueur, p->nb_colonnes, false);
        if(must_have_herisson && !is_herisson_on_case(p, joueur, ligne, colonne)){
            printf("Erreur, veuillez indiquer une case où vous avez un herisson!\n");
        }
        else{
            choix_valide = true;
        }
    }
    coo_t *c = malloc(sizeof(coo_t));
    c->ligne = ligne;
    c->colonne = colonne;
    return c;
}

void demander_info_coup(int joueur){
    printf("Joueur %d, à vous de jouer !\n", joueur);
    //on tire le dé
    int de = lancer_de();
    printf("Vous avez tiré un %d !\n", de);



    //on demande au joueur de choisir un herisson
    

}


int main(){
    srand(time(NULL)); //initialise le générateur de nombre aléatoire, à appeler une seule fois !
    
    /*
    case_t c = creer_case(1,2,3,{'a';'b';'c'});
    for(int i = 0; i < 4; i++){
        cell_print()
    }
    */


    info_partie_t* info = demander_info_partie();


    plateau_t* p = initialiser_partie(NB_LIGNES, NB_COLONNES, info);
    printf("Plateau initialisé !\n");
    //coo_t* c = demander_coo(p, 0, true);
    //printf("Vous avez choisi la case (%d, %d)\n", c->ligne, c->colonne);
    //free(c);

    printf("hehe\n");
    cell_print(p, 0, 0, 0);
    printf("\n");
    cell_print(p, 0, 0, 1);
    printf("\n");
    cell_print(p, 0, 0, 2);
    printf("\n");
    cell_print(p, 0, 0, 3);
    printf("ahha\n");
    liberer_plateau(p);

    free(info);
    return 0;
}