#include "bitboard.h"

#include "magic_constants.h"
#include "position.h"

// Строит битборд, в котором установлен один бит на позиции square
Bitboard square_bb(int square) {
    return 1ULL << square;  // 1ULL - unsigned long long
}

Bitboard knight_attacks[64];
Bitboard king_attacks[64];
Bitboard pawn_attacks[2][64];
Bitboard rook_attacks_table[64][4096];
Bitboard bishop_attacks_table[64][512];
Bitboard rook_masks[64];
Bitboard bishop_masks[64];

void init_knight_attacks() {
    for (int square = 0; square < 64; square++) {
        int rank = rank_of(square);
        int file = file_of(square);

        for (const Offset& o : knight_offsets) {
            int new_rank = rank + o.dr;
            int new_file = file + o.df;

            if (is_valid_square(new_rank, new_file)) {
                knight_attacks[square] |= square_bb(square_of(new_rank, new_file));
            }
        }
    }
}

void init_king_attacks() {
    for (int square = 0; square < 64; square++) {
        int rank = rank_of(square);
        int file = file_of(square);

        for (const Offset& o : king_offsets) {
            int new_rank = rank + o.dr;
            int new_file = file + o.df;

            if (is_valid_square(new_rank, new_file)) {
                king_attacks[square] |= square_bb(square_of(new_rank, new_file));
            }
        }
    }
}

void init_pawn_attacks() {
    for (int square = 0; square < 64; square++) {
        int rank = rank_of(square);
        int file = file_of(square);

        for (Color color : {Color::White, Color::Black}) {
            int direction = color == Color::White ? 1 : -1;
            for (int df : {-1, 1}) {
                int new_rank = rank + direction;
                int new_file = file + df;
                if (is_valid_square(new_rank, new_file)) {
                    pawn_attacks[static_cast<int>(color)][square] |=
                        square_bb(square_of(new_rank, new_file));
                }
            }
        }
    }
}

Bitboard rook_mask(int square) {
    Bitboard mask = 0;

    int rank = rank_of(square);
    int file = file_of(square);

    for (const Offset& d : rook_directions) {
        int new_rank = rank + d.dr;
        int new_file = file + d.df;

        while (true) {
            if (is_valid_square(new_rank, new_file)) {
                int next_rank = new_rank + d.dr;
                int next_file = new_file + d.df;

                if (!is_valid_square(next_rank, next_file)) {
                    break;
                }

                mask |= square_bb(square_of(new_rank, new_file));
                new_rank += d.dr;
                new_file += d.df;
            } else {
                break;
            }
        }
    }

    return mask;
}

Bitboard bishop_mask(int square) {
    Bitboard mask = 0;

    int rank = rank_of(square);
    int file = file_of(square);

    for (const Offset& d : bishop_directions) {
        int new_rank = rank + d.dr;
        int new_file = file + d.df;

        while (true) {
            if (is_valid_square(new_rank, new_file)) {
                int next_rank = new_rank + d.dr;
                int next_file = new_file + d.df;

                if (!is_valid_square(next_rank, next_file)) {
                    break;
                }

                mask |= square_bb(square_of(new_rank, new_file));
                new_rank += d.dr;
                new_file += d.df;
            } else {
                break;
            }
        }
    }

    return mask;
}

Bitboard rook_attacks_otf(int square, Bitboard occupancy) {
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

Bitboard bishop_attacks_otf(int square, Bitboard occupancy) {
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

Bitboard set_occupancy(int index, Bitboard mask) {
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
        rook_masks[square] = mask;

        int bits = rook_relevant_bits[square];
        int count =
            1 << bits;  // 2^bits - количество возможных комбинаций занятости для этой клетки
        for (int index = 0; index < count; index++) {
            // Занятость для этой комбинации
            Bitboard occupancy = set_occupancy(index, mask);

            // Вычисляем атаку ладьи, исходя из вычисленной занятости
            Bitboard attack = rook_attacks_otf(square, occupancy);

            int magic_index = (occupancy * rook_magics[square]) >> (64 - bits);

            rook_attacks_table[square][magic_index] = attack;
        }
    }
}

void init_bishop_magics() {
    for (int square = 0; square < 64; square++) {
        Bitboard mask = bishop_mask(square);
        bishop_masks[square] = mask;

        int bits = bishop_relevant_bits[square];
        int count =
            1 << bits;  // 2^bits - количество возможных комбинаций занятости для этой клетки
        for (int index = 0; index < count; index++) {
            // Занятость для этой комбинации
            Bitboard occupancy = set_occupancy(index, mask);

            // Вычисляем атаку ладьи, исходя из вычисленной занятости
            Bitboard attack = bishop_attacks_otf(square, occupancy);

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

        int magic_index = (occupancy * magic) >> (64 - bits);

        if (used[magic_index] && table[magic_index] != attack) {
            return false;
        }

        used[magic_index] = true;
        table[magic_index] = attack;
    }
    return true;
}

int pop_lsb(Bitboard& bb) {
    int bit_number = __builtin_ctzll(bb);
    bb &= bb - 1;
    return bit_number;
}

// Возвращает битборд с одним установленным битом (самый младший бит bb)
Bitboard lsb_bb(Bitboard bb) { return bb & (-bb); }

int popcount(Bitboard bb) { return __builtin_popcountll(bb); }

// Возвращает битборд всех фигур, атакующих клетку
Bitboard attackers_to(const Position& position, int square, Bitboard occupancy) {
    Bitboard attackers = 0;

    attackers |= knight_attacks[square] &
                 position.by_piece_type[static_cast<int>(PieceType::Knight)] & occupancy;

    attackers |= king_attacks[square] & position.by_piece_type[static_cast<int>(PieceType::King)] &
                 occupancy;

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

// Возвращает битборд с наименее ценным атакующим клетку
Bitboard least_valuable_attacker(const Position& position, Bitboard attackers, Color side,
                                 PieceType& out_type) {
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

// Берем все клетки, атакованные пешками
Bitboard pawn_attacks_bulk(Bitboard pawns, Color color) {
    if (color == Color::White) {
        return ((pawns & ~FILE_A) << 7) | ((pawns & ~FILE_H) << 9);
    } else {
        return ((pawns & ~FILE_H) >> 7) | ((pawns & ~FILE_A) >> 9);
    }
}

Bitboard rook_attacks_from(int square, Bitboard occupancy) {
    Bitboard relevant = occupancy & rook_masks[square];
    int bits = rook_relevant_bits[square];
    int magic_index = (relevant * rook_magics[square]) >> (64 - bits);
    return rook_attacks_table[square][magic_index];
}

Bitboard bishop_attacks_from(int square, Bitboard occupancy) {
    Bitboard relevant = occupancy & bishop_masks[square];
    int bits = bishop_relevant_bits[square];
    int magic_index = (relevant * bishop_magics[square]) >> (64 - bits);
    return bishop_attacks_table[square][magic_index];
}
