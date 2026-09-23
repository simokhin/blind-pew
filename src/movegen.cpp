#include "movegen.h"

std::vector<Offset> knight_offsets = {
    Offset{1, 2},
    Offset{1, -2},
    Offset{-1, 2},
    Offset{-1, -2},
    Offset{2, 1},
    Offset{2, -1},
    Offset{-2, 1},
    Offset{-2, -1},
};

std::vector<Offset> king_offsets = {
    Offset{1, 0},
    Offset{1, 1},
    Offset{1, -1},
    Offset{-1, 0},
    Offset{-1, 1},
    Offset{-1, -1},
    Offset{0, 1},
    Offset{0, -1},
};

// Генерация ходов для фигур-липеров
std::vector<Move> generate_leaper_moves(const Board &board, int square, const std::vector<Offset> &offsets)
{
    int rank = rank_of(square);
    int file = file_of(square);

    std::vector<Move> moves;

    Piece moving_piece = board[square];

    // Вычисляем new_rank и new_file для каждого смещения фигуры
    for (const Offset &o : offsets)
    {

        int new_rank = rank + o.dr;
        int new_file = file + o.df;

        if (is_valid_square(new_rank, new_file))
        {
            int to = square_of(new_rank, new_file);

            Piece target = board[to];

            if (color_of(moving_piece) == color_of(target))
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

std::vector<Move> generate_knight_moves(const Board &board, int square)
{
    return generate_leaper_moves(board, square, knight_offsets);
};

std::vector<Move> generate_king_moves(const Board &board, int square)
{
    return generate_leaper_moves(board, square, king_offsets);
};