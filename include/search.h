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
    std::array<std::array<Move, MAX_PLY>, MAX_PLY> pv_table = {};
    std::array<int, MAX_PLY> pv_length = {};
    std::array<std::array<Move, 2>, MAX_PLY> killers = {};
    std::array<std::array<std::array<int, 64>, 64>, 2> history_heuristic = {};
};

bool make_legal_move(Position& position, const Move& m, UndoInfo& undo);

int negamax(Position& position, int depth, SearchState& state, int alpha = -INFINITE,
            int beta = INFINITE, int ply = 0, bool allow_null = true);

Move find_best_move(Position& position, int max_depth, SearchState& state);

int quiescence(Position& position, int alpha, int beta, SearchState& state, int ply);

int mvv_lva_score(const Position& position, const Move& m);

void print_search_info(int depth, const SearchState& state, int best_score, double elapsed_seconds);

bool has_non_pawn_material(const Position& position, Color color);

void sort_moves(MoveList& moves, const Position& position, const SearchState& state, int ply,
                bool have_tt_move, const Move& tt_move);

void update_history_heuristic(SearchState& state, Color side, int from, int to, int bonus);

int see(const Position& position, int square, PieceType target_type, PieceType attacker_type,
        Color side, Bitboard occupancy, Bitboard from_set);

int see_capture(const Position& position, const Move& move);
