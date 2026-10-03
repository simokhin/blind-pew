#pragma once

#include <array>
#include <string>
#include <vector>

enum class Piece { None, WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK };

enum class PieceType { Pawn, Knight, Bishop, Rook, Queen, King, None };

PieceType piece_type_of(Piece piece);

struct Offset {
    int dr;
    int df;
};

inline constexpr std::array<Offset, 8> knight_offsets = {
    Offset{1, 2}, Offset{1, -2}, Offset{-1, 2}, Offset{-1, -2},
    Offset{2, 1}, Offset{2, -1}, Offset{-2, 1}, Offset{-2, -1},
};

inline constexpr std::array<Offset, 8> king_offsets = {
    Offset{1, 0},  Offset{1, 1},   Offset{1, -1}, Offset{-1, 0},
    Offset{-1, 1}, Offset{-1, -1}, Offset{0, 1},  Offset{0, -1},
};

inline constexpr std::array<Offset, 4> rook_directions = {
    Offset{1, 0},
    Offset{-1, 0},
    Offset{0, 1},
    Offset{0, -1},
};

inline constexpr std::array<Offset, 4> bishop_directions = {
    Offset{1, 1},
    Offset{1, -1},
    Offset{-1, 1},
    Offset{-1, -1},
};

enum class Square {
    A1,
    B1,
    C1,
    D1,
    E1,
    F1,
    G1,
    H1,
    A2,
    B2,
    C2,
    D2,
    E2,
    F2,
    G2,
    H2,
    A3,
    B3,
    C3,
    D3,
    E3,
    F3,
    G3,
    H3,
    A4,
    B4,
    C4,
    D4,
    E4,
    F4,
    G4,
    H4,
    A5,
    B5,
    C5,
    D5,
    E5,
    F5,
    G5,
    H5,
    A6,
    B6,
    C6,
    D6,
    E6,
    F6,
    G6,
    H6,
    A7,
    B7,
    C7,
    D7,
    E7,
    F7,
    G7,
    H7,
    A8,
    B8,
    C8,
    D8,
    E8,
    F8,
    G8,
    H8
};

enum class Color { White, Black, None };

Color color_of(Piece piece);

using Board = std::array<Piece, 64>;

void print_board(const Board& board);

int rank_of(int square);
int file_of(int square);
int square_of(int rank, int file);

int mirror_square(int square);

bool is_valid_square(int rank, int file);
Color opposite_color(Color color);

int square_from_algebraic(const std::string& s);
std::string algebraic_from_square(int square);
