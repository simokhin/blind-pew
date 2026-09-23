#include "movegen.h"

// Смещения коня
std::array<Offset, 8> offsets = {
    Offset{1, 2}, Offset{1, -2}, Offset{-1, 2}, Offset{-1, -2},
    Offset{2, 1}, Offset{2, -1}, Offset{-2, 1}, Offset{-2, -1}};

std::vector<Move> generate_knight_moves(const Board &board, int square)
{
    int rank = rank_of(square);
    int file = file_of(square);

    std::vector<Move> moves;

    Piece moving_piece = board[square];

    // Вычисляем new_rank и new_file для каждого смещения коня
    for (const Offset &o : offsets)
    {

        int new_rank = rank + o.dr;
        int new_file = file + o.df;

        if (is_valid_square(new_rank, new_file))
        {
            int to = square_of(new_rank, new_file);

            Piece target = board[to];

            if ((is_white(moving_piece) && is_white(target)) || (is_black(moving_piece) && is_black(target)))
            {
                continue;
            }

            Move move = {
                square,
                to,
            };

            moves.push_back(move);
        }
    };

    return moves;
};