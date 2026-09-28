#pragma once
#include <atomic>
#include <chrono>
#include <vector>

#include "constants.h"
#include "move.h"
#include "movegen.h"
#include "position.h"

struct SearchState {
    long nodes = 0;

    // Time control
    std::chrono::steady_clock::time_point hard_deadline;
    std::chrono::steady_clock::time_point soft_deadline;
    std::atomic<bool> stopped = false;

    int depth_reached = 0;

    // Game history
    std::vector<uint64_t> history;

    // Principal variation table
    std::array<std::array<Move, MAX_PLY>, MAX_PLY> pv_table = {};
    std::array<int, MAX_PLY> pv_length = {};

    // Killer moves and history heuristic
    std::array<std::array<Move, 2>, MAX_PLY> killers = {};
    std::array<std::array<std::array<int, 64>, 64>, 2> history_heuristic = {};

    bool use_tt = true;  // Использовать ли таблицу транспозиций (нужно для тюнера)
};

// Инициализирует таблицы, нужные для LMR
void init_lmr_table();

Move find_best_move(Position& position, int max_depth, SearchState& state);

int quiescence(Position& position, int alpha, int beta, SearchState& state, int ply);
