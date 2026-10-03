#include "board.h"

#include <array>
#include <iostream>

PieceType piece_type_of(Piece piece) {
    if (piece == Piece::BP || piece == Piece::WP) {
        return PieceType::Pawn;
    } else if (piece == Piece::BN || piece == Piece::WN) {
        return PieceType::Knight;
    } else if (piece == Piece::BB || piece == Piece::WB) {
        return PieceType::Bishop;
    } else if (piece == Piece::BR || piece == Piece::WR) {
        return PieceType::Rook;
    } else if (piece == Piece::BQ || piece == Piece::WQ) {
        return PieceType::Queen;
    } else if (piece == Piece::BK || piece == Piece::WK) {
        return PieceType::King;
    } else {
        return PieceType::None;
    }
}

// Определить цвет фигуры
Color color_of(Piece piece) {
    int value = static_cast<int>(piece);

    if (value == 0) return Color::None;
    if (value <= 6) return Color::White;
    return Color::Black;
}

void print_board(const Board& board) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file <= 7; file++) {
            int index = rank * 8 + file;

            const Piece& p = board[index];

            switch (p) {
                case Piece::WP:
                    std::cout << 'P';
                    break;
                case Piece::WR:
                    std::cout << 'R';
                    break;
                case Piece::WN:
                    std::cout << 'N';
                    break;
                case Piece::WB:
                    std::cout << 'B';
                    break;
                case Piece::WK:
                    std::cout << 'K';
                    break;
                case Piece::WQ:
                    std::cout << 'Q';
                    break;
                case Piece::BP:
                    std::cout << 'p';
                    break;
                case Piece::BR:
                    std::cout << 'r';
                    break;
                case Piece::BN:
                    std::cout << 'n';
                    break;
                case Piece::BB:
                    std::cout << 'b';
                    break;
                case Piece::BK:
                    std::cout << 'k';
                    break;
                case Piece::BQ:
                    std::cout << 'q';
                    break;
                default:
                    std::cout << '.';
                    break;
            }
        }
        std::cout << '\n';
    }
}

int mirror_square(int square) { return square_of(7 - rank_of(square), file_of(square)); };

Color opposite_color(Color color) { return (color == Color::White) ? Color::Black : Color::White; }

int square_from_algebraic(const std::string& s) {
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    return square_of(rank, file);
}

std::string algebraic_from_square(int square) {
    char file = file_of(square) + 'a';
    char rank = rank_of(square) + '1';

    std::string move;
    move += file;
    move += rank;

    return move;
}
