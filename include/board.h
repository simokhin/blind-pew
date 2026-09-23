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

using Board = std::array<Piece, 64>;

Board make_start_position();

void print_board(const Board &board);

int rank_of(int square);
int file_of(int square);
int square_of(int rank, int file);
bool is_valid_square(int rank, int file);

bool is_white(Piece piece);
bool is_black(Piece piece);