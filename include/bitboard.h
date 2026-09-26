#pragma once
#include <cstdint>
#include <random>

using Bitboard = uint64_t;

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
