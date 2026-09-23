#pragma once
#include "board.h"
#include <cstdint>

constexpr uint8_t WHITE_KINGSIDE = 1;
constexpr uint8_t WHITE_QUEENSIDE = 2;
constexpr uint8_t BLACK_KINGSIDE = 4;
constexpr uint8_t BLACK_QUEENSIDE = 8;

struct Position
{
    Board board;
    Color side_to_move;
    uint8_t castling_rights;
    int en_passant_target; // -1, если недоступно
    int halfmove_clock;    // для правила 50 ходов
    int fullmove_number;
};

Position make_start_position();