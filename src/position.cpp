#include "position.h"

// Расстановка стартовой позиции
Position make_start_position()
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

    Position position = {
        .board = board,
        .side_to_move = Color::White,
        .castling_rights = WHITE_KINGSIDE | WHITE_QUEENSIDE | BLACK_KINGSIDE | BLACK_QUEENSIDE,
        .en_passant_target = -1,
        .halfmove_clock = 0,
        .fullmove_number = 1,
    };

    return position;
}