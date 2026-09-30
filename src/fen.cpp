#include "fen.h"

#include <cctype>
#include <sstream>

#include "evaluate.h"
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

    Position position{};

    position.side_to_move = side_to_move;
    position.castling_rights = castling_rights;
    position.en_passant_target = en_passant_target;
    position.halfmove_clock = halfmove_clock;
    position.fullmove_number = fullmove_number;

    // Парсим расположение фигур
    std::istringstream ranks_stream(piece_placement);
    std::string rank_str;

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
                        put_piece(position, Piece::BP, square_of(rank, file));
                        break;
                    case 'P':
                        put_piece(position, Piece::WP, square_of(rank, file));
                        break;
                    case 'r':
                        put_piece(position, Piece::BR, square_of(rank, file));
                        break;
                    case 'R':
                        put_piece(position, Piece::WR, square_of(rank, file));
                        break;
                    case 'n':
                        put_piece(position, Piece::BN, square_of(rank, file));
                        break;
                    case 'N':
                        put_piece(position, Piece::WN, square_of(rank, file));
                        break;
                    case 'b':
                        put_piece(position, Piece::BB, square_of(rank, file));
                        break;
                    case 'B':
                        put_piece(position, Piece::WB, square_of(rank, file));
                        break;
                    case 'k':
                        put_piece(position, Piece::BK, square_of(rank, file));
                        position.black_king_square = square_of(rank, file);
                        break;
                    case 'K':
                        put_piece(position, Piece::WK, square_of(rank, file));
                        position.white_king_square = square_of(rank, file);
                        break;
                    case 'q':
                        put_piece(position, Piece::BQ, square_of(rank, file));
                        break;
                    case 'Q':
                        put_piece(position, Piece::WQ, square_of(rank, file));
                        break;
                    default:
                        break;
                }
                file++;
            }
        }
        rank--;
    }

    position.zobrist_hash = compute_zobrist_hash(position);

    refresh_accumulator(position, Color::White, position.accumulators[0]);
    refresh_accumulator(position, Color::Black, position.accumulators[1]);

    return position;
}
