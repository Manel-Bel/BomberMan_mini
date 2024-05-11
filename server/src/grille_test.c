#include <stdio.h>
#include <stdlib.h>
#include "../header/grille_test.h"

#define H 20
#define W 20

void init_grille(uint8_t *grille) {
    // Initialize all positions as empty space
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            grille[i*W+j] = (uint8_t)0;
        }
    }

    // Set up the outer walls
    for (int i = 0; i < W; i++) {
        grille[0*W+i] = (uint8_t)1; // Top boundary
        grille[(H-1)*W+i] = (uint8_t)1; // Bottom boundary
    }
    for (int i = 0; i < H; i++) {
        grille[i*W+0] = (uint8_t)1; // Left boundary
        grille[i*W+(W-1)] = (uint8_t)1; // Right boundary
    }

    // Randomly generate some obstacles
    srand(time(NULL)); // Initialize the random number generator
    for (int i = 3; i < H-3; i++) {
        for (int j = 3; j < W-3; j++) {
            if (rand() % 100 < 10) { // 10% probability of generating an obstacle
                grille[i*W+j] = (uint8_t)2; // Set as destructible wall
            }
        }
    }

    // Ensure that the four corners and their surroundings are empty space
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            grille[i*W+j] = (uint8_t)0; // Top left corner
            grille[i*W+(W-1-j)] = (uint8_t)0; // Top right corner
            grille[(H-1-i)*W+j] = (uint8_t)0; // Bottom left corner
            grille[(H-1-i)*W+(W-1-j)] = (uint8_t)0; // Bottom right corner
        }
    }
}


// void print_grille(char **grille) {
//     for (int i = 0; i < H; i++) {
//         for (int j = 0; j < W; j++) {
//             switch (grille[i][j]) {
//                 case 0:
//                     printf("0 "); // Espace ouvert
//                     break;
//                 case 1:
//                     printf("1 "); // Mur indestructible
//                     break;
//                 case 2:
//                     printf("2 "); // Mur destructible
//                     break;
//             }
//         }
//         printf("\n");
//     }
// }

void print_grille_1D(uint8_t *grille){

    printf("imprimer grille \n");
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            //printf("numero case à imprimer %d\n",H*i+j);
            printf("%u ",grille[W*i+j]);
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