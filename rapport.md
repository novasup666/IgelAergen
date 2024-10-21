# Rapport - Igel Aergen

Adil Oubninte: temps passé env. x heures

Noé Vincent: temps passé env. 30 heures

## I. Introduction

Igel Aergen est un jeux de plateau multijoueur, l'objectif içi à été de produire un programme C permettant de jouer à ce jeu dans un terminal d'ordinateur en multijoueur local ou réseau. Ce projet permet d'explorer différentes façettes et différents niveaux de la programmation C, allant du réseau à la gestion de structures abstraites, de plus il nous aura permis d'utiliser à nouveau des outils permettant le bon développement d'un projet: la compilation automatisée avec CMake et la collaboration avec Git.

## II. Première (petite) extension - Resizable Char Array
Dans un souci d'économie de la mémoire de résilience fâce à des cas extrêmes (type 25 joueurs ayant 300 hérissons chacuns) nous avons décidé d'implémenter les petites piles qui constituent les cases à l'aide de tableaux dynamique. Un module a donc été créé ```ResizableCharArray``` qui permet le type ```rca_t``` utilisé donc pour stocker les hérissons situés sur une case. L'implémentation est classique, les tableaux sont initialisés de sorte à n'utiliser que peut de mémoire, l'espace alloué est doublé de taille à chaque dépassement de la capacité.

## III. Principale extension - Mode multijoueur en réseau

Afin de permettre un mode multijoueur plus confortable, pour les joueurs, nous avont décidé d'implémenter une solution permettant de jouer à plusieurs depuis différentes machines sur le même réseau local (ou sur des réseaux différents mais dont des ports spécifiques ont été ouverts).

Pour ce faire, une architecture client serveur à été choisie. Pour être plus précis, le serveur est aussi joueur, il executera donc en parallèle : 
- un processus "serveur" permettant d'orchestrer les différentes phases du jeux et la synchronisation des plateaux entre les joueurs.
- un processus "client" connecté au serveur hébergé sur la même machine, permettant au propriétaire de la machine de jouer aussi.


