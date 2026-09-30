#include "evaluate.h"

#include <algorithm>

#include "magic_constants.h"

int evaluate(const Position& position) {
    int pieces = popcount(position.by_color[0] | position.by_color[1]);
    int bucket = (pieces - 2) / 4;

    return nnue_evaluate(
        position.accumulators[static_cast<int>(position.side_to_move)],
        position.accumulators[static_cast<int>(opposite_color(position.side_to_move))], bucket);
}
