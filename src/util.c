#include <stdlib.h>
#include <stdio.h>


int readInt(int size){
    char buffer[size];
    int n = 0;
    while(n<size){
        int c = getc();
        if(c == EOF){
            return -1
        }
        buffer[n] = c;
        n++;
    }
    if(n==size){
        return -2;
    }
    return atoi(buffer)
}

int main(){
    int n = readInt(5);
    printf("valeur de n: %d"\n, n);
    return 0;
}