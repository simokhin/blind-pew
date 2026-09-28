#include "bench.h"

#include <tt.h>

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "constants.h"
#include "fen.h"
#include "position.h"
#include "search.h"

std::vector<std::string> positions = {
    START_FEN,
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 10",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 11",
    "4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B2PP/4RRK1 b - - 7 19",
    "r1bbk1nr/pp3p1p/2n5/1N4p1/2Np1B2/8/PPP2PPP/2KR1B1R w kq - 0 13",
    "3q2k1/pb3p1p/4pbp1/2r5/PpN2N2/1P2P2P/5PP1/Q2R2K1 b - - 4 26",
    "6k1/6p1/6Pp/ppp5/3pn2P/1P3K2/1PP2P2/3N4 b - - 0 1",
    "3b4/5kp1/1p1p1p1p/pP1PpP1P/P1P1P3/3KN3/8/8 w - - 0 1",
    "8/6pk/1p6/8/PP3p1p/5P2/4KP1q/3Q4 w - - 0 1",
    "8/2p5/8/2kPKp1p/2p4P/2P5/3P4/8 w - - 0 1",
    "8/pp2r1k1/2p1p3/3pP2p/1P1P1P1P/P5KR/8/8 w - - 0 1",
    "8/8/8/8/5kp1/P7/8/1K1N4 w - - 0 1",
    "6k1/3b3r/1p1p4/p1n2p2/1PPNpP1q/P3Q1p1/1R1RB1P1/5K2 b - - 0 1",
};

void run_bench(int depth) {
    clear_transposition_table();

    long total_nodes = 0;
    double total_time = 0;

    for (const std::string& fen : positions) {
        Position position = parse_fen(fen);
        SearchState state;
        state.hard_deadline = std::chrono::steady_clock::time_point::max();
        state.soft_deadline = std::chrono::steady_clock::time_point::max();

        auto start = std::chrono::steady_clock::now();

        find_best_move(position, depth, state);

        double elapsed =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

        total_nodes += state.nodes;
        total_time += elapsed;
    }

    long nps = (total_time > 0) ? static_cast<long>(total_nodes / total_time) : 0;

    std::cout << "Total nodes: " << total_nodes << "\n";
    std::cout << "Total time: " << total_time << "s\n";
    std::cout << "NPS: " << nps << "\n";
}
