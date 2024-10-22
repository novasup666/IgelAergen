#ifndef MAIN_H
#define MAIN_H
#include "gametype.h"

#include "resizableCharArray.h"
#include "util.h"
#include "ask_user.h"
#include "multi.h"


#define NB_LIGNES 6
#define NB_COLONNES 9

char board_top (plateau_t*p, int line, int col);
char board_pop(plateau_t* p, int line, int col);
void board_push(plateau_t* p, int line, int col, char ctn);

char player_to_herisson(int joueur);
//fonction pour verifier que les herissons sont bien placés
bool is_herisson_on_case(plateau_t* p, int joueur, int ligne, int colonne);
bool is_herisson_traped(plateau_t* p, int joueur, int ligne, int colonne);
bool is_herisson_last(plateau_t* p, int joueur, int ligne, int colonne);

//fonction d'intialisation de la partie
plateau_t* creer_plateau(int nb_lignes, int nb_colonnes);
void liberer_plateau(plateau_t* p);

//fonction du jeu
info_coup_t* jouer_coup(plateau_t *p, int joueur);

//fonction d'affichage
void board_print(plateau_t * p);

//fonction util
char player_to_herisson(int joueur);
int herisson_to_player(char herisson);

#endif