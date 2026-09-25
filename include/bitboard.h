#pragma once
#include <cstdint>

using Bitboard = uint64_t;

Bitboard square_bb(int square);

extern Bitboard knight_attacks[64];
extern Bitboard king_attacks[64];
extern Bitboard pawn_attacks[2][64];

Bitboard rook_mask(int square);
Bitboard bishop_mask(int square);
Bitboard rook_attacks_otf(int square, Bitboard occupancy);
Bitboard bishop_attacks_otf(int square, Bitboard occupancy);

void init_knight_attacks();
void init_king_attacks();
void init_pawn_attacks();
