#include <stdio.h>
#include <stdlib.h>

#define H 20
#define W 20

void init_grille(char *grille) {
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            // Initialiser les murs indestructibles
            if (i == 0 || i == H-1 || j == 0 || j == W-1 || (i % 2 == 0 && j % 2 == 0)) {
                grille[i*H+j] = 1; // Mur indestructible
            }
            // Alternance entre murs destructibles et espaces ouverts pour créer des chemins
            else {
                if ((i + j) % 4 == 0) { // Cette condition modifie la distribution des murs destructibles
                    grille[i*H+j] = 2; // Mur destructible
                } else {
                    grille[i*H+j] = 0; // Espace ouvert
                }
            }
        }
    }
    grille[0]=5;
}

void print_grille(char **grille) {
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            switch (grille[i][j]) {
                case 0:
                    printf("0 "); // Espace ouvert
                    break;
                case 1:
                    printf("1 "); // Mur indestructible
                    break;
                case 2:
                    printf("2 "); // Mur destructible
                    break;
            }
        }
        printf("\n");
    }
}

void print_grille_1D(char *grille){

    printf("imprimer grille \n");
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            //printf("numero case à imprimer %d\n",H*i+j);
            printf("%d ",grille[W*i+j]);
        }
        printf("\n");
    }

}

/*int main5555() {
    int **grille = malloc(H * sizeof(int*));
    for (int i = 0; i < H; i++) {
        grille[i] = malloc(W * sizeof(int));
    }

    init_grille(grille);
    print_grille(grille);

    // Libérer la mémoire
    for (int i = 0; i < H; i++) {
        free(grille[i]);
    }
    free(grille);

    return 0;
}*/
//gcc -o grille_test grille_test.c && ./grille_test