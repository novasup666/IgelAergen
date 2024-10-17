#include "resizableCharArray.h"
#include <stdio.h>

int main(void){
    rca_t*r = creer_rca(1);
    printf("rca créé\n");
    push_rca(r,'A');
    push_rca(r,'A');
    push_rca(r,'A');
    printf("a pushé\n");
    printf("%c\n", peek_rca(r,0));
    printf("%c\n", peek_rca(r,2));
    printf("%c\n", pop_rca(r));

}