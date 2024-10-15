#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "resizableCharArray.h"


//gère la logique du jeu 
#define row = 3
#define columns = 3
#define hedgehogPerTeam = 2
#define nOfTeam 


case_t* creer_case(int ligne, int colonne, int nb_herrisons, char* herrisons){

    rca_t * h = creer_rca(4); //C'est arbitraire, changera de taille au besoin

    for(int i=0; i<nb_herrisons; i++){
        push_rca(h,herissons[i]);
    }

    case_t* c = malloc(sizeof(case_t));
    c->ligne = ligne;
    c->colonne = colonne;
    c->nb_herisson = nb_herrisons;
    c->herissons = h;
    return c;
}

plateau_t creer_plateau(int lignes, int colonnes){
    plateau_t p = malloc(sizeof(plateau_t));
    
}

int main(){

    return 0;
}