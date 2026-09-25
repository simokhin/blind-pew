#pragma once
#include <cstdint>

#include "board.h"
#include "move.h"

constexpr uint8_t WHITE_KINGSIDE = 1;
constexpr uint8_t WHITE_QUEENSIDE = 2;
constexpr uint8_t BLACK_KINGSIDE = 4;
constexpr uint8_t BLACK_QUEENSIDE = 8;

struct Position {
    Board board;
    Color side_to_move;
    uint8_t castling_rights;
    int en_passant_target;  // -1, если недоступно
    int halfmove_clock;     // для правила 50 ходов
    int fullmove_number;

    // Отслеживаем позицию короля
    int white_king_square;
    int black_king_square;

    uint64_t zobrist_hash;
};

struct UndoInfo {
    Piece captured_piece;
    uint8_t castling_rights;
    int en_passant_target;
    int halfmove_clock;
    int fullmove_number;
    uint64_t zobrist_hash;
};

UndoInfo make_move(Position& position, const Move& move);
void unmake_move(Position& position, const Move& move, const UndoInfo& undo);
int king_square_of(const Position& position, Color color);

void put_piece(Position& position, Piece piece, int square);
void remove_piece(Position& position, int square);
void move_piece(Position& position, int from, int to);

int make_null_move(Position& position);
void unmake_null_move(Position& position, int old_en_passant_target);