#include "evaluate.h"


// PST
std::array<int, 64> pawn_pst = {
    0,  0,  0,  0,  0,  0,  0,  0,  5,  10, 10, -20, -20, 10, 10, 5,  5, -5, -10, 0,  0,  -10,
    -5, 5,  0,  0,  0,  20, 20, 0,  0,  0,  5,  5,   10,  25, 25, 10, 5, 5,  10,  10, 20, 30,
    30, 20, 10, 10, 50, 50, 50, 50, 50, 50, 50, 50,  0,   0,  0,  0,  0, 0,  0,   0,

};

std::array<int, 64> knight_pst = {
    -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   5,   5,   0,   -20, -40,
    -30, 5,   10,  15,  15,  10,  5,   -30, -30, 0,   15,  20,  20,  15,  0,   -30,
    -30, 5,   15,  20,  20,  15,  5,   -30, -30, 0,   10,  15,  15,  10,  0,   -30,
    -40, -20, 0,   0,   0,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50,

};

std::array<int, 64> bishop_pst = {
    -20, -10, -10, -10, -10, -10, -10, -20, -10, 5,   0,   0,   0,   0,   5,   -10,
    -10, 10,  10,  10,  10,  10,  10,  -10, -10, 0,   10,  10,  10,  10,  0,   -10,
    -10, 5,   5,   10,  10,  5,   5,   -10, -10, 0,   5,   10,  10,  5,   0,   -10,
    -10, 0,   0,   0,   0,   0,   0,   -10, -20, -10, -10, -10, -10, -10, -10, -20,
};

std::array<int, 64> rook_pst = {
    0, 0,  0,  5,  5, 0,  0,  0,  -5, 0,  0,  0, 0, 0, 0, -5, -5, 0,  0,  0, 0, 0,
    0, -5, -5, 0,  0, 0,  0,  0,  0,  -5, -5, 0, 0, 0, 0, 0,  0,  -5, -5, 0, 0, 0,
    0, 0,  0,  -5, 5, 10, 10, 10, 10, 10, 10, 5, 0, 0, 0, 0,  0,  0,  0,  0,
};

std::array<int, 64> queen_pst = {
    -20, -10, -10, -5, -5, -10, -10, -20, -10, 0,   0,   0,  0,  0,   0,   -10,
    -10, 0,   5,   5,  5,  5,   0,   -10, -5,  0,   5,   5,  5,  5,   0,   -5,
    0,   0,   5,   5,  5,  5,   0,   -5,  -10, 5,   5,   5,  5,  5,   0,   -10,
    -10, 0,   5,   0,  0,  0,   0,   -10, -20, -10, -10, -5, -5, -10, -10, -20,
};

std::array<int, 64> king_pst_mg = {
    20,  30,  10,  0,   0,   10,  30,  20,  20,  20,  0,   0,   0,   0,   20,  20,
    -10, -20, -20, -20, -20, -20, -20, -10, -20, -30, -30, -40, -40, -30, -30, -20,
    -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30,
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
        case Piece::WN:
            return knight_pst[square];
        case Piece::BN:
            return knight_pst[mirror_square(square)];
        case Piece::WB:
            return bishop_pst[square];
        case Piece::BB:
            return bishop_pst[mirror_square(square)];
        case Piece::WR:
            return rook_pst[square];
        case Piece::BR:
            return rook_pst[mirror_square(square)];
        case Piece::WQ:
            return queen_pst[square];
        case Piece::BQ:
            return queen_pst[mirror_square(square)];
        case Piece::WK:
            return king_pst_mg[square];
        case Piece::BK:
            return king_pst_mg[mirror_square(square)];
        default:
            return 0;
    }
}
