#ifndef MULTI_H
#define MULTI_H

#include "gametype.h"
#include "ask_user.h"
#include "main.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdarg.h>


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

struct commande{
    bool is_cmd; //utile pour handle_cmd càd savoir si le client doit envoyer une réponse. Si c'est false, on assume que tout est NULL ou -1
    int id;
    int nb_args;
    bool auto_instancie;
    char* cmd;
    char** args;
};
typedef struct commande commande_t;

void * serveur(void* arg);
void * client(void* arg);

void* serv2(void* arg);
void* client2(void* arg);


void* serveur_test(void* arg);

int int_to_ascii(int n, char* buffer);

int lookup(char* str, char delim);

commande_t* str_to_cmd(char* buffer);
char* cmd_to_str(commande_t *cmd);
void free_cmd(commande_t* cmd);

#endif