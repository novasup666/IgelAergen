#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "resizableCharArray.h"
#include "util.h"

//définit la taille du buffer pour les entiers
#define TAILLE_MAX_ENTIER 10
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
        p->cases[i] = creer_case(ligne, colonne, 0, NULL); //TODO changer void en le bon argument
    }

    return p;
}

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





void cell_print(plateau_t* p, int line, int row, int slice){
    // ne pas oublier les cardinaux en bas des cases

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
        printf(" %c%c%c ", top,top,top);
    }
    if (slice == 1){
        if (board_height(p,line,row) == 0){
            printf("%c   %c",lb,rb);
        }
        else{
            char team = board_top(p,line,row);
            printf("%c%c%c%c%c",lb,(team  + 0x20),(team  + 0x20),(team  + 0x20),rb);
        }
    }
    if (slice == 2){
        if (board_height(p,line,row) == 0){
            printf("%c   %c",lb,rb);
        }
        if(board_height(p,line,row) == 1){
            char team = board_top(p,line,row);
            printf("%c%c%c%c%c",lb,(team),(team),(team),rb);
        }    
        if(board_height(p,line,row) == 2){
            char team = board_peek(p,line,row,1);
            printf("%c %c %c",lb,(team ),rb);
        }  
        if(board_height(p,line,row) == 3){
            char team1 = board_peek(p,line,row,1);
            char team2 = board_peek(p,line,row,2);
            printf("%c%c %c%c",lb,(team1),(team2),rb);
        }  
        if(board_height(p,line,row) == 4){
            char team1 = board_peek(p,line,row,1);
            char team2 = board_peek(p,line,row,2);
            char team3 = board_peek(p,line,row,3);
            printf("%c%c%c%c%c",lb,(team1),(team2), (team3),rb);
        } 
    }
    if (slice == 3){
        int n = board_height(p,line,row);
        if (n>1){
            printf(" %c%d%c ",bot,n,bot);
        }
        else{
        printf(" %c%c%c ",bot,bot,bot);}
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
            board_push(p, 0, info->placement_herissons[joueur][herisson], player_to_herisson(joueur));
        }
    }
    return p;
}


//todo couper ça en 3 fonctions ou bien utiliser une struct
info_partie_t* demander_info_partie(){
    info_partie_t* info = malloc(sizeof(info_partie_t));
    
    int nb_joueurs;
    printf("Nombre de joueurs (max 27):\n>");
    while((nb_joueurs = readInt(10)) <= 0 && nb_joueurs <= 27){
        printf("Erreur, veuillez indiquer un nombre positif!\n");
    }

    int nb_herrisons_par_joueurs;
    printf("Nombre d'herissons par joueurs (max 99):\n>");
    while((nb_herrisons_par_joueurs = readInt(10)) <= 0){
        printf("Erreur, veuillez indiquer un nombre positif!\n");
    }

    //info va être un tableau 2D qui contient à la case i les positions des hérissons du joueurs i
    int **placement_herisson = malloc(sizeof(int)*nb_joueurs);
    for(int i = 0; i<nb_joueurs; i++){
        placement_herisson[i] = malloc(sizeof(int)*nb_herrisons_par_joueurs);
    }
    
    for(int joueur = 0; joueur < nb_joueurs; joueur++){
        printf("=========================================================");
        printf("\nJoueurs %d: Vous allez placer vos herissons de depart !\n", joueur);
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

int main(){
    info_partie_t* info = demander_info_partie();
    return 0;
}