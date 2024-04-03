Bomberman/
│
├── server/                  # serveur côté code répertoire
│   ├── main.c               # point d'entrée du programme principal côté serveur
│   ├── server.h             # fichier d'en-tête côté serveur
│   ├── server.c             # implémentation côté serveur
│   ├── game_logic.c         # traitement de la logique du jeu
│   ├── network.c            # traitement de la communication réseau
│   ├── session_manager.c    # gestion de session  
│   ├── client_handler.c     # gérer un client unique
│   └── Makefile             # règles de compilation côté serveur
│
├── client/                  # répertoire de code client
│   ├── main.c               # point d'entrée du programme principal client
│   ├── client.h             # fichier d'en-tête client
│   ├── client.c             # implémentation client
│   ├── ui.c                 # code d'interface utilisateur (utilisation de ncurses)
│   ├── network.c            # traitement de la communication réseau
│   └── Makefile             # règles de compilation client
│
├── common/                  # code partagé par le client et le serveur
│   ├── common.h             # fichiers d'en-tête partagés, comme définitions de protocole
│   ├── utilities.c          # fonctions utilitaires partagées, telles que l'empaquetage et le dépaquetage des messages
│   └── constants.h          # définit les constantes du jeu et de la communication
│
├── test/                    # répertoire de code de test
│   ├── server_tests.c       # code de test de fonctionnalité serveur
│   └── client_tests.c       # code de test de fonctionnalité client
│
├── docs/                    # répertoire de documentation de projet
│   ├── README.md            # fichier d'explication du projet
│   ├── protocol.md          # documentation du protocole réseau
│   └── extension.md         # décrit toute fonctionnalité ou extension supplémentaire, le cas échéant
│
└── Makefile                 # Makefile racine, utilisé pour construire le client et le serveur
