#ifndef RCA_H
#define RCA_H


struct rca_s{
    int size;
    int capacity;
    char * tab;
};

typedef struct rca_s rca_t;
rca_t* creer_rca(int taille);

void push_rca(rca_t *rca,char valeur);
char pop_rca(rca_t*rca);
char peek_rca(rca_t*rca, int pos);
void liberer_rca(rca_t*rca);
#endif