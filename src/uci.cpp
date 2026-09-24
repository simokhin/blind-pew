#include "uci.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

#include "bench.h"
#include "constants.h"
#include "fen.h"
#include "movegen.h"
#include "position.h"
#include "search.h"

void uci_loop() {
    std::string line;

    Position position = parse_fen(START_FEN);

    while (std::getline(std::cin, line)) {
        std::istringstream stream(line);
        std::string command;
        stream >> command;

        if (command == "quit") {
            break;
        }

        if (command == "uci") {
            std::cout << "id name " << ENGINE_NAME << "\n";
            std::cout << "id author " << ENGINE_AUTHOR << "\n";
            std::cout << "uciok\n";
        }

        if (command == "isready") {
            std::cout << "readyok\n";
        }

        if (command == "position") {
            std::string sub_command;
            stream >> sub_command;

            if (sub_command == "startpos") {
                position = parse_fen(START_FEN);
            } else if (sub_command == "fen") {
                std::string fen;

                for (int i = 0; i < 6; i++) {
                    std::string token;
                    stream >> token;
                    fen += token + " ";
                }

                position = parse_fen(fen);
            }

            std::string moves_token;
            if (stream >> moves_token && moves_token == "moves") {
                std::string move_str;

                while (stream >> move_str) {
                    int from = square_from_algebraic(move_str.substr(0, 2));
                    int to = square_from_algebraic(move_str.substr(2, 2));
                    PromotionPiece promotion_piece = PromotionPiece::Queen;

                    if (move_str.length() == 5) {
                        switch (move_str[4]) {
                            case 'q':
                                promotion_piece = PromotionPiece::Queen;
                                break;
                            case 'r':
                                promotion_piece = PromotionPiece::Rook;
                                break;
                            case 'b':
                                promotion_piece = PromotionPiece::Bishop;
                                break;
                            case 'n':
                                promotion_piece = PromotionPiece::Knight;
                                break;
                            default:
                                break;
                        }
                    }

                    MoveList legal_moves = generate_legal_moves(position);

                    for (const Move& m : legal_moves) {
                        if (m.from() == from && m.to() == to) {
                            if (m.flag() == MoveFlag::Promotion &&
                                m.promotion() != promotion_piece) {
                                continue;
                            }
                            make_move(position, m);
                            break;
                        }
                    }
                }
            }
        }

        if (command == "go") {
            SearchState state;

            // Значения по умолчанию
            int max_depth = 64;
            state.deadline = std::chrono::steady_clock::time_point::max();

            std::string token;

            int wtime = 0;
            int btime = 0;
            int winc = 0;
            int binc = 0;

            while (stream >> token) {
                if (token == "depth") {
                    stream >> max_depth;
                } else if (token == "movetime") {
                    int ms;
                    stream >> ms;
                    state.deadline =
                        std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
                } else if (token == "infinite") {
                    // Используются значения по умолчанию
                } else if (token == "wtime") {
                    stream >> wtime;
                } else if (token == "btime") {
                    stream >> btime;
                } else if (token == "winc") {
                    stream >> winc;
                } else if (token == "binc") {
                    stream >> binc;
                }
            }

            if (wtime > 0 || btime > 0) {
                if (position.side_to_move == Color::White) {
                    int time_for_move = wtime / 20;
                    state.deadline = std::chrono::steady_clock::now() +
                                     std::chrono::milliseconds(time_for_move + winc);
                } else if (position.side_to_move == Color::Black) {
                    int time_for_move = btime / 20;
                    state.deadline = std::chrono::steady_clock::now() +
                                     std::chrono::milliseconds(time_for_move + binc);
                }
            }

            auto search_start = std::chrono::steady_clock::now();

            Move best_move = find_best_move(position, max_depth, state);

            auto search_end = std::chrono::steady_clock::now();
            double elapsed_seconds =
                std::chrono::duration<double>(search_end - search_start).count();
            long nps = (elapsed_seconds > 0) ? static_cast<long>(state.nodes / elapsed_seconds) : 0;

            std::cout << "info depth " << state.depth_reached << " nodes " << state.nodes << " nps "
                      << nps << "\n";
            std::cout << "bestmove " << algebraic_from_square(best_move.from())
                      << algebraic_from_square(best_move.to());

            if (best_move.flag() == MoveFlag::Promotion) {
                switch (best_move.promotion()) {
                    case PromotionPiece::Queen:
                        std::cout << "q" << "\n";
                        break;
                    case PromotionPiece::Rook:
                        std::cout << "r" << "\n";
                        break;
                    case PromotionPiece::Knight:
                        std::cout << "n" << "\n";
                        break;
                    case PromotionPiece::Bishop:
                        std::cout << "b" << "\n";
                        break;
                    default:
                        break;
                }
            } else {
                std::cout << "\n";
            }
        }

        if (command == "bench") {
            int depth = 4;
            stream >> depth;
            run_bench(depth);
        }
    }
}