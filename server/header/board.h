#ifndef BOARD_H
#define BOARD_H

#define TEXT_SIZE 255
#include "util.h"
/**
 * @struct Board
 * @brief Represents a game board with a grid of cells.
 * 
 * @var Board::h
 * Height of the board.
 * @var Board::w
 * Width of the board.
 * @var Board::grid
 * Pointer to the grid data, stored as a 1D array.
 */
typedef struct Board {
    uint8_t h;
    uint8_t w;
    uint8_t* grid;
} Board;

/**
 * @struct Line
 * @brief Represents a line of text with a cursor position.
 * 
 * @var Line::data
 * Character array to store the text.
 * @var Line::cursor
 * Position of the cursor within the text.
 */
typedef struct Line {
    char data[TEXT_SIZE];
    int cursor;
} Line;

/**
 * @struct Pos
 * @brief Represents a position on the board with x and y coordinates.
 * 
 * @var Pos::x
 * X coordinate of the position.
 * @var Pos::y
 * Y coordinate of the position.
 */
typedef struct Pos{
    uint8_t x;
    uint8_t y;

} Pos;



#endif