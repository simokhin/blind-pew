#pragma once

#include <array>

enum class Piece
{
    None,
    WP,
    WN,
    WB,
    WR,
    WQ,
    WK,
    BP,
    BN,
    BB,
    BR,
    BQ,
    BK
};

enum class Color
{
    White,
    Black,
    None
};

Color color_of(Piece piece);

using Board = std::array<Piece, 64>;

void print_board(const Board &board);

int rank_of(int square);
int file_of(int square);
int square_of(int rank, int file);
bool is_valid_square(int rank, int file);
