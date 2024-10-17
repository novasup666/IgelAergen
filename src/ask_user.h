#ifndef ASK_USER_H
#define ASK_USER_H
#include "gametype.h"

//Gère les I/O avec l'utilisateur

#define TAILLE_MAX_ENTIER 3 //pour eviter les depassements de capacité


info_partie_t* demander_info_partie(int nb_lignes, int nb_colonnes);
int demander_coo_plateau(int joueur, int max, bool is_ligne, bool mandatory);
coo_t* demander_coo(plateau_t *p, int joueur, bool must_have_herisson, bool mandatory);
void liberer_info_partie(info_partie_t* info);
int demander_deplacement(plateau_t *p, int joueur);









#endif