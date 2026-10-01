#include "perft.h"

#include <iostream>

#include "bitboard.h"
#include "constants.h"
#include "fen.h"
#include "zobrist.h"

int main() {
    init_zobrist_keys();
    init_knight_attacks();
    init_king_attacks();
    init_pawn_attacks();
    init_rook_magics();
    init_bishop_magics();

    Position position = parse_fen(START_FEN);

    constexpr long EXPECTED = 4865609;
    long result = perft(position, 5);

    if (result != EXPECTED) {
        std::cerr << "FAIL startpos depth 5: expected " << EXPECTED << ", got " << result << "\n";
        return 1;
    } else {
        std::cerr << "OK startpos depth 5\n";
        return 0;
    }
}
