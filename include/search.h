#pragma once
#include <chrono>

#include "constants.h"
#include "move.h"
#include "position.h"

struct SearchState {
    long nodes = 0;
    std::chrono::steady_clock::time_point deadline;
    bool stopped = false;
    int depth_reached = 0;
};

bool make_legal_move(Position& position, const Move& m, UndoInfo& undo);

int negamax(Position& position, int depth, SearchState& state, int alpha = -INFINITE,
            int beta = INFINITE, int ply = 0);
Move find_best_move(Position& position, int max_depth, SearchState& state);
int quiescence(Position& position, int alpha, int beta, SearchState& state, int ply);
