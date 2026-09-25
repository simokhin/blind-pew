#include "fen.h"

#include <cctype>
#include <sstream>

#include "zobrist.h"

Position parse_fen(const std::string& fen) {
    // Берем строку и превращаем её в поток ввода
    std::istringstream stream(fen);

    std::string piece_placement;
    std::string side_to_move_str;
    std::string castling_rights_str;
    std::string en_passant_str;
    std::string halfmove_clock_str;
    std::string fullmove_number_str;

    // Сохраняем разные части FEN-строки в переменные
    stream >> piece_placement;
    stream >> side_to_move_str;
    stream >> castling_rights_str;
    stream >> en_passant_str;
    stream >> halfmove_clock_str;
    stream >> fullmove_number_str;

    // Парсим сторону, которая ходит
    Color side_to_move;

    if (side_to_move_str == "w") {
        side_to_move = Color::White;
    } else if (side_to_move_str == "b") {
        side_to_move = Color::Black;
    }

    // Парсим права на рокироку
    uint8_t castling_rights = 0;

    for (char c : castling_rights_str) {
        switch (c) {
            case '-':
                break;
            case 'K':
                castling_rights |= WHITE_KINGSIDE;
                break;
            case 'Q':
                castling_rights |= WHITE_QUEENSIDE;
                break;
            case 'k':
                castling_rights |= BLACK_KINGSIDE;
                break;
            case 'q':
                castling_rights |= BLACK_QUEENSIDE;
                break;
            default:
                break;
        }
    }

    // Парсим клетку взятия на проходе
    int en_passant_target;

    if (en_passant_str == "-") {
        en_passant_target = -1;
    } else {
        en_passant_target = square_from_algebraic(en_passant_str);
    }

    // Парсим счетчики полуходов и ходов
    int halfmove_clock = std::stoi(halfmove_clock_str);
    int fullmove_number = std::stoi(fullmove_number_str);

    // Парсим расположение фигур
    std::istringstream ranks_stream(piece_placement);
    std::string rank_str;

    Board board{};

    int white_king_square;
    int black_king_square;

    // Читаем строку до символа '/'
    int rank = 7;
    while (std::getline(ranks_stream, rank_str, '/')) {
        int file = 0;

        for (char c : rank_str) {
            if (std::isdigit(c)) {
                file += c - '0';
            } else {
                switch (c) {
                    case 'p':
                        board[square_of(rank, file)] = Piece::BP;
                        break;
                    case 'P':
                        board[square_of(rank, file)] = Piece::WP;
                        break;
                    case 'r':
                        board[square_of(rank, file)] = Piece::BR;
                        break;
                    case 'R':
                        board[square_of(rank, file)] = Piece::WR;
                        break;
                    case 'n':
                        board[square_of(rank, file)] = Piece::BN;
                        break;
                    case 'N':
                        board[square_of(rank, file)] = Piece::WN;
                        break;
                    case 'b':
                        board[square_of(rank, file)] = Piece::BB;
                        break;
                    case 'B':
                        board[square_of(rank, file)] = Piece::WB;
                        break;
                    case 'k':
                        board[square_of(rank, file)] = Piece::BK;
                        black_king_square = square_of(rank, file);
                        break;
                    case 'K':
                        board[square_of(rank, file)] = Piece::WK;
                        white_king_square = square_of(rank, file);
                        break;
                    case 'q':
                        board[square_of(rank, file)] = Piece::BQ;
                        break;
                    case 'Q':
                        board[square_of(rank, file)] = Piece::WQ;
                        break;
                    default:
                        break;
                }
                file++;
            }
        }
        rank--;
    }

    Position position = {
        .board = board,
        .side_to_move = side_to_move,
        .castling_rights = castling_rights,
        .en_passant_target = en_passant_target,
        .halfmove_clock = halfmove_clock,
        .fullmove_number = fullmove_number,
        .white_king_square = white_king_square,
        .black_king_square = black_king_square,
    };

    position.zobrist_hash = compute_zobrist_hash(position);

    return position;
}
