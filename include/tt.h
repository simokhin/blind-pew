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
};

extern std::vector<TTEntry> transposition_table;

void resize_transposition_table(int size_mb);

TTEntry* tt_probe(uint64_t hash);
void tt_store(uint64_t hash, int depth, int score, Move best_move, TTFlag flag);

int encode_mate_score(int score, int ply);
int decode_mate_score(int score, int ply);