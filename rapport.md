# Igel Aergen - Adil Oubninte, Noé Vincent
Temps dévoué au projet :
- Adil Oubninte: env. 30 heures

- Noé Vincent: env. 20 heures

## I. Introduction

Igel Aergen est un jeux de plateau se jouant à plusieurs (2 à 6 joueurs normalement), l'objectif içi à été de produire un programme C permettant de jouer à ce jeu au travers d'un seul et même terminal d'ordinateur ou, dans notre cas, en réseau. 

Ce projet permet d'explorer différentes façettes et différents niveaux de la programmation C, allant du réseau à la gestion de structures abstraites. De plus, il nous aura permis d'utiliser et de nous familiariser encore un peu plus avec des outils permettant le bon développement d'un projet: la compilation automatisée avec CMake et la collaboration avec Git.


## II. (Pas vraiment une) extension - Resizable Char Array
### _*Objectif*_
Dans un souci d'économie de la mémoire, nous avons décidé d'implémenter les piles qui constituent les cases à l'aide de tableaux dynamiques. 
Ceci permet d'éviter d'allouer, pour chaque case, l'espace nécessaire à stocker les hérissons dans le pire cas possible, c'est-à-dire tout les hérissons sur la même case.

Cette extension n'en est pas vraiment une car elle ne peut être désactivée, c'est plutôt un choix d'implémentation que nous avons considéré comme suffisament influent pour mériter sa place dans le rapport.
### _*Réalisation*_
Un module ```ResizableCharArray``` a  été créé, il permet le type ```rca_t```. Celui-ci est utilisé pour stocker les hérissons situés sur une case. L'implémentation est classique, les tableaux sont initialisés de sorte à n'utiliser que peut de mémoire, l'espace alloué est doublé de taille à chaque dépassement de la capacité. 

## III. (Une toute petite) extension - Choix des positions de départ

> It's not a bug, it's a feature !

### *_Objectif_*

Cette extension vient initialement d'une erreur dans la compréhension du sujet. Nous avons finalement décidé de conserver cette fonctionnalité, le joueur a donc le choix de placer ses pions à la main ou de manière aléatoire (en fonctionde la valeur de `PLACEMENT_RANDOM` dans `main.h`).

### *_Réalisation_*

Dans le cas où cette extension est activée, on demande à chaque joueur de sélectionner les lignes où ils souhaite placer ses hérissons.

## IV. Principale extension - Mode multijoueur en réseau

Afin d'activer, ou non, le mode multijoueur en réseau, deux options sont disponible:
- lancer le programme et suivre les instructions affichées 
- passer comme argument du programme l'entier correspondant (1: mode classique, 2: mode serveur, 3: mode client).

### *_Objectif_*
Afin de permettre un mode multi-joueur plus confortable, pour les joueurs, nous avont décidé d'implémenter une solution permettant de jouer à plusieurs depuis différentes machines  (une par joueur) sur le même réseau local (ou sur des réseaux différents mais dont des ports spécifiques ont été ouverts). 


### *_Réalisation_*
Pour ce faire, une architecture client-serveur à été choisie. L'essentiel du code soutenant cette architecture se situe dans le module ```multi```, créé par nos soins.
Notons que chaque machine stocke sa propre instance du plateau, celle-ci évolue avec les coups jouée par chaque joueur.
Le serveur cherche donc à synchroniser les différents plateaux des différents clients.


Le client execute la fonction ```client()```, celle-ci prends comme argument une structure portant diverses informations de connection et paramêtres de la partie.

Le serveur, lui, à un rôle un peu plus complexe. En effet, le serveur est aussi joueur, il executera donc en parallèle : 
- la fonction ```serveur()``` permettant d'orchestrer les différentes phases du jeux et la synchronisation des plateaux entre les joueurs.
- la fonction ```client()``` connectée au serveur hébergé sur la même machine, permettant au propriétaire de la machine de jouer aussi.

La communication entre clients et serveur se fait au travers de sockets réseau, à l'aide du module ```csapp.h```.
Ainsi cette extension fait intervenir 2 composantes imprévues pour le projet: le gestion de sockets et le multithreading.

### _*Protocole de communication*_

1. Phase d'initialisation:
    - Le serveur envoie à chaque client connecté la requête `"who"`
    - Ceux-ci lui répondent en indiquant leurs numéro de joueur (avec une commande `"im"`)
    - Une fois que le nombre attendu de joueurs s'est connecté, le serveur envoie `"all_player_ok"` à chacun des joueurs 
    - S'ensuit alors un échange avec chaque client afin d'obtentir le placement de leurs hérissons. (Requête `"place"`, réponse et propagation `"placed"`)
    - À la fin de cette phase, le serveur envoie `"start"` à chaque joueur.


2. Phase de jeu
    - à chaque tour, le serveur envoie `"play"` au joueur concerné.
    - celui-ci repond avec un message décrivant son coup avec une commande `"move"`.
    - le serveur propage ce coup aux autres joueurs avec une commande `"move"`.
    - un fois qu'un client a gagné, il propage une commande `"exit"`.

Un commande correspond au type suivant:

```C 
struct commande{
    bool is_cmd; 
    int id;
    int nb_args;
    bool auto_instancie;
    char* cmd;
    char** args;
};
```

Petite explication des champs:

- `is_cmd` indique si une réponse du client est attendue
- `id`  indique le joueur concerné par la commande (si c'est un message du serveur comme `"all_player_ok"` : -1)
- `nb_args` indique la taille de `args`.
- `auto_instancie` indique si les strings fournit ensuite sont sur la pile ou le tas, permettant de savoir si elles doivent être libérées ou non.
- `cmd` indique le type de la commande parmis (`"who"`,`"im"`,`"all_player_ok"`,`"place"`,`"placed"`,`"start"`, `"play"`, `"move"`, `"exit"`)
- `args` contient des informations complémentaires à la commande (exemple, les lignes dans lesquelles sont initialement placés les hérissons d'un joueur, dans le cadre d'une commande `"placed"`)


_Exemple de message_ : 

Placement des hérissons par le joueur 0, en réponse à une commande `"place"` envoyée par le serveur. 
Le joueur 0 place des hérissons aux lignes 0, 4 et 5.

```C
    commande_t c = {true; 0; 3; false; "placed"; {0,4,5}};
```

## V. Conclusion

Remarquons tout d'abord le bilan mitigé de l'extension permettant le choix des positions de départ des hérissons. En effet, celle-ci ajoute une lourdeur considérable au jeu sans apporter un grand avantage ludique.




## VI. Bibliographie


- Module ```csapp.h```, Carnegie Mellon University, [Lien vers csapp.c](https://csapp.cs.cmu.edu/3e/ics3/code/src/csapp.c), [Lien vers csapp.h](https://csapp.cs.cmu.edu/3e/ics3/code/src/csapp.h)

    Ce module a été utilisé selon les conseils de Guillaume Didier, enseignant le Réseau au sein du Module SYS en 1ère année du département d'informatique de l'ENS de Rennes. Ce module a permit de simplifier grandement la gestion des sockets réseau et de ce fait, a permit la communication entre clients et serveur.

- Cours de Guillaume Didier, SYS-L3 INFO ENS Rennes

    Ce cours a été utile dans la compréhension et la mise en pratique du module ```csapp``` cité plus haut.

- Variadic Function in C Programming, Loo Kian Selong, The Startup, [Lien vers l'article](https://medium.com/swlh/variadic-function-in-c-programming-d3632315a48e)
    
    Cet article a permit de maitriser les fonctions de `stdarg.h` utilisée notamment dans les fonctions gérants les commandes et requêtes échangées entre le serveur et les clients.


- StackOverFlow

    Forum utilisé dans des cas marginaux pour traiter des bizarreries de C, en particulier pour le traitement des strings et buffers.