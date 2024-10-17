#ifndef GAMETYPE_H
#define GAMETYPE_H

#include "resizableCharArray.h"

struct case_s{
    int ligne;
    int colonne; 
    bool is_piege;
    rca_t * herissons;

};
typedef struct case_s case_t;

struct plateau_s{
    int nb_lignes;
    int nb_colonnes;
    int nb_joueurs;
    int nb_herrisons_par_joueurs;
    case_t** cases; //tableau 1D de cases* !!!
};
typedef struct plateau_s plateau_t;


struct coo_s{
    int ligne;
    int colonne;
};
typedef struct coo_s coo_t;


struct info_partie_s{
    int nb_joueurs;
    int nb_herissons_par_joueurs;
    int** placement_herissons; //tableau 1D de tableau 1D d'entiers
};
typedef struct info_partie_s info_partie_t;


#endif