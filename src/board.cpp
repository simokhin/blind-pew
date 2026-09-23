#include <array>
#include <iostream>
#include "board.h"

// Расстановка стартовой позиции
Board make_start_position()
{
    // Инициализируем массив, который заполняется 0-ми, что соответствует Piece None
    Board board{};

    // Расставляем стартовую позицию для белых
    board[0] = Piece::WR;
    board[1] = Piece::WN;
    board[2] = Piece::WB;
    board[3] = Piece::WQ;
    board[4] = Piece::WK;
    board[5] = Piece::WB;
    board[6] = Piece::WN;
    board[7] = Piece::WR;

    for (int i = 8; i <= 15; i++)
    {
        board[i] = Piece::WP;
    }

    // Для черных
    board[56] = Piece::BR;
    board[57] = Piece::BN;
    board[58] = Piece::BB;
    board[59] = Piece::BQ;
    board[60] = Piece::BK;
    board[61] = Piece::BB;
    board[62] = Piece::BN;
    board[63] = Piece::BR;

    for (int i = 48; i <= 55; i++)
    {
        board[i] = Piece::BP;
    }

    return board;
}

void print_board(const Board &board)
{
    for (int rank = 7; rank >= 0; rank--)
    {
        for (int file = 0; file <= 7; file++)
        {
            int index = rank * 8 + file;

            const Piece &p = board[index];

            switch (p)
            {
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

int rank_of(int square)
{
    return square / 8;
};

int file_of(int square)
{
    return square % 8;
};

int square_of(int rank, int file)
{
    return rank * 8 + file;
};

bool is_valid_square(int rank, int file)
{
    return rank >= 0 && rank <= 7 && file >= 0 && file <= 7;
}

bool is_white(Piece piece)
{
    int value = static_cast<int>(piece);
    return value >= 1 && value <= 6;
};

bool is_black(Piece piece)
{
    int value = static_cast<int>(piece);
    return value >= 7 && value <= 12;
}