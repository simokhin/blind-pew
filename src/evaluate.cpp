#include "evaluate.h"

// Массив ценности фигур
std::array<int, 13> values = {0, 100, 320, 330, 500, 900, 0, 100, 320, 330, 500, 900, 0};

// PST
std::array<int, 64> pawn_pst = {
    0,  0,  0,  0,  0,  0,  0,  0,  5,  10, 10, -20, -20, 10, 10, 5,  5, -5, -10, 0,  0,  -10,
    -5, 5,  0,  0,  0,  20, 20, 0,  0,  0,  5,  5,   10,  25, 25, 10, 5, 5,  10,  10, 20, 30,
    30, 20, 10, 10, 50, 50, 50, 50, 50, 50, 50, 50,  0,   0,  0,  0,  0, 0,  0,   0,

};

int evaluate(const Position& position) {
    int evaluation = 0;

    for (int square = 0; square < 64; square++) {
        Piece piece = position.board[square];

        if (piece == Piece::None) {
            continue;
        }

        // Бонус за ценность фигуры
        int value = values[static_cast<int>(piece)];

        // Бонус за расположение фигур
        value += pst_bonus(piece, square);

        if (color_of(piece) == position.side_to_move) {
            evaluation += value;
        } else {
            evaluation -= value;
        }
    }

    return evaluation;
}

int pst_bonus(Piece piece, int square) {
    switch (piece) {
        case Piece::WP:
            return pawn_pst[square];
        case Piece::BP:
            return pawn_pst[mirror_square(square)];
        default:
            return 0;
    }
}
