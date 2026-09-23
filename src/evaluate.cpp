#include "evaluate.h"

// Массив ценности фигур
std::array<int, 13> values = {0, 100, 320, 330, 500, 900, 0, 100, 320, 330, 500, 900, 0};

int evaluate(const Position& position) {
    int evaluation = 0;

    for (int square = 0; square < 64; square++) {
        Piece piece = position.board[square];

        if (piece == Piece::None) {
            continue;
        }

        // Бонус за ценность фигуры
        int value = values[static_cast<int>(piece)];

        if (color_of(piece) == position.side_to_move) {
            evaluation += value;
        } else {
            evaluation -= value;
        }
    }

    return evaluation;
}