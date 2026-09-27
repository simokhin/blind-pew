#include "search.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "board.h"
#include "constants.h"
#include "evaluate.h"
#include "movegen.h"
#include "tt.h"

int lmr_table[64][64];

void init_lmr_table() {
    for (int depth = 1; depth < 64; depth++) {
        for (int move_index = 1; move_index < 64; move_index++) {
            // Формула: base + ln(depth)*ln(move_index)/scale.
            lmr_table[depth][move_index] =
                static_cast<int>(0.7844 + std::log(depth) * std::log(move_index) / 2.4696);
        }
    }
}

void ScoredMoveList::add(const Move& move, int score) {
    scored_moves[count] = {move, score};
    count++;
}
int ScoredMoveList::size() const { return count; }

const ScoredMove& ScoredMoveList::operator[](int index) const { return scored_moves[index]; }
ScoredMove& ScoredMoveList::operator[](int index) { return scored_moves[index]; }
const ScoredMove* ScoredMoveList::begin() const { return scored_moves.data(); };
const ScoredMove* ScoredMoveList::end() const { return scored_moves.data() + count; };
ScoredMove* ScoredMoveList::begin() { return scored_moves.data(); };
ScoredMove* ScoredMoveList::end() { return scored_moves.data() + count; }

bool make_legal_move(Position& position, const Move& m, UndoInfo& undo) {
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

int negamax(Position& position, int depth, SearchState& state, int alpha, int beta, int ply,
            bool allow_null) {
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

    state.nodes++;

    // Проверяем, повторялась ли позиция
    if (std::count(state.history.begin(), state.history.end(), position.zobrist_hash) >= 2) {
        return 0;
    }

    // Проверяем правило 50 ходов
    if (position.halfmove_clock >= 100) {
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

    TTEntry* entry = tt_probe(position.zobrist_hash);
    if (entry != nullptr) {
        have_tt_move = true;
        tt_move = entry->best_move;

        if (entry->depth >= depth) {
            int tt_score = decode_mate_score(entry->score, ply);
            if (entry->flag == TTFlag::Exact) {
                return tt_score;
            } else if (entry->flag == TTFlag::LowerBound && tt_score >= beta) {
                return tt_score;
            } else if (entry->flag == TTFlag::UpperBound && tt_score <= alpha) {
                return tt_score;
            }
        }
    }

    // Null move pruning
    bool in_check = is_square_attacked_bb(position, king_square_of(position, position.side_to_move),
                                          opposite_color(position.side_to_move));

    if (!in_check && has_non_pawn_material(position, position.side_to_move) && depth >= 3 &&
        allow_null) {
        constexpr int R = 2;

        int saved_en_passant_sq = make_null_move(position);
        int null_score = -negamax(position, depth - 1 - R, state, -beta, -beta + 1, ply + 1, false);
        unmake_null_move(position, saved_en_passant_sq);

        if (!state.stopped && null_score >= beta) {
            return null_score;
        }
    }

    // Генерируем все псевдолегальные ходы
    MoveList moves = generate_pseudo_legal_moves(position);

    // Сортируем ходы
    ScoredMoveList scored_moves = sort_moves(moves, position, state, ply, have_tt_move, tt_move);

    bool has_legal_move = false;

    int move_index = 0;  // Нужен для LMR

    for (const ScoredMove& sm : scored_moves) {
        const Move& m = sm.move;

        UndoInfo undo;

        bool is_capture = position.board[m.to()] != Piece::None || m.flag() == MoveFlag::EnPassant;

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

            int search_depth = gives_check ? depth : std::max(depth - 1 - reduction, 0);

            score = -negamax(position, search_depth, state, -alpha - 1, -alpha, ply + 1);
            if (score > alpha && score < beta) {
                score = -negamax(position, gives_check ? depth : depth - 1, state, -beta, -alpha,
                                 ply + 1);
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

                // Меняем ходы местами
                if (m != state.killers[ply][0]) {
                    state.killers[ply][1] = state.killers[ply][0];
                    state.killers[ply][0] = m;
                }
            }

            break;
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
    auto search_start = std::chrono::steady_clock::now();

    MoveList moves = generate_legal_moves(position);
    if (moves.size() == 1) {
        return moves[0];
    }

    Move best_move = moves[0];

    // Iterative deepening
    for (int depth = 1; depth <= max_depth && !state.stopped; depth++) {
        // Не начинаем новую итерацию, если на нее нет времени
        if (depth > 1) {
            auto elapsed = std::chrono::steady_clock::now() - search_start;
            auto soft_budget = state.soft_deadline - search_start;
            if (elapsed > soft_budget / 2) {
                break;
            }
        }

        int best_score = -INFINITE;

        Move current_best_move;

        for (const Move& m : moves) {
            UndoInfo undo = make_move(position, m);

            // Добавляем хэш позиции в историю
            state.history.push_back(position.zobrist_hash);

            int score = -negamax(position, depth - 1, state, -INFINITE, -best_score, 1);

            unmake_move(position, m, undo);

            // Удаляем хэш позиции из истории
            state.history.pop_back();

            if (state.stopped) {
                break;
            }

            if (score > best_score) {
                best_score = score;
                current_best_move = m;

                // Обновляем principal variation table
                state.pv_table[0][0] = m;
                for (int i = 0; i < state.pv_length[1]; i++) {
                    state.pv_table[0][i + 1] = state.pv_table[1][i];
                }
                state.pv_length[0] = state.pv_length[1] + 1;
            }
        }

        if (!state.stopped) {
            best_move = current_best_move;

            // Ставим лучший найденный ход в начало списка
            auto it = std::find(moves.begin(), moves.end(), best_move);
            if (it != moves.end()) {
                std::swap(*it, *moves.begin());
            }

            // Печатаем информацию о поиске
            state.depth_reached = depth;
            auto search_end = std::chrono::steady_clock::now();
            double elapsed_seconds =
                std::chrono::duration<double>(search_end - search_start).count();

            print_search_info(depth, state, best_score, elapsed_seconds);
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

    state.nodes++;

    int stand_pat = evaluate(position);
    int best = stand_pat;

    if (stand_pat >= beta) {
        return stand_pat;
    }

    if (stand_pat > alpha) {
        alpha = stand_pat;
    }

    MoveList moves = generate_capture_moves(position);

    // Сортировка через MVV-LVA
    ScoredMoveList scored_moves;
    for (const Move& m : moves) {
        int score = mvv_lva_score(position, m);
        if (m.flag() == MoveFlag::Promotion) {
            score += (promotion_values[static_cast<int>(m.promotion())] - 100) * 10;
        }
        scored_moves.add(m, score);
    }
    std::sort(scored_moves.begin(), scored_moves.end(),
              [](const ScoredMove& a, const ScoredMove& b) { return a.score > b.score; });

    for (const ScoredMove& sm : scored_moves) {
        const Move& m = sm.move;

        bool is_capture = position.board[m.to()] != Piece::None || m.flag() == MoveFlag::EnPassant;

        if (is_capture && see_capture(position, m) < 0) {
            continue;
        }

        UndoInfo undo;

        if (!make_legal_move(position, m, undo)) {
            continue;
        }

        // Вызываем функцию рекурсивно
        int score = -quiescence(position, -beta, -alpha, state, ply + 1);

        unmake_move(position, m, undo);

        if (state.stopped) {
            break;
        }

        // Обвновляем оценку
        if (score > best) {
            best = score;
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

    return best;
}

int mvv_lva_score(const Position& position, const Move& m) {
    int victim = static_cast<int>(piece_type_of(position.board[m.to()]));
    int attacker = static_cast<int>(piece_type_of(position.board[m.from()]));

    if (m.flag() == MoveFlag::EnPassant) {
        return values[static_cast<int>(PieceType::Pawn)] * 10 - values[attacker];
    }

    return values[victim] * 10 - values[attacker];
}

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

bool has_non_pawn_material(const Position& position, Color color) {
    Bitboard pawns_and_king = position.by_piece_type[static_cast<int>(PieceType::Pawn)] |
                              position.by_piece_type[static_cast<int>(PieceType::King)];

    return (position.by_color[static_cast<int>(color)] & ~pawns_and_king) != 0;
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

    // Сортировка через MVV-LVA
    std::sort(scored_moves.begin(), scored_moves.end(),
              [](const ScoredMove& a, const ScoredMove& b) { return a.score > b.score; });

    return scored_moves;
}

void update_history_heuristic(SearchState& state, Color side, int from, int to, int bonus) {
    int clamped_bonus = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);

    // Достаём уже накопленную ценность хода
    int& value = state.history_heuristic[static_cast<int>(side)][from][to];

    // Прибавляем к ней бонус
    value += clamped_bonus - value * std::abs(clamped_bonus) / MAX_HISTORY;
}

int see(const Position& position, int square, PieceType target_type, PieceType attacker_type,
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
