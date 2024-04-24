#include <stdio.h>
#include <stdlib.h>

#define H 20
#define W 20

void init_grille(char *grille) {
    // Initialize all positions as empty space
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            grille[i*W+j] = 0;
        }
    }

    // Set up the outer walls
    for (int i = 0; i < W; i++) {
        grille[0*W+i] = 1; // Top boundary
        grille[(H-1)*W+i] = 1; // Bottom boundary
    }
    for (int i = 0; i < H; i++) {
        grille[i*W+0] = 1; // Left boundary
        grille[i*W+(W-1)] = 1; // Right boundary
    }

    // Randomly generate some obstacles
    srand(time(NULL)); // Initialize the random number generator
    for (int i = 3; i < H-3; i++) {
        for (int j = 3; j < W-3; j++) {
            if (rand() % 100 < 10) { // 10% probability of generating an obstacle
                grille[i*W+j] = 2; // Set as destructible wall
            }
        }
    }

    // Ensure that the four corners and their surroundings are empty space
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            grille[i*W+j] = 0; // Top left corner
            grille[i*W+(W-1-j)] = 0; // Top right corner
            grille[(H-1-i)*W+j] = 0; // Bottom left corner
            grille[(H-1-i)*W+(W-1-j)] = 0; // Bottom right corner
        }
    }
}


void print_grille(char **grille) {
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

void print_grille_1D(char *grille){

    printf("imprimer grille \n");
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            //printf("numero case à imprimer %d\n",H*i+j);
            int value = grille[H*i+j];
            if (value >= 5) {
                // c = value - 5 + '0'; // Display player ID
                printf("%d ",value); // Display player ID
            }else{
                switch (value) {
                    case 0:
                        printf(". "); // Espace ouvert
                        break;
                    case 1:
                        printf("# "); // Mur indestructible
                        break;
                    case 2:
                        printf("* "); // Mur destructible
                        break;
                    case 3:
                        // c = 'B'; // Bomb
                        printf("B "); // Bomb
                        break;
                    case 4:
                        // c = 'E'; // Exploded by bomb
                        printf("E "); // Exploded by bomb
                        break;
                    default:
                        // c = '?'; // Unknown character
                        printf("? "); // Unknown character
                        break;
            }}
        }
        printf("\n");
    }

}

int main5555() {
    char grille[H*W];
    init_grille(grille);
    print_grille_1D(grille);
    return 0;
}
//gcc -o grille_test grille_test.c && ./grille_test