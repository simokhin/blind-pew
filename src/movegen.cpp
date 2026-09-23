#include "movegen.h"

// Смещения коня
std::array<Offset, 8> offsets = {
    Offset{1, 2}, Offset{1, -2}, Offset{-1, 2}, Offset{-1, -2},
    Offset{2, 1}, Offset{2, -1}, Offset{-2, 1}, Offset{-2, -1}};

std::vector<Move> generate_knight_moves(const Board &board, int square)
{
    int rank = square / 8;
    int file = square % 8;

    std::vector<Move> moves;

    // Вычисляем new_rank и new_file для каждого смещения коня
    for (const Offset &o : offsets)
    {
        int new_rank = rank + o.dr;
        int new_file = file + o.df;

        if (new_rank >= 0 && new_rank <= 7 && new_file >= 0 && new_file <= 7)
        {
            int to = new_rank * 8 + new_file;

            Move move = {
                square,
                to,
            };

            moves.push_back(move);
        }
    };

    return moves;
};