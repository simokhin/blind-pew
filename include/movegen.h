#pragma once

#include <vector>
#include "move.h"
#include "board.h"

struct Offset
{
    int dr;
    int df;
};

std::vector<Move> generate_knight_moves(const Board &board, int square);