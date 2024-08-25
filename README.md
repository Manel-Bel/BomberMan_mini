# Network_Project

## Description 
Development of a server and client for a networked Bomberman game with four players on a grid, featuring bomb placement and wall destruction mechanisms.

## Features :
Management of multiple simultaneous games.
Game modes: 4-player free-for-all and 2-player teams.
Real-time communication via TCP and UDP.
Multicasting of the game grid and integrated chat functionality.
Game conclusion with the announcement of the winner.

## Technologies Used : Network programming, TCP/UDP protocols, multiple connection management, real-time updates.

## Compilation :
1.  Include the -lncurses option after the program name:
2.  gcc ./src/client.c -lncurse


# To START A GAME

I. Run server

1.  Navigate to the Projet_reseau/server folder.
2.  Execute make.
3.  Run ./server <freq> (where freq is a number indicating the interval to send the differential in milliseconds).

II. Run the Client

1.  Navigate to the Projet_reseau/client folder.
2.  Execute make.
3.  RRun ./client <server address> <optional: file_name_to_save_logs> (the file is automatically created during execution).

# How to Play 

**To move a player**, use the touch up, down, right, and left.

**To plant a bomb**, use the symbol $ . 

**To quit** the game when you are in the graphical interface, use ~ .

**To send chat messages to team members**, use the symbol @ before entering the message.
