#include "evaluate.h"

#include <tuner.h>

#include <algorithm>

#include "magic_constants.h"
#include "tunable_params.h"

// Массив материальной ценности фигур
static std::array<int, 7> values = {86, 342, 340, 495, 1000, 0, 0};

constexpr int TOTAL_PHASE = 24;

// PST
std::array<int, 64> pawn_pst_mg = {
    0,  0,  0,  0,  0,  0,  0,  0,  5,  3,   -4, 5,  8, 20, 20, 1, 2, -6, 2,  0,  13, 4,
    12, 1,  1,  -4, -4, 8,  4,  4,  -4, -14, 18, 5,  1, 0,  7,  3, 6, 7,  36, 25, 17, -8,
    -6, 14, 13, 18, 45, 33, 14, -7, -6, -10, 20, 14, 0, 0,  0,  0, 0, 0,  0,  0,
};

std::array<int, 64> pawn_pst_eg = {
    0,  0,  0,  0,  0,  0,  0,  0,  5,  3,   -4, 5,  8, 20, 20, 1, 2, -6, 2,  0,  13, 4,
    12, 1,  1,  -4, -4, 8,  4,  4,  -4, -14, 18, 5,  1, 0,  7,  3, 6, 7,  36, 25, 17, -8,
    -6, 14, 13, 18, 45, 33, 14, -7, -6, -10, 20, 14, 0, 0,  0,  0, 0, 0,  0,  0,
};

std::array<int, 64> knight_pst_mg = {
    -36, -7,  -16, 5,  0,   5,  -7,  -27, -8,   -11, 1,   21,  20,  12,  6,   6,
    -12, 1,   14,  22, 29,  19, 19,  -9,  1,    9,   21,  22,  31,  27,  13,  -1,
    8,   14,  17,  49, 27,  31, 7,   9,   -14,  1,   25,  33,  30,  20,  7,   -17,
    -46, -22, 6,   8,  -14, 11, -16, -24, -100, -44, -30, -25, -16, -64, -49, -100,
};

std::array<int, 64> knight_pst_eg = {
    -36, -7,  -16, 5,  0,   5,  -7,  -27, -8,   -11, 1,   21,  20,  12,  6,   6,
    -12, 1,   14,  22, 29,  19, 19,  -9,  1,    9,   21,  22,  31,  27,  13,  -1,
    8,   14,  17,  49, 27,  31, 7,   9,   -14,  1,   25,  33,  30,  20,  7,   -17,
    -46, -22, 6,   8,  -14, 11, -16, -24, -100, -44, -30, -25, -16, -64, -49, -100,
};

std::array<int, 64> bishop_pst_mg = {
    -14, -13, -5,  -6,  -9,  -13, -20, -12, -14, 10,  -2,  -1,  7,   7,   25,  -5,
    -2,  7,   8,   4,   8,   9,   -1,  -4,  -17, -8,  1,   10,  15,  -5,  -9,  -23,
    -20, -14, -2,  10,  10,  -10, -7,  -16, -18, -10, -5,  -2,  -4,  8,   2,   -1,
    -36, -17, -22, -23, -16, -15, -14, -30, -25, -39, -26, -28, -33, -33, -28, -34,
};

std::array<int, 64> bishop_pst_eg = {
    -14, -13, -5,  -6,  -9,  -13, -20, -12, -14, 10,  -2,  -1,  7,   7,   25,  -5,
    -2,  7,   8,   4,   8,   9,   -1,  -4,  -17, -8,  1,   10,  15,  -5,  -9,  -23,
    -20, -14, -2,  10,  10,  -10, -7,  -16, -18, -10, -5,  -2,  -4,  8,   2,   -1,
    -36, -17, -22, -23, -16, -15, -14, -30, -25, -39, -26, -28, -33, -33, -28, -34,
};

std::array<int, 64> rook_pst_mg = {
    -4,  -2,  5,   6,  8,   7,  -17, -5,  -19, -16, -10, -7, -7,  -5,  -6,  -30,
    -20, -15, -12, -9, -7,  -8, -13, -16, -13, -13, -13, -8, -12, -11, -10, -18,
    -7,  -4,  -1,  -2, -10, -4, -8,  -4,  1,   2,   -7,  1,  -5,  -5,  -1,  -9,
    0,   3,   6,   7,  -7,  3,  2,   -2,  12,  9,   9,   0,  2,   0,   0,   5,
};

std::array<int, 64> rook_pst_eg = {
    -4,  -2,  5,   6,  8,   7,  -17, -5,  -19, -16, -10, -7, -7,  -5,  -6,  -30,
    -20, -15, -12, -9, -7,  -8, -13, -16, -13, -13, -13, -8, -12, -11, -10, -18,
    -7,  -4,  -1,  -2, -10, -4, -8,  -4,  1,   2,   -7,  1,  -5,  -5,  -1,  -9,
    0,   3,   6,   7,  -7,  3,  2,   -2,  12,  9,   9,   0,  2,   0,   0,   5,
};

std::array<int, 64> queen_pst_mg = {
    30, 28, 34, 42, 35, 21,  4,  21, 25, 31, 35, 42, 45,  39, 38, 29, 11, 28, 32, 34, 35, 43,
    43, 29, 10, 19, 21, 24,  37, 35, 40, 30, 3,  14, 13,  21, 33, 42, 34, 47, 2,  7,  16, 39,
    53, 63, 64, 64, -4, -16, 7,  19, 16, 52, 14, 38, -12, 12, 25, 33, 30, 32, 8,  23,
};

std::array<int, 64> queen_pst_eg = {
    30, 28, 34, 42, 35, 21,  4,  21, 25, 31, 35, 42, 45,  39, 38, 29, 11, 28, 32, 34, 35, 43,
    43, 29, 10, 19, 21, 24,  37, 35, 40, 30, 3,  14, 13,  21, 33, 42, 34, 47, 2,  7,  16, 39,
    53, 63, 64, 64, -4, -16, 7,  19, 16, 52, 14, 38, -12, 12, 25, 33, 30, 32, 8,  23,
};

std::array<int, 64> king_pst_mg = {
    -40, 45, 35,  -48, 8,   -19, 60,  37,  50, 0,   -29, -54, -41, -42, 17,  21,
    23,  17, -48, -68, -73, -55, -15, -27, 2,  51,  19,  -57, -49, -64, -31, -57,
    17,  28, 63,  7,   -34, 6,   52,  -53, 45, 100, 52,  80,  14,  99,  83,  5,
    41,  99, 100, 34,  78,  45,  0,   -47, 99, 96,  97,  99,  61,  98,  79,  7,
};

std::array<int, 64> king_pst_eg = {
    -42, -41, 7,   -15, -28, -24, -28, -77, -41, -11, 7,   18,  13,  15,  -7,  -34,
    -39, -12, 15,  24,  27,  21,  5,   -17, -44, -16, 5,   21,  23,  24,  8,   -11,
    -32, -9,  -1,  10,  17,  14,  7,   -1,  -28, -4,  -1,  -11, 6,   5,   11,  -6,
    -35, -17, -18, -11, -15, 11,  13,  1,   -82, -37, -42, -38, -31, -29, -37, -51,
};

std::array<int, 13> phase_weights = {0, 0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0};

// Mobility bonus (S(mg, eg)) по числу доступных "безопасных" клеток.

std::array<int, 9> knight_mobility_mg = {
    -28, 1, 12, 20, 28, 34, 39, 41, 49,
};
std::array<int, 9> knight_mobility_eg = {
    -99, -78, -57, -54, -52, -45, -48, -44, -66,
};

std::array<int, 14> bishop_mobility_mg = {
    3, 15, 26, 34, 41, 43, 45, 52, 53, 62, 66, 95, 66, 104,
};
std::array<int, 14> bishop_mobility_eg = {
    -51, -44, -35, -25, -15, -4, 5, -1, 7, -6, 0, -19, 5, -26,
};

std::array<int, 15> rook_mobility_mg = {
    -12, -1, -3, -1, -1, 8, 14, 18, 26, 30, 34, 43, 42, 55, 73,
};
std::array<int, 15> rook_mobility_eg = {
    39, 45, 56, 60, 68, 69, 71, 73, 69, 73, 72, 73, 77, 66, 58,
};

std::array<int, 20> queen_mobility_mg = {
    -4, -3, -2, 2, 5, 9, 9, 10, 14, 16, 17, 20, 20, 20, 19, 18, 20, 26, 37, 59,
};
std::array<int, 20> queen_mobility_eg = {
    -81, -48, -50, -28, -20, -8, 13, 33, 32, 53, 56, 68, 76, 80, 98, 95, 100, 99, 100, 100,
};

int bishop_pair_bonus = 47;
int rook_open_file_bonus = 21;
int rook_semi_open_file_bonus = 16;

int doubled_pawn_penalty = 5;
int isolated_pawn_penalty = 14;

int missing_shield_pawn_penalty = 11;

std::array<int, 8> passed_pawn_bonus_mg = {
    0, 0, 0, 1, 4, 1, 6, 0,
};
std::array<int, 8> passed_pawn_bonus_eg = {
    0, 10, 14, 38, 66, 138, 215, 0,
};

static int pst_bonus(Piece piece, int square, int phase) {
    switch (piece) {
        case Piece::WP: {
            int mg = pawn_pst_mg[square];
            int eg = pawn_pst_eg[square];

            return (mg * (256 - phase) + eg * phase) / 256;
        }
        case Piece::BP: {
            int mg = pawn_pst_mg[mirror_square(square)];
            int eg = pawn_pst_eg[mirror_square(square)];

            return (mg * (256 - phase) + eg * phase) / 256;
        }
        case Piece::WN: {
            int mg = knight_pst_mg[square];
            int eg = knight_pst_eg[square];

            return (mg * (256 - phase) + eg * phase) / 256;
        }
        case Piece::BN: {
            int mg = knight_pst_mg[mirror_square(square)];
            int eg = knight_pst_eg[mirror_square(square)];

            return (mg * (256 - phase) + eg * phase) / 256;
        }
        case Piece::WB: {
            int mg = bishop_pst_mg[square];
            int eg = bishop_pst_eg[square];

            return (mg * (256 - phase) + eg * phase) / 256;
        }
        case Piece::BB: {
            int mg = bishop_pst_mg[mirror_square(square)];
            int eg = bishop_pst_eg[mirror_square(square)];

            return (mg * (256 - phase) + eg * phase) / 256;
        }
        case Piece::WR: {
            int mg = rook_pst_mg[square];
            int eg = rook_pst_eg[square];

            return (mg * (256 - phase) + eg * phase) / 256;
        }
        case Piece::BR: {
            int mg = rook_pst_mg[mirror_square(square)];
            int eg = rook_pst_eg[mirror_square(square)];

            return (mg * (256 - phase) + eg * phase) / 256;
        }
        case Piece::WQ: {
            int mg = queen_pst_mg[square];
            int eg = queen_pst_eg[square];

            return (mg * (256 - phase) + eg * phase) / 256;
        }
        case Piece::BQ: {
            int mg = queen_pst_mg[mirror_square(square)];
            int eg = queen_pst_eg[mirror_square(square)];

            return (mg * (256 - phase) + eg * phase) / 256;
        }
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
static int compute_phase(const Position& position) {
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

static int mobility_count(Bitboard attacks, const Position& position, Color color) {
    Bitboard enemy_pawns = position.by_color[static_cast<int>(opposite_color(color))] &
                           position.by_piece_type[static_cast<int>(PieceType::Pawn)];

    Bitboard enemy_pawn_attacks = pawn_attacks_bulk(enemy_pawns, opposite_color(color));

    // Куда фигура могла бы пойти, исключая свои фигуры и клетки, атакованные пешками врага
    Bitboard moves = attacks & ~position.by_color[static_cast<int>(color)] & ~enemy_pawn_attacks;

    return popcount(moves);
}

static int knight_mobility(const Position& position, int square, Color color) {
    return mobility_count(knight_attacks[square], position, color);
}

static int bishop_mobility(const Position& position, int square, Color color) {
    Bitboard occupancy = position.by_color[0] | position.by_color[1];
    Bitboard attacks = bishop_attacks_from(square, occupancy);

    return mobility_count(attacks, position, color);
}

static int rook_mobility(const Position& position, int square, Color color) {
    Bitboard occupancy = position.by_color[0] | position.by_color[1];
    Bitboard attacks = rook_attacks_from(square, occupancy);

    return mobility_count(attacks, position, color);
}

static int queen_mobility(const Position& position, int square, Color color) {
    Bitboard occupancy = position.by_color[0] | position.by_color[1];

    Bitboard attacks =
        rook_attacks_from(square, occupancy) | bishop_attacks_from(square, occupancy);

    return mobility_count(attacks, position, color);
}

static int mobility_bonus(Piece piece, int square, const Position& position, int phase) {
    PieceType type = piece_type_of(piece);
    Color color = color_of(piece);

    switch (type) {
        case PieceType::Knight: {
            int mobility = knight_mobility(position, square, color);
            int idx = std::min(mobility, static_cast<int>(knight_mobility_mg.size()) - 1);
            return (knight_mobility_mg[idx] * (256 - phase) + knight_mobility_eg[idx] * phase) /
                   256;
        }
        case PieceType::Bishop: {
            int mobility = bishop_mobility(position, square, color);
            int idx = std::min(mobility, static_cast<int>(bishop_mobility_mg.size()) - 1);
            return (bishop_mobility_mg[idx] * (256 - phase) + bishop_mobility_eg[idx] * phase) /
                   256;
        }
        case PieceType::Rook: {
            int mobility = rook_mobility(position, square, color);
            int idx = std::min(mobility, static_cast<int>(rook_mobility_mg.size()) - 1);
            return (rook_mobility_mg[idx] * (256 - phase) + rook_mobility_eg[idx] * phase) / 256;
        }
        case PieceType::Queen: {
            int mobility = queen_mobility(position, square, color);
            int idx = std::min(mobility, static_cast<int>(queen_mobility_mg.size()) - 1);
            return (queen_mobility_mg[idx] * (256 - phase) + queen_mobility_eg[idx] * phase) / 256;
        }
        default:
            return 0;
    }
}

static int evaluate_side(const Position& position, Color color, int phase) {
    int bonus = 0;

    // Бонус за пару слонов
    Bitboard bishops = position.by_color[static_cast<int>(color)] &
                       position.by_piece_type[static_cast<int>(PieceType::Bishop)];
    if (popcount(bishops) >= 2) {
        bonus += bishop_pair_bonus;
    }

    Bitboard all_pawns = position.by_piece_type[static_cast<int>(PieceType::Pawn)];
    Bitboard own_pawns = all_pawns & position.by_color[static_cast<int>(color)];
    Bitboard enemy_pawns = all_pawns & position.by_color[static_cast<int>(opposite_color(color))];

    // Бонус за ладьи на открытых/полуоткрытых файлах
    Bitboard rooks = position.by_color[static_cast<int>(color)] &
                     position.by_piece_type[static_cast<int>(PieceType::Rook)];

    while (rooks != 0) {
        int square = pop_lsb(rooks);
        Bitboard file_mask = FILE_A << file_of(square);

        if ((own_pawns & file_mask) == 0) {
            bonus +=
                (enemy_pawns & file_mask) == 0 ? rook_open_file_bonus : rook_semi_open_file_bonus;
        }
    }

    // Штраф за сдвоенные пешки
    Bitboard side_pawns = own_pawns;
    Bitboard temp_pawns = side_pawns;
    while (temp_pawns != 0) {
        int square = pop_lsb(temp_pawns);
        Bitboard file_mask = FILE_A << file_of(square);
        if (popcount(side_pawns & file_mask) > 1) {
            bonus -= doubled_pawn_penalty;
        }
    }

    // Штраф за изолированные пешки
    Bitboard temp_pawns2 = own_pawns;
    while (temp_pawns2 != 0) {
        int square = pop_lsb(temp_pawns2);
        int file = file_of(square);

        Bitboard adjacent_files = 0;
        if (file > 0) {
            adjacent_files |= FILE_A << (file - 1);
        }
        if (file < 7) {
            adjacent_files |= FILE_A << (file + 1);
        }

        if ((own_pawns & adjacent_files) == 0) {
            bonus -= isolated_pawn_penalty;
        }
    }

    // Бонус за проходные пешки
    Bitboard temp_pawns3 = own_pawns;
    while (temp_pawns3 != 0) {
        int square = pop_lsb(temp_pawns3);
        int file = file_of(square);
        int rank = rank_of(square);

        Bitboard files_mask = FILE_A << file;
        if (file > 0) {
            files_mask |= FILE_A << (file - 1);
        }
        if (file < 7) {
            files_mask |= FILE_A << (file + 1);
        }

        Bitboard ranks_ahead;
        int table_index;
        if (color == Color::White) {
            ranks_ahead = ~0ULL << (8 * (rank + 1));
            table_index = rank;
        } else {
            ranks_ahead = (1ULL << (8 * rank)) - 1;
            table_index = 7 - rank;
        }

        bool blocked_by_own = (own_pawns & (FILE_A << file) & ranks_ahead) != 0;
        bool is_passed = (enemy_pawns & files_mask & ranks_ahead) == 0;

        if (is_passed && !blocked_by_own) {
            bonus += (passed_pawn_bonus_mg[table_index] * (256 - phase) +
                      passed_pawn_bonus_eg[table_index] * phase) /
                     256;
        }
    }

    // Штраф за отсутствие пешечного щита на позиции рокированного короля
    int king_square =
        (color == Color::White) ? position.white_king_square : position.black_king_square;

    bool castled_kingside =
        king_square == (color == Color::White ? square_of(0, 6) : square_of(7, 6));

    bool castled_queenside =
        king_square == (color == Color::White ? square_of(0, 2) : square_of(7, 2));

    if (castled_kingside || castled_queenside) {
        int king_file = file_of(king_square);
        int king_rank = rank_of(king_square);
        int shield_rank = (color == Color::White) ? king_rank + 1 : king_rank - 1;

        Bitboard shield_files =
            (FILE_A << (king_file - 1)) | (FILE_A << king_file) | (FILE_A << (king_file + 1));
        Bitboard shield_rank_mask = 0xFFULL << (8 * shield_rank);
        Bitboard shield_mask = shield_files & shield_rank_mask;

        int shield_pawns_present = popcount(own_pawns & shield_mask);
        bonus -= (3 - shield_pawns_present) * missing_shield_pawn_penalty;
    }

    return bonus;
}

int evaluate(const Position& position) {
    int evaluation = 0;
    int phase = compute_phase(position);

    int material_pst =
        (position.material_pst_score_mg * (256 - phase) + position.material_pst_score_eg * phase) /
        256;
    evaluation += (position.side_to_move == Color::White) ? material_pst : -material_pst;

    for (Piece piece : {Piece::WP, Piece::WN, Piece::WB, Piece::WR, Piece::WQ, Piece::WK, Piece::BP,
                        Piece::BN, Piece::BB, Piece::BR, Piece::BQ, Piece::BK}) {
        Bitboard pieces = position.by_color[static_cast<int>(color_of(piece))] &
                          position.by_piece_type[static_cast<int>(piece_type_of(piece))];

        while (pieces != 0) {
            int square = pop_lsb(pieces);

            // Бонус за мобильность фигур
            int value = mobility_bonus(piece, square, position, phase);

            if (color_of(piece) == position.side_to_move) {
                evaluation += value;
            } else {
                evaluation -= value;
            }
        }
    }

    evaluation += evaluate_side(position, position.side_to_move, phase);
    evaluation -= evaluate_side(position, opposite_color(position.side_to_move), phase);

    return evaluation;
}

void register_eval_tunable() {
    register_tunable("pawn_value", &values[0], 0, 200);
    register_tunable("knight_value", &values[1], 0, 400);
    register_tunable("bishop_value", &values[2], 0, 400);
    register_tunable("rook_value", &values[3], 0, 600);
    register_tunable("queen_value", &values[4], 0, 1000);

    register_tunable_array("pawn_pst_mg", pawn_pst_mg.data(), pawn_pst_mg.size(), -100, 100);
    register_tunable_array("pawn_pst_eg", pawn_pst_eg.data(), pawn_pst_eg.size(), -100, 100);

    register_tunable_array("knight_pst_mg", knight_pst_mg.data(), knight_pst_mg.size(), -100, 100);
    register_tunable_array("knight_pst_eg", knight_pst_eg.data(), knight_pst_eg.size(), -100, 100);

    register_tunable_array("bishop_pst_mg", bishop_pst_mg.data(), bishop_pst_mg.size(), -100, 100);
    register_tunable_array("bishop_pst_eg", bishop_pst_eg.data(), bishop_pst_eg.size(), -100, 100);

    register_tunable_array("rook_pst_mg", rook_pst_mg.data(), rook_pst_mg.size(), -100, 100);
    register_tunable_array("rook_pst_eg", rook_pst_eg.data(), rook_pst_eg.size(), -100, 100);

    register_tunable_array("queen_pst_mg", queen_pst_mg.data(), queen_pst_mg.size(), -100, 100);
    register_tunable_array("queen_pst_eg", queen_pst_eg.data(), queen_pst_eg.size(), -100, 100);

    register_tunable_array("king_pst_mg", king_pst_mg.data(), king_pst_mg.size(), -100, 100);
    register_tunable_array("king_pst_eg", king_pst_eg.data(), king_pst_eg.size(), -100, 100);

    register_tunable_array("knight_mobility_mg", knight_mobility_mg.data(),
                           knight_mobility_mg.size(), -100, 100);
    register_tunable_array("knight_mobility_eg", knight_mobility_eg.data(),
                           knight_mobility_eg.size(), -100, 100);
    register_tunable_array("bishop_mobility_mg", bishop_mobility_mg.data(),
                           bishop_mobility_mg.size(), -150, 150);
    register_tunable_array("bishop_mobility_eg", bishop_mobility_eg.data(),
                           bishop_mobility_eg.size(), -150, 150);
    register_tunable_array("rook_mobility_mg", rook_mobility_mg.data(), rook_mobility_mg.size(),
                           -200, 200);
    register_tunable_array("rook_mobility_eg", rook_mobility_eg.data(), rook_mobility_eg.size(),
                           -200, 200);
    register_tunable_array("queen_mobility_mg", queen_mobility_mg.data(), queen_mobility_mg.size(),
                           -100, 100);
    register_tunable_array("queen_mobility_eg", queen_mobility_eg.data(), queen_mobility_eg.size(),
                           -100, 100);

    register_tunable_array("passed_pawn_bonus_mg", passed_pawn_bonus_mg.data(),
                           passed_pawn_bonus_mg.size(), 0, 200);
    register_tunable_array("passed_pawn_bonus_eg", passed_pawn_bonus_eg.data(),
                           passed_pawn_bonus_eg.size(), 0, 300);

    register_tunable("bishop_pair_bonus", &bishop_pair_bonus, 0, 100);

    register_tunable("rook_open_file_bonus", &rook_open_file_bonus, 0, 100);
    register_tunable("rook_semi_open_file_bonus", &rook_semi_open_file_bonus, 0, 100);

    register_tunable("doubled_pawn_penalty", &doubled_pawn_penalty, 0, 100);
    register_tunable("isolated_pawn_penalty", &isolated_pawn_penalty, 0, 100);

    register_tunable("missing_shield_pawn_penalty", &missing_shield_pawn_penalty, 0, 100);
}

void print_tuned_params(std::ostream& out) {
    print_param(out, "pawn_value", values[0]);
    print_param(out, "knight_value", values[1]);
    print_param(out, "bishop_value", values[2]);
    print_param(out, "rook_value", values[3]);
    print_param(out, "queen_value", values[4]);

    print_param_array(out, "pawn_pst_mg", pawn_pst_mg.data(), pawn_pst_mg.size());
    print_param_array(out, "pawn_pst_eg", pawn_pst_eg.data(), pawn_pst_eg.size());

    print_param_array(out, "knight_pst_mg", knight_pst_mg.data(), knight_pst_mg.size());
    print_param_array(out, "knight_pst_eg", knight_pst_eg.data(), knight_pst_eg.size());

    print_param_array(out, "bishop_pst_mg", bishop_pst_mg.data(), bishop_pst_mg.size());
    print_param_array(out, "bishop_pst_eg", bishop_pst_eg.data(), bishop_pst_eg.size());

    print_param_array(out, "rook_pst_mg", rook_pst_mg.data(), rook_pst_mg.size());
    print_param_array(out, "rook_pst_eg", rook_pst_eg.data(), rook_pst_eg.size());

    print_param_array(out, "queen_pst_mg", queen_pst_mg.data(), queen_pst_mg.size());
    print_param_array(out, "queen_pst_eg", queen_pst_eg.data(), queen_pst_eg.size());

    print_param_array(out, "king_pst_mg", king_pst_mg.data(), king_pst_mg.size());
    print_param_array(out, "king_pst_eg", king_pst_eg.data(), king_pst_eg.size());

    print_param_array(out, "knight_mobility_mg", knight_mobility_mg.data(),
                      knight_mobility_mg.size());
    print_param_array(out, "knight_mobility_eg", knight_mobility_eg.data(),
                      knight_mobility_eg.size());
    print_param_array(out, "bishop_mobility_mg", bishop_mobility_mg.data(),
                      bishop_mobility_mg.size());
    print_param_array(out, "bishop_mobility_eg", bishop_mobility_eg.data(),
                      bishop_mobility_eg.size());
    print_param_array(out, "rook_mobility_mg", rook_mobility_mg.data(), rook_mobility_mg.size());
    print_param_array(out, "rook_mobility_eg", rook_mobility_eg.data(), rook_mobility_eg.size());
    print_param_array(out, "queen_mobility_mg", queen_mobility_mg.data(), queen_mobility_mg.size());
    print_param_array(out, "queen_mobility_eg", queen_mobility_eg.data(), queen_mobility_eg.size());

    print_param_array(out, "passed_pawn_bonus_mg", passed_pawn_bonus_mg.data(),
                      passed_pawn_bonus_mg.size());
    print_param_array(out, "passed_pawn_bonus_eg", passed_pawn_bonus_eg.data(),
                      passed_pawn_bonus_eg.size());

    print_param(out, "bishop_pair_bonus", bishop_pair_bonus);

    print_param(out, "rook_open_file_bonus", rook_open_file_bonus);
    print_param(out, "rook_semi_open_file_bonus", rook_semi_open_file_bonus);

    print_param(out, "doubled_pawn_penalty", doubled_pawn_penalty);
    print_param(out, "isolated_pawn_penalty", isolated_pawn_penalty);

    print_param(out, "missing_shield_pawn_penalty", missing_shield_pawn_penalty);
}

int material_pst_value_mg(Piece piece, int square) {
    return values[static_cast<int>(piece_type_of(piece))] + pst_bonus(piece, square, 0);
}

int material_pst_value_eg(Piece piece, int square) {
    return values[static_cast<int>(piece_type_of(piece))] + pst_bonus(piece, square, 256);
}

void compute_material_pst(Position& position) {
    position.material_pst_score_mg = 0;
    position.material_pst_score_eg = 0;

    for (Piece piece : {Piece::WP, Piece::WN, Piece::WB, Piece::WR, Piece::WQ, Piece::BP, Piece::BN,
                        Piece::BB, Piece::BR, Piece::BQ, Piece::WK, Piece::BK}) {
        Bitboard pieces = position.by_color[static_cast<int>(color_of(piece))] &
                          position.by_piece_type[static_cast<int>(piece_type_of(piece))];

        while (pieces != 0) {
            int square = pop_lsb(pieces);
            int sign = (color_of(piece) == Color::White) ? 1 : -1;

            position.material_pst_score_mg += sign * material_pst_value_mg(piece, square);
            position.material_pst_score_eg += sign * material_pst_value_eg(piece, square);
        }
    }
}
