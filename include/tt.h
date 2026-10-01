#pragma once
#include <cstdint>
#include <vector>

#include "move.h"

enum class TTFlag {
    Exact,
    LowerBound,
    UpperBound,
};

struct TTEntry {
    uint64_t hash;
    int depth;
    int score;
    Move best_move;
    TTFlag flag;
    uint8_t age = 0;
};

void resize_transposition_table(int size_mb);

TTEntry* tt_probe(uint64_t hash);
void tt_store(uint64_t hash, int depth, int score, Move best_move, TTFlag flag);

void tt_prefetch(uint64_t hash);

int encode_mate_score(int score, int ply);
int decode_mate_score(int score, int ply);

void clear_transposition_table();

void tt_new_search();
