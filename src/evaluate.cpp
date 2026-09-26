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

std::array<int, 64> king_pst_eg = {
    -50, -30, -30, -30, -30, -30, -30, -50, -30, -30, 0,   0,   0,   0,   -30, -30,
    -30, -10, 20,  30,  30,  20,  -10, -30, -30, -10, 30,  40,  40,  30,  -10, -30,
    -30, -10, 30,  40,  40,  30,  -10, -30, -30, -10, 20,  30,  30,  20,  -10, -30,
    -30, -20, -10, 0,   0,   -10, -20, -30, -50, -40, -30, -20, -20, -30, -40, -50,
};

std::array<int, 13> phase_weights = {0, 0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0};

int evaluate(const Position& position) {
    int evaluation = 0;
    int phase = compute_phase(position);

    for (Piece piece : {Piece::WP, Piece::WN, Piece::WB, Piece::WR, Piece::WQ, Piece::WK, Piece::BP,
                        Piece::BN, Piece::BB, Piece::BR, Piece::BQ, Piece::BK}) {
        Bitboard pieces = position.by_color[static_cast<int>(color_of(piece))] &
                          position.by_piece_type[static_cast<int>(piece_type_of(piece))];

        while (pieces != 0) {
            int square = pop_lsb(pieces);

            // Бонус за ценность фигуры
            int value = values[static_cast<int>(piece)];

            // Бонус за расположение фигур
            value += pst_bonus(piece, square, phase);

            if (color_of(piece) == position.side_to_move) {
                evaluation += value;
            } else {
                evaluation -= value;
            }
        }
    }

    return evaluation;
}

int pst_bonus(Piece piece, int square, int phase) {
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
        case Piece::WK: {
            int mg = king_pst_mg[square];
            int eg = king_pst_eg[square];

            return (mg * (256 - phase) + eg * phase) / 256;
        }
        case Piece::BK: {
            int mg = king_pst_mg[mirror_square(square)];
            int eg = king_pst_eg[mirror_square(square)];

            return (mg * (256 - phase) + eg * phase) / 256;
        }
        default:
            return 0;
    }
}

// Считает, находится ли игра на стадии эндшпиля
int compute_phase(const Position& position) {
    int phase = TOTAL_PHASE;

    for (int square = 0; square < 64; square++) {
        Piece piece = position.board[square];

        if (piece != Piece::None) {
            phase -= phase_weights[static_cast<int>(piece)];
        }
    }

    // Держим phase в границах от 0 до 24
    if (phase < 0) {
        phase = 0;
    }
    if (phase > TOTAL_PHASE) {
        phase = TOTAL_PHASE;
    }

    phase = (phase * 256 + TOTAL_PHASE / 2) / TOTAL_PHASE;

    return phase;
}
