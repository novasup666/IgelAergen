#ifndef RCA_H
#define RCA_H


struct rca_s{
    int size;
    int capacity;
    char * tab;
};

typedef struct rca_s rca_t;

void push_rca(rca_t *rca,char valeur);
void pop_rca(rca_t*rca);
#endif