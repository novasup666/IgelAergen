#include <stdlib.h>
#include "resizableCharArray.h"


rca_t* creer_rca(int taille){
    rca_t * res = malloc(sizeof(rca_t));
    char*t = malloc(sizeof(char)*taille);
    res->size = 0;
    res->capacity  = taille;
    res->tab = t;
    return res;
}

void resize_rca(rca_t*rca){
    char * nt = malloc(rca->capacity * 2 * sizeof(char));
    for (int i = 0; i < rca->capacity; i++){
        nt[i] = rca->tab[i];
    }
    free(rca->tab);
    rca->tab = nt;
}

void push_rca(rca_t *rca,char valeur){
    if (rca->size >= rca->capacity){
        resize_rca(rca);
    }
    rca->tab[rca->size] = valeur;
    rca->size ++;
}


char pop_rca(rca_t*rca){
    rca->size --;
    return rca->tab[rca->size];
}

char peek_rca(rca_t*rca, int pos){
    return rca->tab[rca->size-(1+pos)];
}


void liberer_rca(rca_t*rca){
    free(rca->tab);
    free(rca);
}