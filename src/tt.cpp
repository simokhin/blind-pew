#include "tt.h"

#include <algorithm>
#include <atomic>

#include "constants.h"

struct TTSlot {
    std::atomic<uint64_t> key_xor_data;
    std::atomic<uint64_t> data;
};

static std::vector<TTSlot> transposition_table;

static std::atomic<uint8_t> current_age = 0;

void tt_new_search() { current_age++; }

void resize_transposition_table(int size_mb) {
    size_t max_entries = (static_cast<size_t>(size_mb) * 1024 * 1024) / sizeof(TTSlot);

    size_t entries = 1;
    while (entries * 2 <= max_entries) {
        entries *= 2;
    }

    transposition_table = std::vector<TTSlot>(entries);
}

static uint64_t pack_tt_data(int score, Move best_move, int depth, TTFlag flag, uint8_t age) {
    return static_cast<uint64_t>(static_cast<uint16_t>(score)) |
           (static_cast<uint64_t>(static_cast<uint16_t>(best_move.raw())) << 16) |
           (static_cast<uint64_t>(static_cast<uint8_t>(depth)) << 32) |
           (static_cast<uint64_t>(static_cast<uint8_t>(flag)) << 40) |
           (static_cast<uint64_t>(age) << 48);
}

static TTEntry unpack_tt_data(uint64_t data) {
    TTEntry entry;
    entry.score = static_cast<int16_t>(data);
    entry.best_move = Move::from_raw(static_cast<uint16_t>(data >> 16));
    entry.depth = static_cast<int8_t>(data >> 32);
    entry.flag = static_cast<TTFlag>(static_cast<uint8_t>(data >> 40));
    entry.age = static_cast<uint8_t>(data >> 48);
    return entry;
}

bool tt_probe(uint64_t hash, TTEntry& out) {
    size_t index = hash & (transposition_table.size() - 1);

    TTSlot& slot = transposition_table[index];
    uint64_t data = slot.data.load();
    uint64_t key = slot.key_xor_data.load();

    if ((key ^ data) == hash) {
        out = unpack_tt_data(data);
        out.hash = hash;
        return true;
    }
    return false;
}

void tt_store(uint64_t hash, int depth, int score, Move best_move, TTFlag flag) {
    size_t index = hash & (transposition_table.size() - 1);
    TTSlot& slot = transposition_table[index];
    TTEntry old = unpack_tt_data(slot.data.load());

    if (depth >= old.depth || old.age != current_age) {
        uint64_t data = pack_tt_data(score, best_move, depth, flag, current_age);
        slot.data.store(data);
        slot.key_xor_data.store(hash ^ data);
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
    for (TTSlot& slot : transposition_table) {
        slot.key_xor_data.store(0);
        slot.data.store(0);
    }
}
