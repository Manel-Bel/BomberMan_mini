# Projet_reseau

## description 
Développement d'un serveur et d'un client pour un jeu de Bomberman en réseau avec quatre joueurs sur une grille, intégrant des mécanismes de placement de bombes et de destruction de murs.

## Fonctionnalités :
Gestion de plusieurs parties simultanées.
Modes de jeu : 4 adversaires et équipes de 2 joueurs.
Échanges en temps réel via TCP et UDP.
Multidiffusion de la grille de jeu et chat intégré.
Fin de partie avec annonce du gagnant.

## Technologies utilisées : Programmation réseau, protocoles TCP/UDP, gestion de connexions multiples, mise à jour en temps réel.

1.  include the -lncurses option after the prograam name
2.  gcc ./src/client.c -lncurse


# To START A GAME

I. Run server

1.  Go to folder Projet_reseau/server
2.  Execute make
3.  Run ./server \<freq> (where freq must be a number which indicates the interval to send the differential in ms)

II. Run client

1.  Go to folder Projet_reseau/client
2.  Execute make
3.  Run ./client \<server address name> \<optionall : file_name_to_save_logs> (the file is automatically created during execution)


# How to Play 

**To move a player**, use the touch up, down, right, and left.

**To plant a bomb**, use the symbol $ . 

**To quit** the game when you are in the graphical interface, use ~ .

**To send chat messages to team members**, use the symbol @ before entering the message.
