#pragma once
#include <cstdint>
#include <random>

#include "board.h"

struct Position;

using Bitboard = uint64_t;

constexpr Bitboard FILE_A = 0x0101010101010101ULL;
constexpr Bitboard FILE_H = 0x8080808080808080ULL;

Bitboard square_bb(int square);

extern Bitboard knight_attacks[64];
extern Bitboard king_attacks[64];
extern Bitboard pawn_attacks[2][64];
extern Bitboard rook_attacks_table[64][4096];
extern Bitboard bishop_attacks_table[64][512];
extern Bitboard rook_masks[64];
extern Bitboard bishop_masks[64];

Bitboard rook_mask(int square);
Bitboard bishop_mask(int square);
Bitboard rook_attacks_otf(int square, Bitboard occupancy);
Bitboard bishop_attacks_otf(int square, Bitboard occupancy);

Bitboard set_occupancy(int index, Bitboard mask);

Bitboard random_sparse_u64(std::mt19937_64& rng);

Bitboard find_rook_magic(int square, std::mt19937_64& rng);
Bitboard find_bishop_magic(int square, std::mt19937_64& rng);

void init_knight_attacks();
void init_king_attacks();
void init_pawn_attacks();
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
