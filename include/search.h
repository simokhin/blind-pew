#pragma once
#include "move.h"
#include "position.h"

struct SearchState {
    long nodes = 0;
};

int negamax(Position& position, int depth, SearchState& state, int ply = 0);
Move find_best_move(Position& position, int depth, SearchState& state);