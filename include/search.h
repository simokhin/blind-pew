#pragma once
#include <chrono>
#include <vector>

#include "constants.h"
#include "move.h"
#include "movegen.h"
#include "position.h"

struct SearchState {
    long nodes = 0;
    std::chrono::steady_clock::time_point deadline;
    bool stopped = false;
    int depth_reached = 0;
    std::vector<uint64_t> history;
    std::array<std::array<Move, MAX_PLY>, MAX_PLY> pv_table;
    std::array<int, MAX_PLY> pv_length;
};

bool make_legal_move(Position& position, const Move& m, UndoInfo& undo);

int negamax(Position& position, int depth, SearchState& state, int alpha = -INFINITE,
            int beta = INFINITE, int ply = 0);

Move find_best_move(Position& position, int max_depth, SearchState& state);

int quiescence(Position& position, int alpha, int beta, SearchState& state, int ply);

int mvv_lva_score(const Position& position, const Move& m);

void print_search_info(int depth, const SearchState& state, int best_score, double elapsed_seconds);
