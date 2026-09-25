#include "tt.h"

#include "constants.h"

std::vector<TTEntry> transposition_table;

void resize_transposition_table(int size_mb) {
    size_t max_entries = (static_cast<size_t>(size_mb) * 1024 * 1024) / sizeof(TTEntry);

    size_t entries = 1;
    while (entries * 2 <= max_entries) {
        entries *= 2;
    }

    transposition_table.resize(entries);
}

TTEntry* tt_probe(uint64_t hash) {
    int index = hash & (transposition_table.size() - 1);
    TTEntry& entry = transposition_table[index];
    if ((entry.hash) == hash) {
        return &entry;
    } else {
        return nullptr;
    }
}

void tt_store(uint64_t hash, int depth, int score, Move best_move, TTFlag flag) {
    int index = hash & (transposition_table.size() - 1);
    TTEntry& entry = transposition_table[index];

    if (depth >= entry.depth) {
        entry.best_move = best_move;
        entry.depth = depth;
        entry.flag = flag;
        entry.hash = hash;
        entry.score = score;
    }
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
