#pragma once
#include <cstdint>
#include <vector>

#include "move.h"

enum class TTFlag : uint8_t {
    Exact,
    LowerBound,
    UpperBound,
};

struct TTEntry {
    uint64_t hash;
    int8_t depth;
    int16_t score;
    Move best_move;
    TTFlag flag;
    uint8_t age = 0;
};

static_assert(sizeof(TTEntry) == 16);

void resize_transposition_table(int size_mb);

bool tt_probe(uint64_t hash, TTEntry& out);
void tt_store(uint64_t hash, int depth, int score, Move best_move, TTFlag flag);

void tt_prefetch(uint64_t hash);

int encode_mate_score(int score, int ply);
int decode_mate_score(int score, int ply);

void clear_transposition_table();

void tt_new_search();
