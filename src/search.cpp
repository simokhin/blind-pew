#include "search.h"

#include "board.h"
#include "constants.h"
#include "evaluate.h"
#include "movegen.h"

int negamax(Position& position, int depth, SearchState& state, int ply) {
    state.nodes++;

    if (depth == 0) {
        return evaluate(position);
    }

    int best = -INFINITE;

    MoveList moves = generate_pseudo_legal_moves(position);

    bool has_legal_move = false;

    for (const Move& m : moves) {
        // Определяем, чей ход был до хода
        Color mover = position.side_to_move;

        // Делаем ход
        UndoInfo undo = make_move(position, m);

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
            continue;
        }

        has_legal_move = true;

        int score = -negamax(position, depth - 1, state, ply + 1);

        unmake_move(position, m, undo);

        if (score > best) {
            best = score;
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

    return best;
}

Move find_best_move(Position& position, int depth, SearchState& state) {
    MoveList moves = generate_legal_moves(position);

    int best_score = -INFINITE;

    Move best_move = {0, 0};

    for (const Move& m : moves) {
        UndoInfo undo = make_move(position, m);

        int score = -negamax(position, depth - 1, state, 1);

        unmake_move(position, m, undo);

        if (score > best_score) {
            best_score = score;
            best_move = m;
        }
    }

    return best_move;
}
