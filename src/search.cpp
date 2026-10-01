#include "search.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <utility>

#include "board.h"
#include "constants.h"
#include "evaluate.h"
#include "movegen.h"
#include "tt.h"

// Инициализация таблицы для LMR
static int lmr_table[64][64];

void init_lmr_table() {
    for (int depth = 1; depth < 64; depth++) {
        for (int move_index = 1; move_index < 64; move_index++) {
            // Формула: base + ln(depth)*ln(move_index)/scale.
            lmr_table[depth][move_index] =
                static_cast<int>(0.7844 + std::log(depth) * std::log(move_index) / 2.4696);
        }
    }
}

// Значения, используюищиеся в сортировке ходов
constexpr std::array<int, 6> see_values = {100, 320, 330, 500, 900, INFINITE};
constexpr std::array<int, 4> promotion_values = {320, 330, 500, 900};
constexpr std::array<int, 13> mvv_lva_values = {0,   100, 320, 330, 500, 900, 0,
                                                100, 320, 330, 500, 900, 0};

namespace {
// Структура хода с оценокой, нужной для сортировки
struct ScoredMove {
    Move move;
    int score;
};

// Список ходов для сортировки
class ScoredMoveList {
   public:
    void add(const Move& move, int score);
    int size() const;

    ScoredMove& operator[](int index);

   private:
    std::array<ScoredMove, MAX_MOVES> scored_moves;
    int count = 0;
};

void ScoredMoveList::add(const Move& move, int score) {
    scored_moves[count] = {move, score};
    count++;
}
int ScoredMoveList::size() const { return count; }

ScoredMove& ScoredMoveList::operator[](int index) { return scored_moves[index]; }

}  // namespace

static ScoredMoveList sort_moves(const MoveList& moves, const Position& position,
                                 const SearchState& state, int ply, bool have_tt_move,
                                 const Move& tt_move);

static bool is_in_check(const Position& position) {
    return is_square_attacked_bb(position, king_square_of(position, position.side_to_move),
                                 opposite_color(position.side_to_move));
}

static bool has_non_pawn_material(const Position& position, Color color) {
    Bitboard pawns_and_king = position.by_piece_type[static_cast<int>(PieceType::Pawn)] |
                              position.by_piece_type[static_cast<int>(PieceType::King)];

    return (position.by_color[static_cast<int>(color)] & ~pawns_and_king) != 0;
}

// Оценивает ход и использует это для сортировки по принципу MVV LVA
static int mvv_lva_score(const Position& position, const Move& m) {
    int victim = static_cast<int>(position.board[m.to()]);
    int attacker = static_cast<int>(position.board[m.from()]);

    if (m.flag() == MoveFlag::EnPassant) {
        return mvv_lva_values[static_cast<int>(Piece::WP)] * 10 - mvv_lva_values[attacker];
    }

    return mvv_lva_values[victim] * 10 - mvv_lva_values[attacker];
}

// Пробует сделать ход и если он был легальный, возвращает true
static bool make_legal_move(Position& position, const Move& m, UndoInfo& undo) {
    // Определяем, чей ход был до хода
    Color mover = position.side_to_move;

    // Делаем ход
    undo = make_move(position, m);

    // Сохраняем позицию короля после того, как ход сделан
    int king_square;

    if (mover == Color::White) {
        king_square = position.white_king_square;
    } else {
        king_square = position.black_king_square;
    }

    // Определяем, является ли ход легальным и если нет - отменяем его
    if (is_square_attacked_bb(position, king_square, position.side_to_move)) {
        unmake_move(position, m, undo);
        return false;
    }

    return true;
}

static int see_capture(const Position& position, const Move& move);

static void print_search_info(int depth, const SearchState& state, int best_score,
                              double elapsed_seconds);

static void update_history_heuristic(SearchState& state, Color side, int from, int to, int bonus);

static bool is_repetition(const Position& position, const SearchState& state) {
    return std::count(state.history.begin(), state.history.end(), position.zobrist_hash) >= 2;
}

static bool move_gives_check(const Position& position, const Move& m) {
    Color us = position.side_to_move;
    Color them = opposite_color(position.side_to_move);
    int king_square = king_square_of(position, them);

    int from = m.from();
    int to = m.to();
    Piece piece = position.board[from];

    Bitboard occ_after =
        ((position.by_color[0] | position.by_color[1]) & ~square_bb(from)) | square_bb(to);

    if (m.flag() != MoveFlag::Normal) {
        return true;
    }

    PieceType type = piece_type_of(piece);

    // Конь даёт шах королю
    if (type == PieceType::Knight && (knight_attacks[to] & square_bb(king_square)) != 0) {
        return true;
    }

    // Пешка даёт шах королю
    if (type == PieceType::Pawn &&
        (pawn_attacks[static_cast<int>(us)][to] & square_bb(king_square)) != 0) {
        return true;
    }

    // Слон даёт шах королю
    if (type == PieceType::Bishop &&
        (bishop_attacks_from(to, occ_after) & square_bb(king_square)) != 0) {
        return true;
    }

    // Ладья даёт шах королю
    if (type == PieceType::Rook &&
        (rook_attacks_from(to, occ_after) & square_bb(king_square)) != 0) {
        return true;
    }

    // Ферзь даёт шах королю
    if (type == PieceType::Queen &&
        ((rook_attacks_from(to, occ_after) | bishop_attacks_from(to, occ_after)) &
         square_bb(king_square)) != 0) {
        return true;
    }

    Bitboard rooks_and_queen = (position.by_piece_type[static_cast<int>(PieceType::Rook)] |
                                position.by_piece_type[static_cast<int>(PieceType::Queen)]) &
                               position.by_color[static_cast<int>(us)] & ~square_bb(from);

    if ((rook_attacks_from(king_square, occ_after) & rooks_and_queen) != 0) {
        return true;
    }

    Bitboard bishops_and_queen = (position.by_piece_type[static_cast<int>(PieceType::Bishop)] |
                                  position.by_piece_type[static_cast<int>(PieceType::Queen)]) &
                                 position.by_color[static_cast<int>(us)] & ~square_bb(from);

    if ((bishop_attacks_from(king_square, occ_after) & bishops_and_queen) != 0) {
        return true;
    }

    return false;
}

static int negamax(Position& position, int depth, SearchState& state, int alpha, int beta, int ply,
                   bool allow_null = true) {
    if (ply >= MAX_PLY) {
        return evaluate(position);
    }
    state.pv_length[ply] = 0;

    // Проверяем, является ли данная нода principal variation
    bool pv_node = (beta - alpha) > 1;

    // Проверяем, остановлен ли поиск
    if (state.stopped) {
        return 0;
    }

    // Смотрим каждые 2048 узлов, истекло ли время
    if (state.nodes % 2048 == 0 && std::chrono::steady_clock::now() >= state.hard_deadline) {
        state.stopped = true;
        return 0;
    }

    // Если поиск шел через go nodes, проверяем, не вышли ли мы за лимит
    if (state.hard_node_limit > 0 && state.nodes >= state.hard_node_limit) {
        state.stopped = true;
        return 0;
    }

    state.nodes++;

    // Проверяем, повторялась ли позиция
    if (ply > 0 && is_repetition(position, state)) {
        return 0;
    }

    // Проверяем правило 50 ходов
    if (ply > 0 && position.halfmove_clock >= 100) {
        return 0;
    }

    if (depth == 0) {
        return quiescence(position, alpha, beta, state, ply);
    }

    int best = -INFINITE;
    Move best_move;

    // Transposition table
    int original_alpha = alpha;

    bool have_tt_move = false;
    Move tt_move = {0, 0};

    TTEntry entry;
    if (tt_probe(position.zobrist_hash, entry)) {
        have_tt_move = true;
        tt_move = entry.best_move;

        if (entry.depth >= depth && !pv_node) {
            int tt_score = decode_mate_score(entry.score, ply);
            if (entry.flag == TTFlag::Exact) {
                return tt_score;
            } else if (entry.flag == TTFlag::LowerBound && tt_score >= beta) {
                return tt_score;
            } else if (entry.flag == TTFlag::UpperBound && tt_score <= alpha) {
                return tt_score;
            }
        }
    }

    bool in_check = is_in_check(position);

    int static_eval = INFINITE;

    if (!in_check) {
        static_eval = evaluate(position);
    }

    // Reverse futility pruning
    if (!in_check && !pv_node && depth <= 11 && beta < MATE_THRESHOLD) {
        int margin = 80 * depth;
        if (static_eval - margin >= beta) {
            return static_eval;
        }
    }

    if (!in_check && has_non_pawn_material(position, position.side_to_move) && depth >= 3 &&
        allow_null && static_eval >= beta && !pv_node) {
        constexpr int R = 2;

        int saved_en_passant_sq = make_null_move(position);
        int null_score = -negamax(position, depth - 1 - R, state, -beta, -beta + 1, ply + 1, false);
        unmake_null_move(position, saved_en_passant_sq);

        if (!state.stopped && null_score >= beta) {
            return null_score >= MATE_THRESHOLD ? beta : null_score;
        }
    }

    // Генерируем все псевдолегальные ходы
    MoveList moves = generate_pseudo_legal_moves(position);

    // Сортируем ходы
    ScoredMoveList scored_moves = sort_moves(moves, position, state, ply, have_tt_move, tt_move);

    bool has_legal_move = false;

    int move_index = 0;  // Нужен для LMR

    Move quiets_tried[64];
    int quiets_count = 0;

    for (int i = 0; i < scored_moves.size(); i++) {
        int best_idx = i;

        for (int j = i + 1; j < scored_moves.size(); j++) {
            if (scored_moves[j].score > scored_moves[best_idx].score) {
                best_idx = j;
            }
        }

        if (best_idx != i) {
            std::swap(scored_moves[i], scored_moves[best_idx]);
        }

        const ScoredMove& sm = scored_moves[i];
        const Move& m = sm.move;

        UndoInfo undo;

        bool is_capture = position.board[m.to()] != Piece::None || m.flag() == MoveFlag::EnPassant;

        bool predicted = move_gives_check(position, m);

        // Futility pruning
        if (depth == 1 && !in_check && has_legal_move && !is_capture && !predicted &&
            m.flag() != MoveFlag::Promotion && alpha > -MATE_THRESHOLD && beta < MATE_THRESHOLD) {
            constexpr int FUTILITY_MARGIN = 350;
            if (static_eval + FUTILITY_MARGIN <= alpha) {
                continue;
            }
        }

        // LMP
        if (depth <= 4 && !pv_node && !in_check && !is_capture && m.flag() != MoveFlag::Promotion &&
            !predicted && m != state.killers[ply][0] && m != state.killers[ply][1] &&
            best > -MATE_THRESHOLD && move_index >= 2 + depth * depth) {
            continue;
        }

        if (!make_legal_move(position, m, undo)) {
            continue;
        }

        // Check extention
        int opp_king_square = king_square_of(position, position.side_to_move);
        bool gives_check =
            is_square_attacked_bb(position, opp_king_square, opposite_color(position.side_to_move));

        // Проверяем, выполнены ли условия для LMR
        bool can_reduce = depth >= 3 && move_index > 3 && !is_capture && !gives_check &&
                          !in_check && m != state.killers[ply][0] && m != state.killers[ply][1] &&
                          m.flag() != MoveFlag::Promotion;

        bool is_first_move = !has_legal_move;
        has_legal_move = true;
        move_index++;

        // Добавляем хэш позиции в историю
        state.history.push_back(position.zobrist_hash);

        // Вызываем функцию рекурсивно
        // Principal variation search
        int score;
        if (is_first_move) {
            // Если это первый ход, то ищем с широким окном
            score =
                -negamax(position, gives_check ? depth : depth - 1, state, -beta, -alpha, ply + 1);
        } else {
            // Вычисляем reduction из таблицы, инициализированной при старте
            int reduction = 0;
            if (can_reduce) {
                reduction = lmr_table[std::min(depth, 63)][std::min(move_index, 63)];
                if (pv_node) {
                    reduction--;
                }
                reduction = std::max(0, std::min(reduction, depth - 2));
            }

            int full_depth = gives_check ? depth : std::max(depth - 1, 0);
            int search_depth = gives_check ? depth : std::max(depth - 1 - reduction, 0);

            // 1. Сокращенный поиск с нулевым окном
            score = -negamax(position, search_depth, state, -alpha - 1, -alpha, ply + 1);

            // 2. Если сокращение дало fail-high, перепроверяем на полной глубине
            if (score > alpha && reduction > 0) {
                score = -negamax(position, full_depth, state, -alpha - 1, -alpha, ply + 1);
            }

            // 3. Если и на полной глубине fail-high, и мы в PV-узле - переискиваем с полным окном
            // ради точной оценки.
            if (score > alpha && score < beta) {
                score = -negamax(position, full_depth, state, -beta, -alpha, ply + 1);
            }
        }

        unmake_move(position, m, undo);

        // Удаляем хэш позиции из истории
        state.history.pop_back();

        if (state.stopped) {
            break;
        }

        // Обвновляем оценку
        if (score > best) {
            best = score;
            best_move = m;
        }

        // Обновляем альфу
        if (score > alpha) {
            alpha = score;

            // Обнволяем principal variation table
            state.pv_table[ply][0] = m;
            for (int i = 0; i < state.pv_length[ply + 1]; i++) {
                state.pv_table[ply][i + 1] = state.pv_table[ply + 1][i];
            }
            state.pv_length[ply] = state.pv_length[ply + 1] + 1;
        }

        // Отсекаем остальные варианты, если альфа больше беты
        if (alpha >= beta) {
            // Если тихий ход вызывал отсечение, сохраняем его
            bool is_capture =
                position.board[m.to()] != Piece::None || m.flag() == MoveFlag::EnPassant;

            if (!is_capture) {
                // Обновляем history heuristic
                update_history_heuristic(state, position.side_to_move, m.from(), m.to(),
                                         depth * depth);

                // Штрафуем тихие ходы, которые пробовали раньше и которые не вызывали отсечение
                for (int q = 0; q < quiets_count; q++) {
                    update_history_heuristic(state, position.side_to_move, quiets_tried[q].from(),
                                             quiets_tried[q].to(), -(depth * depth));
                }

                // Меняем ходы местами
                if (m != state.killers[ply][0]) {
                    state.killers[ply][1] = state.killers[ply][0];
                    state.killers[ply][0] = m;
                }
            }

            break;
        }

        // Запоминаем тихий ход, который не вызвал отсечение
        if (!is_capture && quiets_count < 64) {
            quiets_tried[quiets_count] = m;
            quiets_count++;
        }
    }

    // Проверка на мат и пат
    if (!has_legal_move) {
        int king_square = (position.side_to_move == Color::White) ? position.white_king_square
                                                                  : position.black_king_square;
        if (is_square_attacked_bb(position, king_square, opposite_color(position.side_to_move))) {
            return -(MATE - ply);
        }
        return 0;
    }

    // Определяем флаг для записи в таблице транспозиций
    TTFlag flag;
    if (!state.stopped) {
        if (best <= original_alpha) {
            flag = TTFlag::UpperBound;
        } else if (best >= beta) {
            flag = TTFlag::LowerBound;
        } else {
            flag = TTFlag::Exact;
        }

        // Сохраняем запись в таблицу
        tt_store(position.zobrist_hash, depth, encode_mate_score(best, ply), best_move, flag);
    }

    return best;
}

Move find_best_move(Position& position, int max_depth, SearchState& state) {
    // От количества вызовов функции за игру, зависит возраст записей в таблице
    // state.thread_id == 0 - значит, что задействован только 1 поток
    if (state.thread_id == 0) {
        tt_new_search();
    }

    auto search_start = std::chrono::steady_clock::now();

    MoveList moves = generate_legal_moves(position);
    if (moves.size() == 1) {
        return moves[0];
    }

    Move best_move = moves[0];

    int prev_score = 0;
    constexpr int ASPIRATION_WINDOW = 20;

    // Iterative deepening
    for (int depth = 1; depth <= max_depth && !state.stopped; depth++) {
        int alpha = -INFINITE;
        int beta = INFINITE;

        int window = ASPIRATION_WINDOW;

        if (depth >= 4) {
            alpha = std::max(-INFINITE, prev_score - ASPIRATION_WINDOW);
            beta = std::min(INFINITE, prev_score + ASPIRATION_WINDOW);
        }

        // Не начинаем новую итерацию, если на нее нет времени
        if (depth > 1) {
            auto elapsed = std::chrono::steady_clock::now() - search_start;
            auto soft_budget = state.soft_deadline - search_start;
            if (elapsed > soft_budget / 2) {
                break;
            }

            if (state.soft_node_limit > 0 && state.nodes >= state.soft_node_limit) {
                break;
            }
        }

        int best_score = -INFINITE;

        Move current_best_move;

        while (true) {
            int score = negamax(position, depth, state, alpha, beta, 0);

            if (state.stopped) {
                break;
            }

            if (score <= alpha) {
                alpha = std::max(-INFINITE, alpha - window);
                window += window / 3;
            } else if (score >= beta) {
                beta = std::min(INFINITE, beta + window);
                window += window / 3;
            } else {
                best_score = score;
                current_best_move = state.pv_table[0][0];
                break;
            }
        }

        if (!state.stopped) {
            best_move = current_best_move;

            // Печатаем информацию о поиске
            state.depth_reached = depth;
            auto search_end = std::chrono::steady_clock::now();
            double elapsed_seconds =
                std::chrono::duration<double>(search_end - search_start).count();

            if (state.thread_id == 0) {
                print_search_info(depth, state, best_score, elapsed_seconds);
            }

            prev_score = best_score;
        }
    }

    return best_move;
}

int quiescence(Position& position, int alpha, int beta, SearchState& state, int ply) {
    if (ply >= MAX_PLY) {
        return evaluate(position);
    }

    state.pv_length[ply] = 0;

    // Проверяем, остановлен ли поиск
    if (state.stopped) {
        return 0;
    }

    // Смотрим каждые 2048 узлов, истекло ли время
    if (state.nodes % 2048 == 0 && std::chrono::steady_clock::now() >= state.hard_deadline) {
        state.stopped = true;
        return 0;
    }

    // Если поиск шел через go nodes, проверяем, не вышли ли мы за лимит
    if (state.hard_node_limit > 0 && state.nodes >= state.hard_node_limit) {
        state.stopped = true;
        return 0;
    }

    state.nodes++;

    bool in_check = is_in_check(position);

    int original_alpha = alpha;

    // Ищем запись в таблице транспозиций
    if (state.use_tt) {
        TTEntry entry;
        if (tt_probe(position.zobrist_hash, entry)) {
            int tt_score = decode_mate_score(entry.score, ply);
            if (entry.flag == TTFlag::Exact) {
                return tt_score;
            } else if (entry.flag == TTFlag::LowerBound && tt_score >= beta) {
                return tt_score;
            } else if (entry.flag == TTFlag::UpperBound && tt_score <= alpha) {
                return tt_score;
            }
        }
    }

    Move best_move;

    int best;
    if (!in_check) {
        int stand_pat = evaluate(position);
        best = stand_pat;

        if (stand_pat >= beta) {
            return stand_pat;
        }

        if (stand_pat > alpha) {
            alpha = stand_pat;
        }
    } else {
        best = -INFINITE;
    }

    MoveList moves =
        in_check ? generate_pseudo_legal_moves(position) : generate_capture_moves(position);

    // Сортировка через MVV-LVA
    ScoredMoveList scored_moves;
    for (const Move& m : moves) {
        int score = mvv_lva_score(position, m);
        if (m.flag() == MoveFlag::Promotion) {
            score += (promotion_values[static_cast<int>(m.promotion())] - 100) * 10;
        }
        scored_moves.add(m, score);
    }

    bool has_legal_move = false;

    for (int i = 0; i < scored_moves.size(); i++) {
        const ScoredMove& sm = scored_moves[i];

        int best_idx = i;

        for (int j = i + 1; j < scored_moves.size(); j++) {
            if (scored_moves[j].score > scored_moves[best_idx].score) {
                best_idx = j;
            }
        }

        if (best_idx != i) {
            std::swap(scored_moves[i], scored_moves[best_idx]);
        }

        const Move& m = sm.move;

        bool is_capture = position.board[m.to()] != Piece::None || m.flag() == MoveFlag::EnPassant;

        if (in_check && !is_capture && best > -MATE_THRESHOLD) {
            continue;
        };

        // Пропускаем не выгодные взятия
        if (!in_check && is_capture && see_capture(position, m) < 0) {
            continue;
        }

        UndoInfo undo;

        if (!make_legal_move(position, m, undo)) {
            continue;
        }

        tt_prefetch(position.zobrist_hash);

        has_legal_move = true;

        // Вызываем функцию рекурсивно
        int score = -quiescence(position, -beta, -alpha, state, ply + 1);

        unmake_move(position, m, undo);

        if (state.stopped) {
            break;
        }

        // Обвновляем оценку
        if (score > best) {
            best = score;
            best_move = m;
        }

        // Обновляем альфу
        if (score > alpha) {
            alpha = score;
        }

        // Отсекаем остальные варианты, если альфа больше беты
        if (alpha >= beta) {
            break;
        }
    }

    if (in_check && !has_legal_move) {
        return -(MATE - ply);
    }

    // Сохраняем запись в таблицу транспозиций
    TTFlag flag;
    if (!state.stopped && state.use_tt) {
        if (best <= original_alpha) {
            flag = TTFlag::UpperBound;
        } else if (best >= beta) {
            flag = TTFlag::LowerBound;
        } else {
            flag = TTFlag::Exact;
        }

        tt_store(position.zobrist_hash, 0, encode_mate_score(best, ply), best_move, flag);
    }

    return best;
}

void prepare_helper_state(SearchState& helper, const SearchState& main) {
    helper.hard_deadline = main.hard_deadline;
    helper.soft_deadline = main.soft_deadline;
    helper.history = main.history;
    helper.use_tt = main.use_tt;
    helper.nodes = 0;
    helper.stopped = false;
    helper.soft_node_limit = 0;
    helper.hard_node_limit = 0;
}

// Печатает информацию о поиске в stdout
void print_search_info(int depth, const SearchState& state, int best_score,
                       double elapsed_seconds) {
    // Вычисляем NPS
    long nps = (elapsed_seconds > 0) ? static_cast<long>(state.nodes / elapsed_seconds) : 0;

    // Печатаем вывод
    std::cout << "info depth " << depth << " nodes " << state.nodes << " time "
              << static_cast<long>(elapsed_seconds * 1000) << " nps " << nps;

    // Печатаем, сколько ходов до мата либо обычную оценку позиции
    if (best_score >= MATE_THRESHOLD) {
        int plies_to_mate = MATE - best_score;
        int moves_to_mate = (plies_to_mate + 1) / 2;
        std::cout << " score mate " << moves_to_mate;
    } else if (best_score <= -MATE_THRESHOLD) {
        int plies_to_mate = MATE + best_score;
        int moves_to_mate = (plies_to_mate + 1) / 2;
        std::cout << " score mate -" << moves_to_mate;
    } else {
        std::cout << " score cp " << best_score;
    }

    // Печатать PV
    std::cout << " pv ";
    for (int i = 0; i < state.pv_length[0]; i++) {
        std::cout << move_to_uci(state.pv_table[0][i]) << " ";
    }

    std::cout << "\n";
    std::cout.flush();
}

ScoredMoveList sort_moves(const MoveList& moves, const Position& position, const SearchState& state,
                          int ply, bool have_tt_move, const Move& tt_move) {
    ScoredMoveList scored_moves;

    // Сортировка killer moves
    auto move_score = [&](const Move& m) {
        // Если ход уже есть в таблице транспозиций, сортируем его первым
        if (have_tt_move && m == tt_move) {
            return 1000000;
        }

        bool is_capture = position.board[m.to()] != Piece::None || m.flag() == MoveFlag::EnPassant;
        bool is_promotion = m.flag() == MoveFlag::Promotion;
        int promotion_bonus =
            is_promotion ? (promotion_values[static_cast<int>(m.promotion())] - 100) * 10 : 0;

        if (is_capture) {
            int score = mvv_lva_score(position, m) + promotion_bonus;
            if (see_capture(position, m) < 0) {
                score -= 20000;  // помечаем взятие, как не выгодное
            } else {
                score += MAX_HISTORY;
            }
            return score;
        } else if (is_promotion) {
            return promotion_bonus + MAX_HISTORY;
        } else {
            if (m == state.killers[ply][0]) {
                return MAX_HISTORY + 2;
            } else if (m == state.killers[ply][1]) {
                return MAX_HISTORY + 1;
            } else {
                // Ходы из history heuristic
                return state
                    .history_heuristic[static_cast<int>(position.side_to_move)][m.from()][m.to()];
            }
        }
    };

    for (const Move& m : moves) {
        scored_moves.add(m, move_score(m));
    }

    return scored_moves;
}

void update_history_heuristic(SearchState& state, Color side, int from, int to, int bonus) {
    int clamped_bonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);

    // Достаём уже накопленную ценность хода
    int& value = state.history_heuristic[static_cast<int>(side)][from][to];

    // Прибавляем к ней бонус
    value += clamped_bonus - value * std::abs(clamped_bonus) / MAX_HISTORY;
}

static int see(const Position& position, int square, PieceType target_type, PieceType attacker_type,
               Color side, Bitboard occupancy, Bitboard from_set) {
    int gain[32];
    int d = 0;
    Color current_side = side;

    // Битборд фигур всех цветов, которые могут вскрыть что-то за собой при снятии
    Bitboard may_xray = position.by_piece_type[static_cast<int>(PieceType::Pawn)] |
                        position.by_piece_type[static_cast<int>(PieceType::Bishop)] |
                        position.by_piece_type[static_cast<int>(PieceType::Rook)] |
                        position.by_piece_type[static_cast<int>(PieceType::Queen)];

    Bitboard attackers = attackers_to(position, square, occupancy);

    // Ценность фигуры, которая стояла на клетке изначально
    gain[0] = see_values[static_cast<int>(target_type)];
    do {
        d++;  // Переходим на следующий шаг

        gain[d] = see_values[static_cast<int>(attacker_type)] - gain[d - 1];

        // Убираем текущую фигуру, которая делала взятие
        attackers ^= from_set;
        occupancy ^= from_set;

        // Если атакующая фигура могла что-то вскрыть за собой, пересчитываем атаки на клетку
        if (from_set & may_xray) {
            attackers |= attackers_to(position, square, occupancy) & occupancy;
        }

        current_side = opposite_color(current_side);

        // Находим следующую наименее значительную атакующую фигуру для другой стороны
        from_set = least_valuable_attacker(position, attackers, current_side, attacker_type);
    } while (from_set != 0);

    while (--d) {
        gain[d - 1] = -std::max(-gain[d - 1], gain[d]);
    }

    return gain[0];
}

int see_capture(const Position& position, const Move& move) {
    int square = move.to();
    Color side = position.side_to_move;
    Bitboard occupancy = position.by_color[0] | position.by_color[1];
    Bitboard from_set = square_bb(move.from());
    PieceType attacker_type = piece_type_of(position.board[move.from()]);
    PieceType target_type = piece_type_of(position.board[move.to()]);

    if (move.flag() == MoveFlag::EnPassant) {
        target_type = PieceType::Pawn;
        int captured_square = square_of(rank_of(move.from()), file_of(move.to()));
        occupancy &= ~square_bb(captured_square);
    }

    int value = see(position, square, target_type, attacker_type, side, occupancy, from_set);

    return value;
}
