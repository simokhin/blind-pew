#include "zobrist.h"

#include <random>

#include "board.h"

std::array<std::array<uint64_t, 64>, 13> piece_square_keys;
uint64_t side_to_move_key;
std::array<uint64_t, 16> castling_keys;
std::array<uint64_t, 8> en_passant_file_keys;

void init_zobrist_keys() {
    std::mt19937_64 rng(12345);
    side_to_move_key = rng();

    for (uint64_t& key : castling_keys) {
        key = rng();
    }

    for (uint64_t& key : en_passant_file_keys) {
        key = rng();
    }

    for (std::array<uint64_t, 64>& row : piece_square_keys) {
        for (uint64_t& key : row) {
            key = rng();
        }
    }
}

uint64_t compute_zobrist_hash(const Position& position) {
    uint64_t hash = 0;

    for (int square = 0; square < 64; square++) {
        Piece piece = position.board[square];

        if (piece != Piece::None) {
            hash ^= piece_square_keys[static_cast<int>(piece)][square];
        }
    }

    if (position.side_to_move == Color::Black) {
        hash ^= side_to_move_key;
    }

    hash ^= castling_keys[position.castling_rights];

    if (position.en_passant_target != -1) {
        hash ^= en_passant_file_keys[file_of(position.en_passant_target)];
    }

    return hash;
}
