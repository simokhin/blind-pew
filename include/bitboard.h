#pragma once
#include <cstdint>
#include <random>

#include "board.h"

struct Position;

using Bitboard = uint64_t;

constexpr Bitboard FILE_A = 0x0101010101010101ULL;
constexpr Bitboard FILE_H = 0x8080808080808080ULL;

constexpr Bitboard square_bb(int square) { return 1ULL << square; }

constexpr std::array<Bitboard, 64> make_knight_attacks() {
    std::array<Bitboard, 64> result{};

    for (int square = 0; square < 64; square++) {
        int rank = rank_of(square);
        int file = file_of(square);

        for (const Offset& o : knight_offsets) {
            int new_rank = rank + o.dr;
            int new_file = file + o.df;

            if (is_valid_square(new_rank, new_file)) {
                result[square] |= square_bb(square_of(new_rank, new_file));
            }
        }
    }
    return result;
}

constexpr std::array<Bitboard, 64> make_king_attacks() {
    std::array<Bitboard, 64> result{};

    for (int square = 0; square < 64; square++) {
        int rank = rank_of(square);
        int file = file_of(square);

        for (const Offset& o : king_offsets) {
            int new_rank = rank + o.dr;
            int new_file = file + o.df;

            if (is_valid_square(new_rank, new_file)) {
                result[square] |= square_bb(square_of(new_rank, new_file));
            }
        }
    }

    return result;
}

constexpr std::array<std::array<Bitboard, 64>, 2> make_pawn_attacks() {
    std::array<std::array<Bitboard, 64>, 2> result{};

    for (int square = 0; square < 64; square++) {
        int rank = rank_of(square);
        int file = file_of(square);

        for (Color color : {Color::White, Color::Black}) {
            int direction = color == Color::White ? 1 : -1;
            for (int df : {-1, 1}) {
                int new_rank = rank + direction;
                int new_file = file + df;
                if (is_valid_square(new_rank, new_file)) {
                    result[static_cast<int>(color)][square] |=
                        square_bb(square_of(new_rank, new_file));
                }
            }
        }
    }

    return result;
}

constexpr Bitboard rook_mask(int square) {
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

constexpr Bitboard bishop_mask(int square) {
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

constexpr std::array<Bitboard, 64> make_rook_masks() {
    std::array<Bitboard, 64> result{};

    for (int square = 0; square < 64; square++) {
        result[square] = rook_mask(square);
    }

    return result;
}

constexpr std::array<Bitboard, 64> make_bishop_masks() {
    std::array<Bitboard, 64> result{};

    for (int square = 0; square < 64; square++) {
        result[square] = bishop_mask(square);
    }

    return result;
}

inline constexpr std::array<Bitboard, 64> knight_attacks = make_knight_attacks();
static_assert(knight_attacks[0] == (square_bb(10) | square_bb(17)),
              "knight_attacks[A1] must be C2 and B3");

inline constexpr std::array<Bitboard, 64> king_attacks = make_king_attacks();
static_assert(king_attacks[0] == (square_bb(1) | square_bb(8) | square_bb(9)),
              "king_attacks[A1] must be A2, B1 and B2");

inline constexpr std::array<std::array<Bitboard, 64>, 2> pawn_attacks = make_pawn_attacks();
static_assert(pawn_attacks[static_cast<int>(Color::White)][8] == square_bb(17),
              "white pawn on A2 must attack only B3");
static_assert(pawn_attacks[static_cast<int>(Color::Black)][55] == square_bb(46),
              "black pawn on H7 must attack only G6");

inline constexpr std::array<Bitboard, 64> rook_masks = make_rook_masks();
static_assert(std::popcount(rook_masks[0]) == 12, "rook mask on A1 must have 12 squares");

inline constexpr std::array<Bitboard, 64> bishop_masks = make_bishop_masks();
static_assert(std::popcount(bishop_masks[0]) == 6, "bishop mask on A1 must have 6 squares");

Bitboard random_sparse_u64(std::mt19937_64& rng);

Bitboard find_rook_magic(int square, std::mt19937_64& rng);
Bitboard find_bishop_magic(int square, std::mt19937_64& rng);

void init_rook_magics();
void init_bishop_magics();

bool is_magic_valid(int square, Bitboard magic, Bitboard mask, int bits,
                    Bitboard (*attacks_fn)(int, Bitboard));

int pop_lsb(Bitboard& bb);
Bitboard lsb_bb(Bitboard bb);
int popcount(Bitboard bb);

Bitboard attackers_to(const Position& position, int square, Bitboard occupancy);
Bitboard least_valuable_attacker(const Position& position, Bitboard attackers, Color side,
                                 PieceType& out_type);

Bitboard pawn_attacks_bulk(Bitboard pawns, Color color);

Bitboard rook_attacks_from(int square, Bitboard occupancy);
Bitboard bishop_attacks_from(int square, Bitboard occupancy);
