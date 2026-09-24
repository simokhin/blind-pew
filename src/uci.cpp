#include "uci.h"

#include <iostream>
#include <sstream>
#include <string>

#include "constants.h"
#include "fen.h"
#include "movegen.h"
#include "position.h"

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
    }
}