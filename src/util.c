#include <stdlib.h>
#include <stdio.h>


//renvoie la valeur si c'est correcte, -1 si NaN -2 si il y a un depassement de capacité et -3  si EOF
int readInt(int size){
    char buffer[size];
    int n = 0;
    while(n<size){
        int c = getc(stdin);
        if(c == EOF){
            return -1; //not suppose to happen
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

    int value = atoi(buffer);
    if(value == 0){
        //atoi returns 0 if NaN
        return -1;
    }
    return value;
}
