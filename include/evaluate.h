#pragma once
#include "bitboard.h"
#include "position.h"

// Массив ценности фигур
const std::array<int, 13> values = {0, 100, 320, 330, 500, 900, 0, 100, 320, 330, 500, 900, 0};

constexpr int TOTAL_PHASE = 24;

int evaluate(const Position& position);
int pst_bonus(Piece piece, int square, int phase);

int compute_phase(const Position& position);

// Moblity
int knight_mobility(const Position& position, int square, Color color);
int bishop_mobility(const Position& position, int square, Color color);
int rook_mobility(const Position& position, int square, Color color);
int queen_mobility(const Position& position, int square, Color color);
int mobility_count(Bitboard attacks, const Position& position, Color color);
int mobility_bonus(Piece piece, int square, const Position& position, int phase);
