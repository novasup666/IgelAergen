#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "util.h"
#include "main.h"
#include "ask_user.h"

info_placement_herisson_t * demander_placement_herisson(int joueur, int nb_herissons, int nb_lignes){
    info_placement_herisson_t * info = malloc(sizeof(info_placement_herisson_t));
    info->joueur = joueur;
    info->nb_herissons = nb_herissons;
    info->lignes = malloc(nb_herissons*sizeof(int));
    printf("Joueur %d, vous allez placer vos herissons de depart !\n", joueur);
    for(int i = 0; i<nb_herissons; i++){
        printf("Placer le herisson %d:\n>", i);
        int c = -1;
        while((c=readInt(TAILLE_MAX_ENTIER)) <0 || c >= nb_lignes){
            printf("Erreur, joueur %d, veuillez indiquer un nombre entre 0 et %d !\n", joueur, nb_lignes-1);
            printf("Placer le herisson %d:\n>", i);
        }
        info->lignes[i] = c;
    }
    return info;
}

info_partie_t* demander_info_partie(int nb_lignes, int nb_colonnes){
    info_partie_t* info = calloc(1, sizeof(info_partie_t));
    
    int nb_joueurs = -1;
    printf("Nombre de joueurs (max 27):\n>");
    while((nb_joueurs = readInt(2)) <= 0 || nb_joueurs > 27){ //ici on utilise 2 car on veut un nombre entre 0 et 27
        printf("Erreur, veuillez indiquer un nombre entre 1 et 27!\n");
        printf("Nombre de joueurs (max 27):\n>");
    }
    printf("Nombre de joueurs: %d\n", nb_joueurs);

    int nb_herrisons_par_joueurs = -1;
    printf("Nombre d'herissons par joueurs (max 999):\n>");
    while((nb_herrisons_par_joueurs = readInt(TAILLE_MAX_ENTIER)) <= 0){
        printf("Erreur, veuillez indiquer un nombre positif!\n");
        printf("Nombre d'herissons par joueurs (max 999):\n>");

    }
    printf("Nombre d'herissons par joueurs: %d\n", nb_herrisons_par_joueurs);

    //info va être un tableau 2D qui contient à la case i les positions des hérissons du joueurs i
    int **placement_herisson = calloc(nb_joueurs, sizeof(int*)); //DEBUG merci valgrind
    for(int j = 0; j<nb_joueurs; j++){
        placement_herisson[j] = calloc(nb_herrisons_par_joueurs, sizeof(int));
    }
    
    for(int joueur = 0; joueur < nb_joueurs; joueur++){
        printf("=========================================================\n");
        
            info_placement_herisson_t * info_herisson = demander_placement_herisson(joueur, nb_herrisons_par_joueurs, nb_lignes);
            for(int herisson = 0; herisson < nb_herrisons_par_joueurs; herisson++){
                placement_herisson[joueur][herisson] = info_herisson->lignes[herisson];
            }
            free(info_herisson->lignes);
            free(info_herisson);
        }

    info->nb_joueurs = nb_joueurs;
    info->nb_herissons_par_joueurs = nb_herrisons_par_joueurs;
    info->placement_herissons = placement_herisson;
    return info;
}


int ask_exit_choice(int joueur){
    printf("Joueur %d, ce choix est optionel: 0 pour ne rien faire, 1 faire un choix\n>",joueur);
    int c = readInt(TAILLE_MAX_ENTIER);
    return c;
}

int demander_coo_plateau(int joueur, int max, bool is_ligne, bool mandatory){
    int r;
    bool choix_valide = false;
    const char* type = is_ligne ? "ligne" : "colonne";

    while(!choix_valide){
        if(!mandatory && !ask_exit_choice(joueur)){
            return -1;
        }
        printf("Choisissez une %s entre 0 et %d:\n>", type, max-1);
        r = readInt(TAILLE_MAX_ENTIER);
        if(r <0){
            printf("Erreur, veuillez indiquer un nombre positif !\n");
        }
        else if(r >= max){
            printf("Erreur, veuillez indiquer un nombre inférieur à %d !\n", max-1);
        }else{
            choix_valide = true;
        }
    }
    return r;
}

//TODO
bool exists_herisson_deplaceable(plateau_t *p, int joueur, int de){

}



//attention, peut renvoyer NULL si !mandatory et que le joueur ne veut pas jouer
coo_t* demander_coo(plateau_t *p, int joueur, bool must_have_herisson, bool mandatory){
    int ligne;
    int colonne;
    bool choix_valide = false;
    // sont moches les égals là
    //printf("=========================================================\n");
    while(!choix_valide){
        if(!mandatory && !ask_exit_choice(joueur)){
            return NULL;
        }
        printf("Le joueur %d: Choisissez un herisson:\n>", joueur);

        ligne = demander_coo_plateau(joueur, p->nb_lignes, true, true);
        colonne = demander_coo_plateau(joueur, p->nb_colonnes, false, true);
        if(must_have_herisson && !is_herisson_on_case(p, joueur, ligne, colonne)){
            printf("Erreur, veuillez indiquer une case dominée par un de vos herisson !\n");
            if(!mandatory && !ask_exit_choice(joueur)){ //on repete ça au cas ou le joueur ne peux pas choisir une case avec un de ses herissons, pour eviter qu'il soit bloqué
                return NULL;
            }
        }
        else{
            choix_valide = true;
        }
    }
    coo_t *c = calloc(1, sizeof(coo_t));
    c->ligne = ligne;
    c->colonne = colonne;
    return c;
}

int demander_deplacement(plateau_t *p, int joueur){
    bool choix_valide = false;
    int deplacement = -1;
    while(deplacement != 1 && deplacement != 2){
        printf("Joueur %d, voulez vous déplacer vers le haut (1) ou vers le bas (2) ?\n>", joueur);
        deplacement = readInt(TAILLE_MAX_ENTIER);
        if(deplacement != 1 && deplacement != 2){
            printf("Erreur, veuillez indiquer 1 ou 2 !\n");
        }
    }
    return deplacement;
}





void liberer_info_partie(info_partie_t* info){
    for(int i = 0; i<info->nb_joueurs; i++){
        free(info->placement_herissons[i]);
    }
    free(info->placement_herissons);
    free(info);
}