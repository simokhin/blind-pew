#pragma once

#include <vector>
#include "move.h"
#include "board.h"

struct Offset
{
    int dr;
    int df;
};

std::vector<Move> generate_leaper_moves(const Board &board, int square, const std::vector<Offset> &offsets);
std::vector<Move> generate_knight_moves(const Board &board, int square);
std::vector<Move> generate_king_moves(const Board &board, int square);
std::vector<Move> generate_slider_moves(const Board &board, int square, const std::vector<Offset> &directions);
std::vector<Move> generate_rook_moves(const Board &board, int square);
std::vector<Move> generate_bishop_moves(const Board &board, int square);
std::vector<Move> generate_queen_moves(const Board &board, int square);