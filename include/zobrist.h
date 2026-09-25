#pragma once

#include <array>
#include <cstdint>

#include "position.h"

extern std::array<std::array<uint64_t, 64>, 13> piece_square_keys;
extern uint64_t side_to_move_key;
extern std::array<uint64_t, 16> castling_keys;
extern std::array<uint64_t, 8> en_passant_file_keys;

void init_zobrist_keys();

uint64_t compute_zobrist_hash(const Position& position);