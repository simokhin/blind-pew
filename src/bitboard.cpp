#include "bitboard.h"

#include "magic_constants.h"
#include "position.h"

namespace {
// Таблицы атак для магических битбордов: `[клетка][магический индекс]`.
// Второй размер - максимум 2^bits по всем клекткам (12 бит у ладьи, 9 у слона).
Bitboard rook_attacks_table[64][4096];
Bitboard bishop_attacks_table[64][512];

// Считает атаки ладьи обходом лучей до первой занятой клетки включительно.
// Медленно, поэтому используется только при заполнении таблиц.
constexpr Bitboard rook_attacks_otf(int square, Bitboard occupancy) {
    Bitboard attacks = 0;

    int rank = rank_of(square);
    int file = file_of(square);

    for (const Offset& d : rook_directions) {
        int new_rank = rank + d.dr;
        int new_file = file + d.df;

        while (true) {
            if (is_valid_square(new_rank, new_file)) {
                if (occupancy & square_bb(square_of(new_rank, new_file))) {
                    attacks |= square_bb(square_of(new_rank, new_file));
                    break;
                }

                attacks |= square_bb(square_of(new_rank, new_file));
                new_rank += d.dr;
                new_file += d.df;
            } else {
                break;
            }
        }
    }

    return attacks;
}

// Считает атаки слона обходом лучей до первой занятой клетки включительно.
// Медленно, поэтому используется только при заполнении таблиц.
constexpr Bitboard bishop_attacks_otf(int square, Bitboard occupancy) {
    Bitboard attacks = 0;

    int rank = rank_of(square);
    int file = file_of(square);

    for (const Offset& d : bishop_directions) {
        int new_rank = rank + d.dr;
        int new_file = file + d.df;

        while (true) {
            if (is_valid_square(new_rank, new_file)) {
                if (occupancy & square_bb(square_of(new_rank, new_file))) {
                    attacks |= square_bb(square_of(new_rank, new_file));
                    break;
                }

                attacks |= square_bb(square_of(new_rank, new_file));
                new_rank += d.dr;
                new_file += d.df;
            } else {
                break;
            }
        }
    }

    return attacks;
}

// Раскладывает биты числа `index` по установленным битам `mask`: i-й установленный бит маски
// берётся равным i-му биту `index`. Перебор `index` от 0 до 2^popcount(mask) - 1 даёт все возможные
// подмножества маски.
constexpr Bitboard set_occupancy(int index, Bitboard mask) {
    Bitboard occupancy = 0;
    int bit_index = 0;

    for (int square = 0; square < 64; square++) {
        if (mask & square_bb(square)) {
            if (index & (1 << bit_index)) {
                occupancy |= square_bb(square);
            }
            bit_index++;
        }
    }

    return occupancy;
}
}  // namespace

Bitboard random_sparse_u64(std::mt19937_64& rng) { return rng() & rng() & rng(); }

Bitboard find_rook_magic(int square, std::mt19937_64& rng) {
    Bitboard mask = rook_mask(square);
    int bits = rook_relevant_bits[square];
    while (true) {
        Bitboard candidate = random_sparse_u64(rng);
        if (is_magic_valid(square, candidate, mask, bits, rook_attacks_otf)) {
            return candidate;
        }
    }
}

Bitboard find_bishop_magic(int square, std::mt19937_64& rng) {
    Bitboard mask = bishop_mask(square);
    int bits = bishop_relevant_bits[square];
    while (true) {
        Bitboard candidate = random_sparse_u64(rng);
        if (is_magic_valid(square, candidate, mask, bits, bishop_attacks_otf)) {
            return candidate;
        }
    }
}

void init_rook_magics() {
    for (int square = 0; square < 64; square++) {
        Bitboard mask = rook_mask(square);

        int bits = rook_relevant_bits[square];
        int count =
            1 << bits;  // 2^bits - количество возможных комбинаций занятости для этой клетки
        for (int index = 0; index < count; index++) {
            Bitboard occupancy = set_occupancy(index, mask);

            Bitboard attack = rook_attacks_otf(square, occupancy);

            // Разные занятости могут дать одинаковый индекс: это допустимо, потому что магическое
            // число подобрано так, что при совпадении индекса совпадают и атаки.
            int magic_index = (occupancy * rook_magics[square]) >> (64 - bits);

            rook_attacks_table[square][magic_index] = attack;
        }
    }
}

void init_bishop_magics() {
    for (int square = 0; square < 64; square++) {
        Bitboard mask = bishop_mask(square);

        int bits = bishop_relevant_bits[square];
        int count =
            1 << bits;  // 2^bits - количество возможных комбинаций занятости для этой клетки
        for (int index = 0; index < count; index++) {
            Bitboard occupancy = set_occupancy(index, mask);

            Bitboard attack = bishop_attacks_otf(square, occupancy);

            // Разные занятости могут дать одинаковый индекс: это допустимо, потому что магическое
            // число подобрано так, что при совпадении индекса совпадают и атаки.
            int magic_index = (occupancy * bishop_magics[square]) >> (64 - bits);

            bishop_attacks_table[square][magic_index] = attack;
        }
    }
}

bool is_magic_valid(int square, Bitboard magic, Bitboard mask, int bits,
                    Bitboard (*attacks_fn)(int, Bitboard)) {
    std::array<bool, 4096> used{};
    std::array<Bitboard, 4096> table;

    int count = 1 << bits;

    for (int index = 0; index < count; index++) {
        Bitboard occupancy = set_occupancy(index, mask);
        Bitboard attack = attacks_fn(square, occupancy);

        // Разные занятости могут дать одинаковый индекс: это допустимо, потому что магическое
        // число подобрано так, что при совпадении индекса совпадают и атаки.
        int magic_index = (occupancy * magic) >> (64 - bits);

        if (used[magic_index] && table[magic_index] != attack) {
            return false;
        }

        used[magic_index] = true;
        table[magic_index] = attack;
    }
    return true;
}

Bitboard attackers_to(const Position& position, int square, Bitboard occupancy) {
    Bitboard attackers = 0;

    attackers |= knight_attacks[square] &
                 position.by_piece_type[static_cast<int>(PieceType::Knight)] & occupancy;

    attackers |= king_attacks[square] & position.by_piece_type[static_cast<int>(PieceType::King)] &
                 occupancy;

    // Пешка цвета X атакует `square`, если стоит там, куда попала бы пешка противоположного цвета,
    // стоящая до `square`. Поэтому для поиска белых пешек берётся таблица чёрных и наоборот.
    attackers |= pawn_attacks[static_cast<int>(Color::White)][square] &
                 position.by_color[static_cast<int>(Color::Black)] &
                 position.by_piece_type[static_cast<int>(PieceType::Pawn)] & occupancy;

    attackers |= pawn_attacks[static_cast<int>(Color::Black)][square] &
                 position.by_color[static_cast<int>(Color::White)] &
                 position.by_piece_type[static_cast<int>(PieceType::Pawn)] & occupancy;

    Bitboard rook_attacks_here = rook_attacks_from(square, occupancy);

    attackers |= rook_attacks_here &
                 (position.by_piece_type[static_cast<int>(PieceType::Rook)] |
                  position.by_piece_type[static_cast<int>(PieceType::Queen)]) &
                 occupancy;

    Bitboard bishop_attacks_here = bishop_attacks_from(square, occupancy);

    attackers |= bishop_attacks_here &
                 (position.by_piece_type[static_cast<int>(PieceType::Bishop)] |
                  position.by_piece_type[static_cast<int>(PieceType::Queen)]) &
                 occupancy;

    return attackers;
}

Bitboard least_valuable_attacker(const Position& position, Bitboard attackers, Color side,
                                 PieceType& out_type) {
    // Перебор идёт по значению `PieceType`: порядок от пешки к королю совпадает с порядком
    // возрастания ценности, на который опирается функция.
    for (int type = 0; type <= static_cast<int>(PieceType::King); type++) {
        Bitboard candidates =
            attackers & position.by_color[static_cast<int>(side)] & position.by_piece_type[type];
        if (candidates != 0) {
            out_type = static_cast<PieceType>(type);
            return lsb_bb(candidates);
        }
    }

    return 0;
}

Bitboard pawn_attacks_bulk(Bitboard pawns, Color color) {
    // Белые пешки: сдвиг на 7 - это атака вперёд-влево, на 9 - вперёд-вправо; у черных наоборот
    // (сдвиг вниз). Маски `FILE_A`/`FILE_H` убирают пешки, которые при сдвиге перешли бы на
    // противоположный край доски.
    if (color == Color::White) {
        return ((pawns & ~FILE_A) << 7) | ((pawns & ~FILE_H) << 9);
    } else {
        return ((pawns & ~FILE_H) >> 7) | ((pawns & ~FILE_A) >> 9);
    }
}

Bitboard rook_attacks_from(int square, Bitboard occupancy) {
    Bitboard relevant = occupancy & rook_masks[square];
    int bits = rook_relevant_bits[square];

    // Магический индекс: произведение значимой занятости на магическое число, затем сдвиг оставляет
    // в старших `bits` битах номер, уникальный для атак этой раскладки.
    int magic_index = (relevant * rook_magics[square]) >> (64 - bits);
    return rook_attacks_table[square][magic_index];
}

Bitboard bishop_attacks_from(int square, Bitboard occupancy) {
    Bitboard relevant = occupancy & bishop_masks[square];
    int bits = bishop_relevant_bits[square];

    // Магический индекс: произведение значимой занятости на магическое число, затем сдвиг оставляет
    // в старших `bits` битах номер, уникальный для атак этой раскладки.
    int magic_index = (relevant * bishop_magics[square]) >> (64 - bits);
    return bishop_attacks_table[square][magic_index];
}
