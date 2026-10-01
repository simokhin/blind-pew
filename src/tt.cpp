#include "tt.h"

#include <algorithm>

#include "constants.h"

static std::vector<TTEntry> transposition_table;

static uint8_t current_age = 0;

void tt_new_search() { current_age++; }

void resize_transposition_table(int size_mb) {
    size_t max_entries = (static_cast<size_t>(size_mb) * 1024 * 1024) / sizeof(TTEntry);

    size_t entries = 1;
    while (entries * 2 <= max_entries) {
        entries *= 2;
    }

    transposition_table.resize(entries);
}

bool tt_probe(uint64_t hash, TTEntry& out) {
    size_t index = hash & (transposition_table.size() - 1);
    TTEntry& entry = transposition_table[index];
    if ((entry.hash) == hash) {
        out = entry;
        return true;
    } else {
        return false;
    }
}

void tt_store(uint64_t hash, int depth, int score, Move best_move, TTFlag flag) {
    size_t index = hash & (transposition_table.size() - 1);
    TTEntry& entry = transposition_table[index];

    if (depth >= entry.depth || entry.age != current_age) {
        entry.best_move = best_move;
        entry.depth = depth;
        entry.flag = flag;
        entry.hash = hash;
        entry.score = score;
        entry.age = current_age;
    }
}

void tt_prefetch(uint64_t hash) {
    size_t index = hash & (transposition_table.size() - 1);

    __builtin_prefetch(&transposition_table[index]);
}

int encode_mate_score(int score, int ply) {
    if (score >= MATE_THRESHOLD) {
        return score + ply;
    } else if (score <= -MATE_THRESHOLD) {
        return score - ply;
    } else {
        return score;
    }
}

int decode_mate_score(int score, int ply) {
    if (score >= MATE_THRESHOLD) {
        return score - ply;
    } else if (score <= -MATE_THRESHOLD) {
        return score + ply;
    } else {
        return score;
    }
}

void clear_transposition_table() {
    std::fill(transposition_table.begin(), transposition_table.end(), TTEntry{});
}
