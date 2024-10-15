#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "resizableCharArray.h"


//gère la logique du jeu 
#define row = 3
#define columns = 3
#define hedgehogPerTeam = 2
#define nOfTeam


case_t* creer_case(int ligne, int colonne, int nb_herissons, char* herissons){

    rca_t * h = creer_rca(4); //C'est arbitraire, changera de taille au besoin

    for(int i=0; i<nb_herissons; i++){
        push_rca(h,herissons[i]);
    }

    case_t* c = malloc(sizeof(case_t));
    c->ligne = ligne;
    c->colonne = colonne;
    c->nb_herisson = nb_herissons;
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
    push_rca(p->cases[line][row],ctn);
}

char board_pop(plateau_t* p, int line, int row){
    pop_rca(p->cases[line][row])
}

int board_height(plateau_t*p, int line, int row){
    return(p->cases[line][row])->size;
}

char board_top (plateau_t*p, int line, int row){
    peek_rca(p->cases[line][row],0)
}

char board_peek(plateau_t*p, int line, int row, int pos){
    peek_rca(p->cases[line][row],pos)
}

void initialiser_partie(plateau_t* p, int nb_joueurs, int){

}


//TODO écrire un scanf correcte qui fail si ce n'est pas un entier !
int** demander_info_partie(){
    int nb_joueurs;
    printf("Nombre de joueurs:\n>");
    while(scanf("%d", &nb_joueurs) == EOF){
        printf("Erreur, veuillez indiquer un nombre !\n");
    }

    printf("nombre de joueurs: %d", nb_joueurs);
    return NULL;
}

int main(){
    demander_info_partie();
    return 0;
}