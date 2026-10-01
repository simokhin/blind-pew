#include "uci.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "bench.h"
#include "constants.h"
#include "fen.h"
#include "movegen.h"
#include "position.h"
#include "search.h"
#include "tt.h"

void uci_loop() {
    std::string line;

    Position position = parse_fen(START_FEN);

    std::vector<uint64_t> position_history;

    std::vector<SearchState> states(1);
    std::vector<SearchState> helper_states;

    int thread_count = 1;

    SearchState& state = states[0];

    std::thread search_thread;
    std::vector<std::thread> helper_threads;

    auto join_helpers = [&helper_threads]() {
        for (std::thread& t : helper_threads) {
            t.join();
        }
        helper_threads.clear();
    };

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
            std::cout << "option name Hash type spin default 16 min 1 max 65536\n";
            std::cout << "option name Threads type spin default 1 min 1 max 64\n";
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
                position_history.clear();
                position_history.push_back(position.zobrist_hash);
            } else if (sub_command == "fen") {
                std::string fen;

                for (int i = 0; i < 6; i++) {
                    std::string token;
                    stream >> token;
                    fen += token + " ";
                }

                position = parse_fen(fen);
                position_history.clear();
                position_history.push_back(position.zobrist_hash);
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
                            position_history.push_back(position.zobrist_hash);
                            break;
                        }
                    }
                }
            }
        }

        if (command == "go") {
            // Значения по умолчанию
            int max_depth = 64;
            state.hard_deadline = std::chrono::steady_clock::time_point::max();
            state.soft_deadline = std::chrono::steady_clock::time_point::max();

            std::string token;

            int wtime = 0;
            int btime = 0;
            int winc = 0;
            int binc = 0;

            state.stopped = false;
            state.nodes = 0;

            state.soft_node_limit = 0;
            state.hard_node_limit = 0;

            if (search_thread.joinable()) {
                search_thread.join();
            }

            // Дожидаемся завершения работы каждого потока перед тем, как начать новый поиск
            join_helpers();

            while (stream >> token) {
                if (token == "depth") {
                    stream >> max_depth;
                } else if (token == "movetime") {
                    int ms;
                    stream >> ms;
                    state.hard_deadline =
                        std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
                    state.soft_deadline = state.hard_deadline;
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
                } else if (token == "nodes") {
                    long nodes;
                    stream >> nodes;
                    state.hard_node_limit = nodes;
                    state.soft_node_limit = nodes;
                }
            }

            if (wtime > 0 || btime > 0) {
                if (position.side_to_move == Color::White) {
                    int soft_time = wtime / 20 + winc / 2;
                    int hard_time = wtime / 2;
                    auto now = std::chrono::steady_clock::now();
                    state.soft_deadline = now + std::chrono::milliseconds(soft_time);
                    state.hard_deadline = now + std::chrono::milliseconds(hard_time);
                } else if (position.side_to_move == Color::Black) {
                    int soft_time = btime / 20 + binc / 2;
                    int hard_time = btime / 2;
                    auto now = std::chrono::steady_clock::now();
                    state.soft_deadline = now + std::chrono::milliseconds(soft_time);
                    state.hard_deadline = now + std::chrono::milliseconds(hard_time);
                }
            }

            state.history = position_history;

            for (size_t i = 0; i < helper_states.size(); i++) {
                // Создаем своё состояние поиска для каждого потока
                prepare_helper_state(helper_states[i], state);

                helper_states[i].thread_id = static_cast<int>(i) + 1;

                // Даём каждому потоку копию позиции и ссылку на его state
                helper_threads.emplace_back(
                    [local_position = position, &helper = helper_states[i], max_depth]() mutable {
                        find_best_move(local_position, max_depth, helper);
                    });
            }

            search_thread = std::thread([&position, &state, max_depth, &helper_states]() {
                Position local_position = position;

                Move best_move = find_best_move(local_position, max_depth, state);

                for (SearchState& s : helper_states) {
                    s.stopped = true;
                }

                std::cout << "bestmove " + move_to_uci(best_move) + "\n";
                std::cout.flush();
            });
        }

        if (command == "bench") {
            int depth = 4;
            stream >> depth;
            run_bench(depth);
        }

        if (command == "setoption") {
            std::string token;

            join_helpers();
            while (stream >> token) {
                if (token == "Hash") {
                    // Сбрасываем слово "value"
                    stream >> token;

                    int size_mb;
                    stream >> size_mb;

                    resize_transposition_table(size_mb);
                } else if (token == "Threads") {
                    stream >> token;

                    stream >> thread_count;
                    helper_states = std::vector<SearchState>(thread_count - 1);
                }
            }
        }

        if (command == "stop") {
            state.stopped = true;
            if (search_thread.joinable()) {
                search_thread.join();
            }
            join_helpers();

            for (SearchState& s : helper_states) {
                s.stopped = true;
            }
        }

        if (command == "ucinewgame") {
            state.stopped = true;

            if (search_thread.joinable()) {
                search_thread.join();
            }
            join_helpers();

            state.nodes = 0;
            state.stopped = false;
            state.depth_reached = 0;
            state.history.clear();
            state.pv_table = {};
            state.pv_length = {};
            state.killers = {};
            state.history_heuristic = {};
            clear_transposition_table();
        }
    }

    state.stopped = true;
    if (search_thread.joinable()) {
        search_thread.join();
    }
    join_helpers();
}
