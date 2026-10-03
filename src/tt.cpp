#include "tt.h"

#include <atomic>
#include <vector>

#include "constants.h"

namespace {
// Слот таблицы: два числа `uint64_t`, которые каждый поток читает и пишет независимо, без
// блокировок. В `data` лежат упакованные данные записи, в `key_xor_data` - `hash ^ data`.
// Если слово `data` прочитано от одной записи, а `key_xor_data` от другой, проверка `(key ^ data)
// == hash` в `tt_probe` не сойдется, и такая запись игнорируется.
struct TTSlot {
    std::atomic<uint64_t> key_xor_data;
    std::atomic<uint64_t> data;
};

std::vector<TTSlot> transposition_table;

std::atomic<uint8_t> current_age = 0;

// Упаковывает запись в 64 бита. Раскладка (номера битов): 0-15 оценка, 16-31 ход, 32-39 глубина,
// 40-47 флаг, 48-55 возраст; старшие 8 бит не используются.
uint64_t pack_tt_data(int score, Move best_move, int depth, TTFlag flag, uint8_t age) {
    return static_cast<uint64_t>(static_cast<uint16_t>(score)) |
           (static_cast<uint64_t>(static_cast<uint16_t>(best_move.raw())) << 16) |
           (static_cast<uint64_t>(static_cast<uint8_t>(depth)) << 32) |
           (static_cast<uint64_t>(static_cast<uint8_t>(flag)) << 40) |
           (static_cast<uint64_t>(age) << 48);
}

// Обратное преобразование к `pack_tt_data`. Поле `hash` не заполняется: его знает вызывающий.
TTEntry unpack_tt_data(uint64_t data) {
    TTEntry entry;
    entry.score = static_cast<int16_t>(data);
    entry.best_move = Move::from_raw(static_cast<uint16_t>(data >> 16));
    entry.depth = static_cast<int8_t>(data >> 32);
    entry.flag = static_cast<TTFlag>(static_cast<uint8_t>(data >> 40));
    entry.age = static_cast<uint8_t>(data >> 48);
    return entry;
}
}  // namespace

void tt_new_search() { current_age++; }

void resize_transposition_table(int size_mb) {
    size_t max_entries = (static_cast<size_t>(size_mb) * 1024 * 1024) / sizeof(TTSlot);

    size_t entries = 1;
    while (entries * 2 <= max_entries) {
        entries *= 2;
    }

    transposition_table = std::vector<TTSlot>(entries);
}

bool tt_probe(uint64_t hash, TTEntry& out) {
    size_t index = hash & (transposition_table.size() - 1);

    TTSlot& slot = transposition_table[index];
    uint64_t data = slot.data.load();
    uint64_t key = slot.key_xor_data.load();

    // Слот принадлежит этой позиции и записан целиком (см. `TTSlot`)
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

    // Политика замены: перезаписываем, если новая глубина не меньше старой или старая запись
    // осталась от предыдущего поиска.
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
