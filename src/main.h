#ifndef MAIN_H
#define MAIN_H
#include "gametype.h"

#include "resizableCharArray.h"
#include "util.h"
#include "ask_user.h"

char board_top (plateau_t*p, int line, int col);



//fonction pour verifier que les herissons sont bien placés
bool is_herisson_on_case(plateau_t* p, int joueur, int ligne, int colonne);
bool is_herisson_traped(plateau_t* p, int joueur, int ligne, int colonne);
bool is_herisson_last(plateau_t* p, int joueur, int ligne, int colonne);

#endif