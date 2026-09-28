#include "evaluate.h"

#include <tuner.h>

#include <algorithm>

#include "magic_constants.h"
#include "tunable_params.h"

// Массив материальной ценности фигур
std::array<int, 7> values = {87, 335, 355, 534, 935, 0, 0};

// PST
std::array<int, 64> pawn_pst = {
    0,  0,  0,  0,  0,  0,  0,  0,  1,  0,   -7, -16, -2, 20, 18, 0, 0,  -6, 2,  -5, 8,  1,
    12, 1,  -1, -1, -4, 9,  6,  2,  -3, -14, 20, 10,  6,  5,  14, 9, 10, 9,  40, 30, 23, -5,
    1,  26, 20, 26, 58, 24, 15, 15, 15, 15,  15, 31,  0,  0,  0,  0, 0,  0,  0,  0,
};

std::array<int, 64> knight_pst = {
    -61, -23, -37, -18, -25, -10, -24, -60, -24, -26, -7,  6,   7,   6,   -9,  -6,
    -19, -1,  19,  18,  22,  17,  17,  -24, -11, 0,   23,  20,  30,  22,  4,   -15,
    -3,  16,  18,  49,  29,  32,  9,   0,   -25, -2,  22,  34,  28,  37,  10,  -24,
    -59, -24, 25,  -1,  11,  14,  -23, -33, -85, -66, -38, -30, -29, -65, -60, -85,
};

std::array<int, 64> bishop_pst = {
    -21, -21, -16, -13, -19, -24, -27, -21, -17, 8,   0,   -4,  1,   8,   23,  -7,
    -1,  9,   9,   10,  11,  11,  2,   -2,  -15, -1,  8,   15,  22,  4,   0,   -27,
    -11, -5,  9,   19,  17,  3,   -2,  -12, -12, 1,   18,  13,  15,  19,  11,  8,
    -35, -4,  -11, -14, 1,   8,   -5,  -9,  -25, -34, -24, -21, -25, -24, -19, -33,
};

std::array<int, 64> rook_pst = {
    1,  0,  4,  6,  5,  8,  -18, -8, -14, -7, -6, -5, -6, 1,  2,  -25, -12, -7, -7, -4, -4, -3,
    -5, -9, -3, -2, -3, 0,  -2,  0,  0,   -8, 7,  5,  11, 7,  2,  9,   9,   11, 14, 13, 8,  10,
    8,  14, 13, 10, 17, 18, 19,  18, 12,  22, 19, 15, 27, 22, 20, 15,  17,  17, 15, 18,
};

std::array<int, 64> queen_pst = {
    15,  -5,  3,  15, 2,  -6, -19, -6, -4,  15,  17,  18,  22, 20, 17,  6,
    -10, 11,  17, 17, 18, 22, 26,  6,  -6,  12,  10,  17,  30, 21, 31,  9,
    -14, 5,   13, 20, 34, 40, 21,  30, -14, 0,   27,  35,  40, 40, 35,  25,
    -18, -15, 6,  22, 19, 35, 35,  25, -55, -45, -10, -16, 1,  -7, -45, -32,
};

std::array<int, 64> king_pst_mg = {
    -3,  38,  36,  -35, 11,  -9,  51,  20,  23,  23,  -9,  -35, -35, -13, 35,  20,
    -10, -12, -33, -40, -40, -27, -9,  -3,  -23, -25, -45, -61, -54, -45, -24, -17,
    -31, -33, -48, -69, -70, -42, -20, -10, -25, -5,  -37, -40, -64, -34, -5,  -3,
    -23, -14, -35, -48, -46, -15, -19, -8,  -29, -16, -26, -46, -42, -24, -9,  -19,
};

std::array<int, 64> king_pst_eg = {
    -57, -33, -4, -26, -25, -34, -20, -52, -36, -26, -3,  7,   10,  8,   -13, -31,
    -38, -11, 7,  14,  16,  13,  3,   -21, -45, -10, 9,   17,  21,  19,  -1,  -23,
    -32, -3,  11, 18,  17,  20,  13,  -8,  -22, 9,   10,  7,   11,  22,  23,  -2,
    -25, 4,   -5, -1,  1,   15,  4,   -7,  -52, -23, -25, -22, -18, -15, -17, -42,
};

std::array<int, 13> phase_weights = {0, 0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0};

// Mobility bonus (S(mg, eg)) по числу доступных "безопасных" клеток.

std::array<int, 9> knight_mobility_mg = {
    -30, -20, -10, 0, 10, 18, 24, 28, 30,
};
std::array<int, 9> knight_mobility_eg = {
    -30, -20, -10, 0, 10, 18, 24, 28, 30,
};

std::array<int, 14> bishop_mobility_mg = {
    -20, -10, 5, 20, 35, 48, 58, 65, 70, 73, 75, 76, 77, 77,
};
std::array<int, 14> bishop_mobility_eg = {
    -25, -15, 0, 15, 30, 42, 52, 58, 62, 65, 67, 68, 68, 69,
};

std::array<int, 15> rook_mobility_mg = {
    -20, -12, -5, 0, 5, 10, 14, 18, 21, 23, 25, 26, 27, 28, 28,
};
std::array<int, 15> rook_mobility_eg = {
    -35, -18, -5, 12, 28, 44, 58, 72, 85, 95, 103, 108, 112, 115, 117,
};

std::array<int, 20> queen_mobility_mg = {
    -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 15, 16, 17, 18, 19, 20, 20,
};
std::array<int, 20> queen_mobility_eg = {
    -18, -14, -10, -6, -2, 2, 6, 10, 14, 18, 21, 24, 27, 29, 31, 32, 33, 34, 35, 35,
};

int bishop_pair_bonus = 46;
int rook_open_file_bonus = 32;
int rook_semi_open_file_bonus = 24;

int doubled_pawn_penalty = 5;
int isolated_pawn_penalty = 12;

int missing_shield_pawn_penalty = 9;

std::array<int, 8> passed_pawn_bonus_mg = {
    0, 0, 0, 7, 12, 39, 65, 0,
};
std::array<int, 8> passed_pawn_bonus_eg = {
    0, 11, 16, 30, 55, 108, 152, 0,
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
