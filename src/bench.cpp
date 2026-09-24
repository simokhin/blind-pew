#include "bench.h"

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
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
};

void run_bench(int depth) {
    long total_nodes = 0;
    double total_time = 0;

    for (const std::string& fen : positions) {
        Position position = parse_fen(fen);
        SearchState state;
        state.deadline = std::chrono::steady_clock::time_point::max();

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