#include "evaluate.h"

#include <algorithm>

#include "magic_constants.h"

int evaluate(const Position& position) {
    return nnue_evaluate(
        position.accumulators[static_cast<int>(position.side_to_move)],
        position.accumulators[static_cast<int>(opposite_color(position.side_to_move))]);
}
