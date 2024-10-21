#ifndef MULTI_H
#define MULTI_H

#include "gametype.h"
#include "ask_user.h"
#include "main.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <string.h>


//source: syllabus du cours de réseau

/*struct addrinfo{
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    size_t ai_addrlen;
    struct sockaddr *ai_addr;
    char *ai_canonname;
    struct addrinfo *ai_next;
};*/

//typedef struct addrinfo addrinfo_t;




struct server_partie_info {
    int nb_joueur;
    int nb_herisson_par_joueur;
    char* port;
};
typedef struct server_partie_info server_partie_info_t;

struct client_partie_info {
    int nb_joueur;
    int nb_herisson_par_joueur;
    int joueur;
    char* port;
    char* hostname;
    info_partie_t* info;
};
typedef struct client_partie_info client_partie_info_t;



void * serveur(void* arg);
void * client(void* arg);


void* serveur_test(void* arg);

int int_to_ascii(int n, char* buffer);

#endif