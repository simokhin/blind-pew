#include "perft.h"

#include <format>
#include <iostream>
#include <string>

#include "bitboard.h"
#include "constants.h"
#include "fen.h"
#include "zobrist.h"

bool check(const std::string& name, const std::string& fen, int depth, long expected) {
    Position position = parse_fen(fen);
    long result = perft(position, depth);

    if (result == expected) {
        std::cerr << std::format("OK {}\n", name);
        return true;
    } else {
        std::cerr << std::format("FAIL {}: expected {}, got {}\n", name, expected, result);
        return false;
    }
}

int main() {
    init_zobrist_keys();

    init_rook_magics();
    init_bishop_magics();

    bool ok = true;

    // Initial position
    ok = check("startpos depth 5", START_FEN, 5, 4865609) && ok;

    // Position 2 (Kiwipete)
    ok =
        check("kiwipete depth 4",
              "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4, 4085603) &&
        ok;

    // Position 3
    ok = check("position 3 depth 4", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 4, 43238) && ok;

    // Position 4
    ok = check("position 4 depth 4",
               "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4, 422333) &&
         ok;

    // Position 5
    ok = check("position 5 depth 4", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4,
               2103487) &&
         ok;

    return ok ? 0 : 1;
}
