#include "search.h"

#include <algorithm>
#include <iostream>

#include "board.h"
#include "constants.h"
#include "evaluate.h"
#include "movegen.h"
#include "tt.h"

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
    if (is_square_attacked(position, king_square, position.side_to_move)) {
        unmake_move(position, m, undo);
        return false;
    }

    return true;
}

int negamax(Position& position, int depth, SearchState& state, int alpha, int beta, int ply) {
    state.pv_length[ply] = 0;

    // Проверяем, остановлен ли поиск
    if (state.stopped) {
        return 0;
    }

    // Смотрим каждые 2048 узлов, истекло ли время
    if (state.nodes % 2048 == 0 && std::chrono::steady_clock::now() >= state.deadline) {
        state.stopped = true;
        return 0;
    }

    state.nodes++;

    // Проверяем, повторялась ли позиция
    if (std::count(state.history.begin(), state.history.end(), position.zobrist_hash) >= 2) {
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

    // Генерируем все псевдолегальные ходы
    MoveList moves = generate_pseudo_legal_moves(position);

    // Сортировка через MVV-LVA с поправкой на Transposposition table
    std::sort(moves.begin(), moves.end(),
              [&position, have_tt_move, tt_move](const Move& a, const Move& b) {
                  if (have_tt_move) {
                      if (a == tt_move) {
                          return true;
                      } else if (b == tt_move) {
                          return false;
                      }
                  }
                  return mvv_lva_score(position, a) > mvv_lva_score(position, b);
              });

    bool has_legal_move = false;

    for (const Move& m : moves) {
        UndoInfo undo;

        if (!make_legal_move(position, m, undo)) {
            continue;
        }

        // Check extention
        int opp_king_square = king_square_of(position, position.side_to_move);
        bool gives_check =
            is_square_attacked(position, opp_king_square, opposite_color(position.side_to_move));

        has_legal_move = true;

        // Добавляем хэш позиции в историю
        state.history.push_back(position.zobrist_hash);

        // Вызываем функцию рекурсивно
        int score =
            -negamax(position, gives_check ? depth : depth - 1, state, -beta, -alpha, ply + 1);

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
            break;
        }
    }

    // Проверка на мат и пат
    if (!has_legal_move) {
        int king_square = (position.side_to_move == Color::White) ? position.white_king_square
                                                                  : position.black_king_square;
        if (is_square_attacked(position, king_square, opposite_color(position.side_to_move))) {
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

    Move best_move = moves[0];

    // Iterative deepening
    for (int depth = 1; depth <= max_depth && !state.stopped; depth++) {
        int best_score = -INFINITE;

        Move current_best_move;

        for (const Move& m : moves) {
            UndoInfo undo = make_move(position, m);

            // Добавляем хэш позиции в историю
            state.history.push_back(position.zobrist_hash);

            int score = -negamax(position, depth - 1, state, -INFINITE, INFINITE, 1);

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

            // Печатаем информацию о поиске, используя principal variation table
            state.depth_reached = depth;
            auto search_end = std::chrono::steady_clock::now();
            double elapsed_seconds =
                std::chrono::duration<double>(search_end - search_start).count();
            long nps = (elapsed_seconds > 0) ? static_cast<long>(state.nodes / elapsed_seconds) : 0;

            std::cout << "info depth " << depth << " nodes " << state.nodes << " nps " << nps
                      << " pv ";
            for (int i = 0; i < state.pv_length[0]; i++) {
                std::cout << move_to_uci(state.pv_table[0][i]) << " ";
            }
            std::cout << "\n";
        }
    }

    return best_move;
}

int quiescence(Position& position, int alpha, int beta, SearchState& state, int ply) {
    state.pv_length[ply] = 0;

    // Проверяем, остановлен ли поиск
    if (state.stopped) {
        return 0;
    }

    // Смотрим каждые 2048 узлов, истекло ли время
    if (state.nodes % 2048 == 0 && std::chrono::steady_clock::now() >= state.deadline) {
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
    std::sort(moves.begin(), moves.end(), [&position](const Move& a, const Move& b) {
        return mvv_lva_score(position, a) > mvv_lva_score(position, b);
    });

    for (const Move& m : moves) {
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
    int victim = static_cast<int>(position.board[m.to()]);
    int attacker = static_cast<int>(position.board[m.from()]);

    if (m.flag() == MoveFlag::EnPassant) {
        return values[static_cast<int>(Piece::WP)] * 10 - values[attacker];
    }

    return values[victim] * 10 - values[attacker];
}
