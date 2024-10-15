#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "resizableCharArray.h"
#include "util.h"


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
    pop_rca((p->cases[line][row]).herissons);
}

int board_height(plateau_t*p, int line, int row){
    return((p->cases[line][row]).herissons)->size;
}

char board_top (plateau_t*p, int line, int row){
    peek_rca((p->cases[line][row]).herissons,0);
}

char board_peek(plateau_t*p, int line, int row, int pos){
    peek_rca((p->cases[line][row]).herissons,pos);
}

void normal_cell_print(plateau_t* p, int line, int row, int slice){
    if (slice == 0){
        printf(" ___ ");
    }
    if (slice == 1){
        if (board_height(p,line,row) == 0){
            printf("|   |");
        }
        else{
            char team = board_top(p,line,row);
            printf("|%c%c%c|",(team  + 0x20),(team  + 0x20),(team  + 0x20));
        }
    }
    if (slice == 2){
        if (board_height(p,line,row) == 0){
            printf("|   |");
        }
        if(board_height(p,line,row) == 1){
            char team = board_top(p,line,row);
            printf("|%c%c%c|",(team),(team,(team));
        }    
        if(board_height(p,line,row) == 2){
            char team = board_peek(p,line,row,1);
            printf("| %c |",(team ));
        }  
        if(board_height(p,line,row) == 3){
            char team1 = board_peek(p,line,row,1);
            char team2 = board_peek(p,line,row,2);
            printf("|%c %c|",(team1),(team2);
        }  
        if(board_height(p,line,row) == 4){
            char team1 = board_peek(p,line,row,1);
            char team2 = board_peek(p,line,row,2);
            char team3 = board_peek(p,line,row,3);
            printf("|%c%c%c|",(team1),(team2), (team3));
        } 
    }
    if (slice == 3){
        printf(" ___ ")
    }
}

void trapped_cell_print(plateau_t* p, int line, int row, int slice){
    if (slice == 0){
        printf(" vvv ");
    }
    if (slice == 1){
        if (board_height(p,line,row) == 0){
            printf(">   <");
        }
        else{
            char team = board_top(p,line,row);
            printf(">%c%c%c<",(team  + 0x20),(team  + 0x20),(team  + 0x20));
        }
    }
    if (slice == 2){
        if (board_height(p,line,row) == 0){
            printf(">   <");
        }
        if(board_height(p,line,row) == 1){
            char team = board_top(p,line,row);
            printf(">%c%c%c<",(team),(team,(team));
        }    
        if(board_height(p,line,row) == 2){
            char team = board_peek(p,line,row,1);
            printf("> %c <",(team ));
        }  
        if(board_height(p,line,row) == 3){
            char team1 = board_peek(p,line,row,1);
            char team2 = board_peek(p,line,row,2);
            printf(">%c %c<",(team1),(team2);
        }  
        if(board_height(p,line,row) == 4){
            char team1 = board_peek(p,line,row,1);
            char team2 = board_peek(p,line,row,2);
            char team3 = board_peek(p,line,row,3);
            printf("|%c%c%c|",(team1),(team2), (team3));
        } 
    }
    if (slice == 3){
        printf(" ^^^ ")
    }
}


void cell_print(plateau_t* p, int line, int row, int slice){
    if ((p.cases[line][rox])->is_piege){
        trapped_cell_print(p,line,row,slice);
    }
    else {
        normal_cell_print(p,line,row,slice);
    }
    
}

void initialiser_partie(plateau_t* p, int nb_joueurs, int nb_herrisons_par_joueurs){

}



int** demander_info_partie(){
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
    int **info = malloc(sizeof(int)*nb_joueurs);
    for(int i = 0; i<nb_joueurs; i++){
        info[i] = malloc(sizeof(int)*nb_herrisons_par_joueurs);
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
            info[joueur][herisson]=c;
        }
    }
    return info;
}

int main(){
    demander_info_partie();
    return 0;
}