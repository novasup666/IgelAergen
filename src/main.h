#ifndef MAIN_H
#define MAIN_H
#include <stdbool.h>
#include "resizableCharArray.h"


struct case_s{
    int ligne;
    int colonne; 
    int nb_herisson;
    bool is_piege;
    rca_t * herissons;

};
typedef struct case_s case_t;

struct plateau_s{
    int nb_lignes;
    int nb_colonnes;
    case_t** cases;
};
typedef struct plateau_s plateau_t;

#endif