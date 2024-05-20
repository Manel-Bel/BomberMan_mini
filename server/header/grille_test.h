#ifndef GRILLE_TEST_H
#define GRILLE_TEST_H

#include <time.h>
#include <sys/types.h>
#include <stdint.h>
#include "board.h"


void init_grille(uint8_t *grille);
/**
 * @brief Prints the grid in a 1D format.
 * 
 * @param grille Pointer to the grid to be printed.
 */
void print_grille_1D(uint8_t *grille);
void print_grille(Board * b);
#endif