#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "main.h"


//définit la taille du buffer pour les entiers

//TODO remplacer ca par la lecture des arguments du main
#define NB_LIGNES 3
#define NB_COLONNES 5
//gère la logique du jeu 

// /!\ ON STOCKERA DES MINUSCULES !!!!!!

case_t* creer_case(int ligne, int colonne, int nb_herissons, char* herissons){

    rca_t * h = creer_rca(1); 
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
    plateau_t *p = calloc(1, sizeof(plateau_t));
    p->nb_lignes = nb_lignes;
    p->nb_colonnes = nb_colonnes;
    p->cases = calloc(nb_lignes*nb_colonnes, sizeof(case_t));
    for(int i = 0; i< nb_lignes*nb_colonnes; i++){
        int ligne = i/nb_colonnes;
        int colonne = i%nb_lignes;
        p->cases[i] = creer_case(ligne, colonne, 0, NULL); //TODO changer Null en le bon argument. NULL est le bon argument pour l'initialisation
    }

    return p;
}

int lancer_de(){
    return rand()%6 + 1;
}


//ATTENTION ca bug encore
//TODO remove printf
void board_push(plateau_t* p, int line, int col, char ctn){
    printf("A %d %d=========================================================\n", line, col);
    printf("Debug: (1,3): %c\n", board_top(p, 1, 3));
    printf("Debug: (0,2) %c\n", board_top(p, 2, 0));
    push_rca((p->cases[line* (p->nb_colonnes) +col])->herissons,ctn); 
        printf("B=========================================================\n");
    printf("Debug: (1,3): %c\n", board_top(p, 1, 3));
    printf("Debug: (0,2) %c\n", board_top(p, 2, 0));
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

bool board_is_empty(plateau_t*p, int line, int col){
    return board_top(p, line, col) == 0;
}




//attention faut faire gaffe aux "\n", mais on peu laisser ça à print_plateau
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
        if(board_height(p,line,col) == 4){
            char team1 = board_peek(p,line,col,1);
            char team2 = board_peek(p,line,col,2);
            char team3 = board_peek(p,line,col,3);
            printf(" %c%c%c%c%c ",lb,(team1),(team2), (team3),rb);
        } 
    }
    if (slice == 3){
        int n = board_height(p,line,col);
        if (n>1){
            printf("  %c%d%c  ",bot,n,bot);
        }
        else{
        printf("  %c%c%c  ",bot,bot,bot);}
    }
    
}

void board_print(plateau_t * p){
   
    printf("        START ");
    for(int i = 0; i<(p->nb_colonnes)-2; i++){
        printf("       ");
    }
    printf("FINISH \n");
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
            if(s == 1){printf("  row  ");}
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
    p->nb_herrisons_par_joueurs = info->nb_herissons_par_joueurs;
    //place les herissons correctements sur la 1er ligne
    for(int joueur = 0; joueur < info->nb_joueurs; joueur++){
        for(int herisson = 0; herisson < info->nb_herissons_par_joueurs; herisson++){
            printf("Debug: joueur %d, herisson %d\n", joueur, herisson); //TODO remove comment, mais il confirme que l'erreur de placement n'est pas ici
            printf("valeur: %d\n", info->placement_herissons[joueur][herisson]);
            board_push(p, info->placement_herissons[joueur][herisson] , 0, player_to_herisson(joueur));
            printf("=========================================================\n");
            printf("Debug: (1,3): %c\n", board_top(p, 1, 3));
            printf("Debug: (0,2) %c\n", board_top(p, 2, 0));

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
bool is_herisson_traped(plateau_t* p, int joueur, int ligne, int colonne){

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

int jouer_coup(plateau_t *p, int joueur){
    int de = lancer_de();
    printf("Le joueur %d a lancé un %d et peu donc faire avancer un herisson de la ligne %d\n", joueur, de, de);
    
    //on laisse le joueur deplacer un herisson verticalement si il le désire
    bool choix_vertical_valide = false;
    while(!choix_veritcal_valide){
        printf("Selectionnez un herisson à déplacer verticalement:\n");
        coo_t* c = demander_coo(p, joueur, true, false);
        if(c==NULL){
            printf("Joueur %d, vous avez choisi de ne pas déplacer de herisson verticalement\n", joueur);
            choix_vertical_valide = true;
        }
        else{
            int deplacement = demander_depalcement(p, joueur); //1=haut 2=bas
            if(deplacement == 1){
                if(c->ligne <= 0){
                    printf("Erreur, vous ne pouvez pas déplacer le herisson %d vers le haut\n", joueur);
                }
                else{
                    board_push(p, c->ligne - 1, c->colonne, player_to_herisson(joueur));
                    board_pop(p, c->ligne, c->colonne);
                    choix_vertical_valide = true;
                }
            }
            else{
                if(c->ligne >= p->nb_lignes){
                    printf("Erreur, vous ne pouvez pas déplacer le herisson %d vers le bas\n", joueur);
                }
                else{
                    board_push(p, c->ligne + 1, c->colonne, player_to_herisson(joueur));
                    board_pop(p, c->ligne, c->colonne);
                    choix_vertical_valide = true;
                }
            }
        }
    }

    //TODO: ici on doit input une colonne, vérifier qu'il y a un herisson, et le déplacer 
    //OU rien input et verifier qu'il n'y a pas de herisson à déplacer sur la ligne du dé

    //le joueur doit ensuite déplacer un herisson horizontalement de la ligne déterminée par le dé
    bool choix_horizontal_valide = false;
    while(!choix_horizontal_valide){
        printf("Joueur %d: Selectionnez un herisson à déplacer horizontalement sur la ligne %d:\n", joueur, de);
        int colonne = demander_coo_plateau(joueur, p->nb_colonnes, false, true);
        //on vérifie qu'il y a bien un herisson à déplacer
        break;
    }

}








int main(){
    srand(time(NULL)); //initialise le générateur de nombre aléatoire, à appeler une seule fois !
    
    /*
    case_t c = creer_case(1,2,3,{'a';'b';'c'});
    for(int i = 0; i < 4; i++){
        cell_print()
    }
    */


    info_partie_t* info = demander_info_partie(NB_LIGNES, NB_COLONNES);


    plateau_t* p = initialiser_partie(NB_LIGNES, NB_COLONNES, info);
    printf("\n\n<<<<<<< < < <  <  Plateau initialisé !  >  > > > >>>>>>>\n\n");
    //coo_t* c = demander_coo(p, 0, true);
    //printf("Vous avez choisi la case (%d, %d)\n", c->ligne, c->colonne);
    //free(c);
    


    board_print(p);
    
    printf("=========================================================\n");
    printf("Debug: (1,3): %c\n", board_top(p, 1, 3));
    printf("Debug: (0,2) %c\n", board_top(p, 2, 0));
    
    liberer_plateau(p);
    liberer_info_partie(info);
    return 0;
}

//TODO y'a un pb
/*
Quand on test 
1 joueur
1 herisson
qu'on le met à la ligne 2
ça affiche 2 herissons en (0,2) et (1,3)
En revanche il n'y a pas de bug si le plateau est un carré 
*/