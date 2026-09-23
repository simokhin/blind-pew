#include "perft.h"

long perft(Position& p, int depth) {
    if (depth == 0) {
        return 1;
    }

    MoveList moves = generate_legal_moves(p);
    long nodes = 0;

    for (const Move& m : moves) {
        UndoInfo undo = make_move(p, m);
        nodes += perft(p, depth - 1);
        unmake_move(p, m, undo);
    }

    return nodes;
}