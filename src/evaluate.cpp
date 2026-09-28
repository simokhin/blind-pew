#include "evaluate.h"

#include <tuner.h>

#include <algorithm>

#include "magic_constants.h"
#include "tunable_params.h"

// Массив материальной ценности фигур
std::array<int, 7> values = {94, 342, 344, 523, 946, 0, 0};

// PST
std::array<int, 64> pawn_pst = {
    0,  0,  0,  0,  0,  0,  0,  0,  5,  3,   -8, -5, 6, 20, 22, 1, 2, -6, -2, -4, 13, 2,
    12, 1,  -1, -4, -6, 4,  2,  4,  -4, -16, 20, 9,  1, -2, 7,  5, 8, 9,  46, 35, 23, -8,
    2,  24, 29, 28, 69, 35, 26, 19, 20, 26,  26, 42, 0, 0,  0,  0, 0, 0,  0,  0,
};

std::array<int, 64> knight_pst = {
    -50, -13, -26, -7, -14, -1, -13, -49, -14, -19, -7,  17,  14,  6,   -2,  4,
    -20, -7,  10,  16, 23,  15, 15,  -17, -7,  1,   17,  18,  29,  23,  9,   -9,
    2,   10,  11,  49, 25,  23, 1,   5,   -20, -5,  21,  25,  24,  26,  3,   -23,
    -56, -24, 14,  0,  0,   11, -18, -30, -96, -58, -44, -29, -32, -74, -61, -96,
};

std::array<int, 64> bishop_pst = {
    -14, -15, -5,  -6,  -9,  -13, -26, -14, -18, 10,  -6,  -5,  5,   5,   27,  -5,
    -6,  5,   2,   0,   4,   5,   -5,  -6,  -19, -12, -3,  4,   11,  -7,  -11, -25,
    -22, -16, -2,  8,   6,   -8,  -13, -18, -22, -10, 7,   2,   4,   8,   0,   -3,
    -42, -15, -22, -25, -10, -3,  -16, -20, -31, -45, -30, -28, -35, -35, -30, -42,
};

std::array<int, 64> rook_pst = {
    -2,  -2,  5,   4,   8,  11, -19, -5,  -19, -16, -12, -9, -9,  -5,  -6, -32,
    -18, -15, -14, -11, -9, -8, -15, -14, -9,  -11, -11, -8, -12, -11, -8, -16,
    -1,  -2,  1,   -2,  -8, -2, -2,  4,   3,   4,   -3,  -1, -3,  3,   3,  -1,
    6,   7,   8,   7,   1,  11, 8,   4,   16,  11,  9,   4,  6,   6,   4,  7,
};

std::array<int, 64> queen_pst = {
    22, 6,  14, 26, 13,  5,   -8, 5,  7,  21, 23, 28, 33,  29,  24, 15, -3, 14, 20,  22,  25, 27,
    33, 15, -6, 9,  13,  12,  33, 27, 34, 18, -7, 4,  11,  15,  31, 42, 28, 39, -8,  5,   18, 43,
    51, 51, 46, 36, -18, -26, 7,  25, 18, 46, 24, 34, -44, -34, 1,  -5, 12, 4,  -34, -21,
};

std::array<int, 64> king_pst_mg = {
    -8,  39,  41,  -46, 8,   -7,  60,  27, 32,  22,  -17, -46, -43, -24, 25,  19,
    -11, -11, -40, -40, -49, -27, -9,  -3, -24, -23, -45, -61, -53, -44, -21, -17,
    -31, -28, -37, -69, -70, -40, -12, -9, -23, 6,   -32, -40, -56, -29, 5,   -1,
    -17, -5,  -28, -46, -42, -11, -12, -7, -29, -16, -19, -35, -39, -18, -5,  -9,
};

std::array<int, 64> king_pst_eg = {
    -62, -33, 3,  -23, -24, -32, -24, -61, -45, -27, -5,  10,  9,   9,  -9,  -32,
    -43, -12, 5,  14,  19,  13,  3,   -21, -46, -8,  9,   17,  23,  20, 2,   -23,
    -32, -1,  17, 18,  17,  22,  19,  -7,  -20, 16,  15,  7,   18,  27, 31,  0,
    -21, 7,   2,  1,   5,   19,  11,  -3,  -52, -23, -18, -12, -15, -9, -13, -31,
};

std::array<int, 13> phase_weights = {0, 0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0};

// Mobility bonus (S(mg, eg)) по числу доступных "безопасных" клеток.

std::array<int, 9> knight_mobility_mg = {
    -40, -11, 0, 8, 16, 24, 23, 25, 25,
};
std::array<int, 9> knight_mobility_eg = {
    -39, -18, -1, 6, 8, 13, 24, 26, 26,
};

std::array<int, 14> bishop_mobility_mg = {
    -9, 1, 16, 28, 35, 39, 47, 54, 59, 62, 64, 67, 66, 68,
};
std::array<int, 14> bishop_mobility_eg = {
    -15, -4, 11, 17, 31, 36, 43, 47, 51, 54, 56, 57, 57, 58,
};

std::array<int, 15> rook_mobility_mg = {
    -12, -1, -3, -1, -1, 4, 10, 14, 12, 14, 14, 15, 18, 17, 17,
};
std::array<int, 15> rook_mobility_eg = {
    -25, -7, 4, 22, 32, 51, 57, 73, 79, 85, 92, 97, 101, 104, 106,
};

std::array<int, 20> queen_mobility_mg = {
    0, -1, -2, 2, 5, 7, 9, 10, 14, 16, 17, 20, 20, 20, 27, 26, 28, 30, 31, 31,
};
std::array<int, 20> queen_mobility_eg = {
    -7, -6, -12, -4, 2, 10, 13, 21, 24, 29, 32, 32, 38, 40, 42, 43, 44, 45, 46, 46,
};

int bishop_pair_bonus = 43;
int rook_open_file_bonus = 21;
int rook_semi_open_file_bonus = 18;

int doubled_pawn_penalty = 7;
int isolated_pawn_penalty = 12;

int missing_shield_pawn_penalty = 11;

std::array<int, 8> passed_pawn_bonus_mg = {
    0, 0, 0, 11, 16, 47, 70, 0,
};
std::array<int, 8> passed_pawn_bonus_eg = {
    0, 8, 14, 34, 62, 116, 163, 0,
};

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
            int value = values[static_cast<int>(piece_type_of(piece))];

            // Бонус за расположение фигур
            value += pst_bonus(piece, square, phase);

            // Бонус за мобильность фигур
            value += mobility_bonus(piece, square, position, phase);

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

int mobility_count(Bitboard attacks, const Position& position, Color color) {
    Bitboard enemy_pawns = position.by_color[static_cast<int>(opposite_color(color))] &
                           position.by_piece_type[static_cast<int>(PieceType::Pawn)];

    Bitboard enemy_pawn_attacks = pawn_attacks_bulk(enemy_pawns, opposite_color(color));

    // Куда фигура могла бы пойти, исключая свои фигуры и клетки, атакованные пешками врага
    Bitboard moves = attacks & ~position.by_color[static_cast<int>(color)] & ~enemy_pawn_attacks;

    return popcount(moves);
}

int mobility_bonus(Piece piece, int square, const Position& position, int phase) {
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

int knight_mobility(const Position& position, int square, Color color) {
    return mobility_count(knight_attacks[square], position, color);
}

int bishop_mobility(const Position& position, int square, Color color) {
    Bitboard occupancy = position.by_color[0] | position.by_color[1];
    Bitboard attacks = bishop_attacks_from(square, occupancy);

    return mobility_count(attacks, position, color);
}

int rook_mobility(const Position& position, int square, Color color) {
    Bitboard occupancy = position.by_color[0] | position.by_color[1];
    Bitboard attacks = rook_attacks_from(square, occupancy);

    return mobility_count(attacks, position, color);
}

int queen_mobility(const Position& position, int square, Color color) {
    Bitboard occupancy = position.by_color[0] | position.by_color[1];

    Bitboard attacks =
        rook_attacks_from(square, occupancy) | bishop_attacks_from(square, occupancy);

    return mobility_count(attacks, position, color);
}

int evaluate_side(const Position& position, Color color, int phase) {
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

void register_eval_tunable() {
    register_tunable("pawn_value", &values[0], 0, 200);
    register_tunable("knight_value", &values[1], 0, 400);
    register_tunable("bishop_value", &values[2], 0, 400);
    register_tunable("rook_value", &values[3], 0, 600);
    register_tunable("queen_value", &values[4], 0, 1000);

    register_tunable_array("pawn_pst", pawn_pst.data(), pawn_pst.size(), -100, 100);
    register_tunable_array("knight_pst", knight_pst.data(), knight_pst.size(), -100, 100);
    register_tunable_array("bishop_pst", bishop_pst.data(), bishop_pst.size(), -100, 100);
    register_tunable_array("rook_pst", rook_pst.data(), rook_pst.size(), -100, 100);
    register_tunable_array("queen_pst", queen_pst.data(), queen_pst.size(), -100, 100);
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

    print_param_array(out, "pawn_pst", pawn_pst.data(), pawn_pst.size());
    print_param_array(out, "knight_pst", knight_pst.data(), knight_pst.size());
    print_param_array(out, "bishop_pst", bishop_pst.data(), bishop_pst.size());
    print_param_array(out, "rook_pst", rook_pst.data(), rook_pst.size());
    print_param_array(out, "queen_pst", queen_pst.data(), queen_pst.size());
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
