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
