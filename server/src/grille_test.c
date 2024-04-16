#include <stdio.h>
#include <stdlib.h>

#define H 20
#define W 20

void init_grille(int **grille) {
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            // Initialiser les murs indestructibles
            if (i == 0 || i == H-1 || j == 0 || j == W-1 || (i % 2 == 0 && j % 2 == 0)) {
                grille[i][j] = 1; // Mur indestructible
            }
            // Alternance entre murs destructibles et espaces ouverts pour créer des chemins
            else {
                if ((i + j) % 4 == 0) { // Cette condition modifie la distribution des murs destructibles
                    grille[i][j] = 2; // Mur destructible
                } else {
                    grille[i][j] = 0; // Espace ouvert
                }
            }
        }
    }
}

void print_grille(int **grille) {
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            switch (grille[i][j]) {
                case 0:
                    printf(". "); // Espace ouvert
                    break;
                case 1:
                    printf("# "); // Mur indestructible
                    break;
                case 2:
                    printf("* "); // Mur destructible
                    break;
            }
        }
        printf("\n");
    }
}

int main5555() {
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
}
//gcc -o grille_test grille_test.c && ./grille_test