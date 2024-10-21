#include <stdio.h>

int main(void){
    int entier;
    printf("entre un entier\n>");
    scanf("%d",&entier);
    char * string;
    asprintf(&string,"je mets l'entier entre les chevrons >%d<%d\n", entier,entier);
    printf("%s",string);

}