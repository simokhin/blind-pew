#include "movegen.h"

#include "move.h"

std::vector<Offset> knight_offsets = {
    Offset{1, 2}, Offset{1, -2}, Offset{-1, 2}, Offset{-1, -2},
    Offset{2, 1}, Offset{2, -1}, Offset{-2, 1}, Offset{-2, -1},
};

std::vector<Offset> king_offsets = {
    Offset{1, 0},  Offset{1, 1},   Offset{1, -1}, Offset{-1, 0},
    Offset{-1, 1}, Offset{-1, -1}, Offset{0, 1},  Offset{0, -1},
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

std::array<PromotionPiece, 4> promotion_pieces = {
    PromotionPiece::Knight,
    PromotionPiece::Bishop,
    PromotionPiece::Rook,
    PromotionPiece::Queen,
};

std::vector<Move> generate_leaper_moves(const Board& board, int square,
                                        const std::vector<Offset>& offsets) {
    int rank = rank_of(square);
    int file = file_of(square);

    std::vector<Move> moves;

    Piece moving_piece = board[square];

    // Вычисляем new_rank и new_file для каждого смещения фигуры
    for (const Offset& o : offsets) {
        int new_rank = rank + o.dr;
        int new_file = file + o.df;

        if (is_valid_square(new_rank, new_file)) {
            int to = square_of(new_rank, new_file);

            Piece target = board[to];

            if (color_of(moving_piece) == color_of(target)) {
                continue;
            }

            Move move = Move(square, to);
            moves.push_back(move);
        }
    };

    return moves;
}

std::vector<Move> generate_knight_moves(const Board& board, int square) {
    return generate_leaper_moves(board, square, knight_offsets);
}

std::vector<Move> generate_king_moves(const Board& board, int square) {
    return generate_leaper_moves(board, square, king_offsets);
}

std::vector<Move> generate_slider_moves(const Board& board, int square,
                                        const std::vector<Offset>& directions) {
    int rank = rank_of(square);
    int file = file_of(square);

    std::vector<Move> moves;

    Piece moving_piece = board[square];

    for (const Offset& d : directions) {
        int new_rank = rank + d.dr;
        int new_file = file + d.df;

        while (true) {
            if (is_valid_square(new_rank, new_file)) {
                int to = square_of(new_rank, new_file);

                Piece target = board[to];

                if (color_of(target) == color_of(moving_piece)) {
                    // Останавливаемся, если на пути своя фигура
                    break;
                } else if (target != Piece::None) {
                    // Если на пути чужая фигура, сохраняем взятие в массив ходов и останавливаемся
                    moves.push_back(Move(square, to));
                    break;
                } else {
                    // На пути нет фигуры, сохраняем ход и двигаемся дальше
                    moves.push_back(Move(square, to));
                    new_rank += d.dr;
                    new_file += d.df;
                }
            } else {
                break;
            }
        }
    };

    return moves;
}

std::vector<Move> generate_rook_moves(const Board& board, int square) {
    return generate_slider_moves(board, square, rook_directions);
}

std::vector<Move> generate_bishop_moves(const Board& board, int square) {
    return generate_slider_moves(board, square, bishop_directions);
}

std::vector<Move> generate_queen_moves(const Board& board, int square) {
    std::vector<Move> moves = generate_rook_moves(board, square);
    std::vector<Move> bishop_moves = generate_bishop_moves(board, square);

    moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());

    return moves;
}

std::vector<Move> generate_pawn_moves(const Position& position, int square) {
    const Board& board = position.board;

    int rank = rank_of(square);
    int file = file_of(square);

    std::vector<Move> moves;

    Piece moving_piece = board[square];

    int direction = (color_of(moving_piece) == Color::White) ? 1 : -1;

    int promotion_rank = (direction == 1) ? 7 : 0;
    int start_rank = (direction == 1) ? 1 : 6;

    int new_rank = rank + direction;

    int to = square_of(new_rank, file);

    Piece target = board[to];

    // Создаем ход-превращение
    if (new_rank == promotion_rank && target == Piece::None) {
        for (PromotionPiece p : promotion_pieces) {
            moves.push_back(Move(square, to, MoveFlag::Promotion, p));
        }
    } else if (target == Piece::None && is_valid_square(new_rank, file)) {
        // Создаем ход на одну клетку вперед
        moves.push_back(Move(square, to));
    }

    // Проверяем, идёт ли пешка со стартовой клетки
    if (rank == start_rank && target == Piece::None) {
        // Проверяем, стоит ли фигура впереди через две клетки
        int double_pawn_move_rank = new_rank + direction;

        int double_pawn_move_to = square_of(double_pawn_move_rank, file);
        Piece target = board[double_pawn_move_to];

        if (target == Piece::None) {
            moves.push_back(Move(square, double_pawn_move_to));
        }
    }

    // Добавляем взятия
    for (int df : {-1, 1}) {
        int capture_rank = rank + direction;
        int capture_file = file + df;

        if (is_valid_square(capture_rank, capture_file)) {
            int to = square_of(capture_rank, capture_file);

            // Добавляем взятия на проходе
            if (to == position.en_passant_target) {
                moves.push_back(Move(square, to, MoveFlag::EnPassant));
            }

            Piece target = board[to];

            if (target != Piece::None && color_of(target) != color_of(moving_piece)) {
                if (capture_rank == promotion_rank) {
                    for (PromotionPiece p : promotion_pieces) {
                        moves.push_back(Move(square, to, MoveFlag::Promotion, p));
                    }
                } else {
                    moves.push_back(Move(square, to));
                }
            }
        }
    }

    return moves;
}

std::vector<Move> generate_castling_moves(const Position& position) {
    std::vector<Move> moves;

    if (position.side_to_move == Color::White) {
        if (position.castling_rights & WHITE_QUEENSIDE) {
            if ((position.board[static_cast<int>(Square::B1)] == Piece::None) &&
                (position.board[static_cast<int>(Square::C1)] == Piece::None) &&
                (position.board[static_cast<int>(Square::D1)] == Piece::None)) {
                moves.push_back(Move(static_cast<int>(Square::E1), static_cast<int>(Square::C1),
                                     MoveFlag::Castling));
            }
        }
        if (position.castling_rights & WHITE_KINGSIDE) {
            if ((position.board[static_cast<int>(Square::F1)] == Piece::None) &&
                (position.board[static_cast<int>(Square::G1)] == Piece::None)) {
                moves.push_back(Move(static_cast<int>(Square::E1), static_cast<int>(Square::G1),
                                     MoveFlag::Castling));
            }
        }
    } else {
        if (position.castling_rights & BLACK_QUEENSIDE) {
            if ((position.board[static_cast<int>(Square::B8)] == Piece::None) &&
                (position.board[static_cast<int>(Square::C8)] == Piece::None) &&
                (position.board[static_cast<int>(Square::D8)] == Piece::None)) {
                moves.push_back(Move(static_cast<int>(Square::E8), static_cast<int>(Square::C8),
                                     MoveFlag::Castling));
            }
        }
        if (position.castling_rights & BLACK_KINGSIDE) {
            if ((position.board[static_cast<int>(Square::F8)] == Piece::None) &&
                (position.board[static_cast<int>(Square::G8)] == Piece::None)) {
                moves.push_back(Move(static_cast<int>(Square::E8), static_cast<int>(Square::G8),
                                     MoveFlag::Castling));
            }
        }
    }

    return moves;
}
