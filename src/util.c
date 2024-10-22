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


void flush_stdin(){
    int c;
    while((c = getc(stdin)) != '\n' && c != EOF);
}

//renvoie la valeur si c'est correcte, -1 si NaN -2 si il y a un depassement de capacité et -3  si EOF
int _readInt(int size){

    // Philosophiquement plus intéressante mais finalement pas utilisée

    fflush(stdout);
    char* buffer = calloc(size+1, sizeof(char));
    int n = 0;
    while(n<=size){
        int c = getc(stdin);
        if(c==EOF){ 
            exit(-1); //l'user a fait un ctrl+D, on quitte le programme
            //return -3;
        }
        if(c=='\n'){
            break;
        }
        buffer[n] = c;
        n++;
    }
    if(n>=size){
        flush_stdin();
        return -2; //depassement de capacité
    }
    buffer[size] = '\0';
    //flush_stdin();
    int v = atoiv2(buffer); //-1 si NaN 
    free(buffer);
    return v;
}

int readInt(int size){
    int i;
    scanf("%d", &i);
    return i;
}
