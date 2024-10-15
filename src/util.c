#include <stdlib.h>
#include <stdio.h>



int atoiv2(char* str){
    int resultat = 0;
    while(*str != '\0'){
        if(*str < '0' || *str > '9'){
            return -1;
        }
        resultat *=10;
        resultat+= *str-'0';
        str++;
    }
    return resultat;
}

//renvoie la valeur si c'est correcte, -1 si NaN -2 si il y a un depassement de capacité et -3  si EOF
int readInt(int size){
    char* buffer = malloc(sizeof(char)*size);
    int n = 0;
    while(n<size){
        int c = getc(stdin);
        if(c == EOF){
            return -3; //wtf ?
        }
        if(c=='\n'){
            break;
        }
        buffer[n] = c;
        n++;
    }
    if(n==size){
        return -2; //depassement de capacité
    }

    return atoiv2(buffer); //-1 si NaN 
}
