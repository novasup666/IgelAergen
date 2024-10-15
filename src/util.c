#include <stdlib.h>
#include <stdio.h>


int readInt(int size){
    char buffer[size];
    int n = 0;
    while(n<size){
        int c = getc(stdin);
        printf("char: %c\n", c);
        if(c == EOF){
            return -1; //not suppose to happen
        }
        if(c=='\n'){
            break;
        }
        buffer[n] = 0;
        n++;
    }
    if(n==size){
        return -2; //depassement de capacité
    }
    return atoi(buffer);
}

int main(){
    printf("Une valeur stp:\n>");
    int n = readInt(5);
    printf("valeur de n: %d\n", n);
    return 0;
}