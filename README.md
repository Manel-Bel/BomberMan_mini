# Projet_reseau

## Getting started

1.  include the -lncurses option after the prograam name
2.  gcc ./src/client.c -lncurse

### **Structure**

project_folder/  
│  
├── client/  
│ ├── header/  
│ │ └── (client headers)  
│ ├── src/  
│ │ └── (client sources)  
│ └── Makefile  
│  
├── server/  
│ ├── header/  
│ │ └── (server headers)  
│ ├── src/  
│ │ └── (server sources)  
│ └── Makefile  
│

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
