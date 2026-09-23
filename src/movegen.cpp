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

std::vector<Offset> rook_directions = {
    Offset{1, 0},
    Offset{-1, 0},
    Offset{0, 1},
    Offset{0, -1},
};

std::vector<Offset> bishop_directions = {
    Offset{1, 1},
    Offset{1, -1},
    Offset{-1, 1},
    Offset{-1, -1},
};

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
}

std::vector<Move> generate_knight_moves(const Board &board, int square)
{
    return generate_leaper_moves(board, square, knight_offsets);
}

std::vector<Move> generate_king_moves(const Board &board, int square)
{
    return generate_leaper_moves(board, square, king_offsets);
}

std::vector<Move> generate_slider_moves(const Board &board, int square, const std::vector<Offset> &directions)
{
    int rank = rank_of(square);
    int file = file_of(square);

    std::vector<Move> moves;

    Piece moving_piece = board[square];

    for (const Offset &d : directions)
    {
        int new_rank = rank + d.dr;
        int new_file = file + d.df;

        while (true)
        {

            if (is_valid_square(new_rank, new_file))
            {
                int to = square_of(new_rank, new_file);

                Piece target = board[to];

                if (color_of(target) == color_of(moving_piece))
                {
                    // Останавливаемся, если на пути своя фигура
                    break;
                }
                else if (target != Piece::None)
                {
                    // Если на пути чужая фигура, сохраняем взятие в массив ходов и останавливаемся
                    moves.push_back(Move{square, to});
                    break;
                }
                else
                {
                    // На пути нет фигуры, сохраняем ход и двигаемся дальше
                    moves.push_back(Move{square, to});
                    new_rank += d.dr;
                    new_file += d.df;
                }
            }
            else
            {
                break;
            }
        }
    };

    return moves;
}

std::vector<Move> generate_rook_moves(const Board &board, int square)
{
    return generate_slider_moves(board, square, rook_directions);
}

std::vector<Move> generate_bishop_moves(const Board &board, int square)
{
    return generate_slider_moves(board, square, bishop_directions);
}

std::vector<Move> generate_queen_moves(const Board &board, int square)
{
    std::vector<Move> moves = generate_rook_moves(board, square);
    std::vector<Move> bishop_moves = generate_bishop_moves(board, square);

    moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());

    return moves;
}