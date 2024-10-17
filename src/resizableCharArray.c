#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "resizableCharArray.h"


char board_top_debug (rca_t* h){
    return is_empty_rca(h) ? 0 : peek_rca(h, 0);
}


rca_t* creer_rca(int taille){
    rca_t * res = calloc(1, sizeof(rca_t));
    char*t = calloc(taille, sizeof(char));
    res->size = 0;
    res->capacity  = taille;
    res->tab = t;
    return res;
}

void resize_rca(rca_t*rca){
    char * nt = calloc((rca->capacity * 2 +1), sizeof(char));
    for (int i = 0; i < rca->capacity; i++){
        nt[i] = rca->tab[i];
    }
    free(rca->tab);
    rca->tab = nt;
    rca->capacity = 2 * (rca->capacity) + 1;
}

void push_rca(rca_t *rca,char valeur){
    if (rca->size >= rca->capacity){
        resize_rca(rca);
    }
    rca->tab[rca->size] = valeur;
    rca->size ++;
}


char pop_rca(rca_t*rca){
    assert(rca->size>0);
    rca->size --;
    return rca->tab[rca->size];
}

bool is_empty_rca(rca_t*rca){
    return rca->size == 0;
}

char peek_rca(rca_t*rca, int pos){
    assert(rca->size>pos);
    return rca->tab[rca->size-(1+pos)];
}


void liberer_rca(rca_t*rca){
    free(rca->tab);
    free(rca);
}
