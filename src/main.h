#ifndef MAIN_H
#define MAIN_H

struct case_s{
    int ligne;
    int colonne; 
    int nb_herisson;
    char* herissons;

};
typedef struct case_s case_t;

struct plateau_s{
    int nb_lignes;
    int nb_colonnes;
    case_t* case_t;
};
typedef struct plateau_s plateau_t;

#endif