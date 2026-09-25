#include "movegen.h"

#include "board.h"
#include "move.h"

std::array<PromotionPiece, 4> promotion_pieces = {
    PromotionPiece::Knight,
    PromotionPiece::Bishop,
    PromotionPiece::Rook,
    PromotionPiece::Queen,
};

MoveList generate_leaper_moves(const Board& board, int square, const std::vector<Offset>& offsets) {
    int rank = rank_of(square);
    int file = file_of(square);

    MoveList moves;

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
            moves.add(move);
        }
    };

    return moves;
}

MoveList generate_knight_moves(const Board& board, int square) {
    return generate_leaper_moves(board, square, knight_offsets);
}

MoveList generate_king_moves(const Board& board, int square) {
    return generate_leaper_moves(board, square, king_offsets);
}

MoveList generate_slider_moves(const Board& board, int square,
                               const std::vector<Offset>& directions) {
    int rank = rank_of(square);
    int file = file_of(square);

    MoveList moves;

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
                    moves.add(Move(square, to));
                    break;
                } else {
                    // На пути нет фигуры, сохраняем ход и двигаемся дальше
                    moves.add(Move(square, to));
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

MoveList generate_rook_moves(const Board& board, int square) {
    return generate_slider_moves(board, square, rook_directions);
}

MoveList generate_bishop_moves(const Board& board, int square) {
    return generate_slider_moves(board, square, bishop_directions);
}

MoveList generate_queen_moves(const Board& board, int square) {
    MoveList moves = generate_rook_moves(board, square);
    MoveList bishop_moves = generate_bishop_moves(board, square);

    for (const Move& m : bishop_moves) {
        moves.add(m);
    }

    return moves;
}

MoveList generate_pawn_moves(const Position& position, int square) {
    const Board& board = position.board;

    int rank = rank_of(square);
    int file = file_of(square);

    MoveList moves;

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
            moves.add(Move(square, to, MoveFlag::Promotion, p));
        }
    } else if (target == Piece::None && is_valid_square(new_rank, file)) {
        // Создаем ход на одну клетку вперед
        moves.add(Move(square, to));
    }

    // Проверяем, идёт ли пешка со стартовой клетки
    if (rank == start_rank && target == Piece::None) {
        // Проверяем, стоит ли фигура впереди через две клетки
        int double_pawn_move_rank = new_rank + direction;

        int double_pawn_move_to = square_of(double_pawn_move_rank, file);
        Piece target = board[double_pawn_move_to];

        if (target == Piece::None) {
            moves.add(Move(square, double_pawn_move_to));
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
                moves.add(Move(square, to, MoveFlag::EnPassant));
            }

            Piece target = board[to];

            if (target != Piece::None && color_of(target) != color_of(moving_piece)) {
                if (capture_rank == promotion_rank) {
                    for (PromotionPiece p : promotion_pieces) {
                        moves.add(Move(square, to, MoveFlag::Promotion, p));
                    }
                } else {
                    moves.add(Move(square, to));
                }
            }
        }
    }

    return moves;
}

MoveList generate_castling_moves(const Position& position) {
    MoveList moves;

    if (position.side_to_move == Color::White) {
        if (position.castling_rights & WHITE_QUEENSIDE) {
            if ((position.board[static_cast<int>(Square::B1)] == Piece::None) &&
                (position.board[static_cast<int>(Square::C1)] == Piece::None) &&
                (position.board[static_cast<int>(Square::D1)] == Piece::None) &&
                !is_square_attacked(position, static_cast<int>(Square::E1), Color::Black) &&
                !is_square_attacked(position, static_cast<int>(Square::D1), Color::Black) &&
                !is_square_attacked(position, static_cast<int>(Square::C1), Color::Black)) {
                moves.add(Move(static_cast<int>(Square::E1), static_cast<int>(Square::C1),
                               MoveFlag::Castling));
            }
        }
        if (position.castling_rights & WHITE_KINGSIDE) {
            if ((position.board[static_cast<int>(Square::F1)] == Piece::None) &&
                (position.board[static_cast<int>(Square::G1)] == Piece::None) &&
                !is_square_attacked(position, static_cast<int>(Square::E1), Color::Black) &&
                !is_square_attacked(position, static_cast<int>(Square::F1), Color::Black) &&
                !is_square_attacked(position, static_cast<int>(Square::G1), Color::Black)) {
                moves.add(Move(static_cast<int>(Square::E1), static_cast<int>(Square::G1),
                               MoveFlag::Castling));
            }
        }
    } else {
        if (position.castling_rights & BLACK_QUEENSIDE) {
            if ((position.board[static_cast<int>(Square::B8)] == Piece::None) &&
                (position.board[static_cast<int>(Square::C8)] == Piece::None) &&
                (position.board[static_cast<int>(Square::D8)] == Piece::None) &&
                !is_square_attacked(position, static_cast<int>(Square::E8), Color::White) &&
                !is_square_attacked(position, static_cast<int>(Square::D8), Color::White) &&
                !is_square_attacked(position, static_cast<int>(Square::C8), Color::White)) {
                moves.add(Move(static_cast<int>(Square::E8), static_cast<int>(Square::C8),
                               MoveFlag::Castling));
            }
        }
        if (position.castling_rights & BLACK_KINGSIDE) {
            if ((position.board[static_cast<int>(Square::F8)] == Piece::None) &&
                (position.board[static_cast<int>(Square::G8)] == Piece::None) &&
                !is_square_attacked(position, static_cast<int>(Square::E8), Color::White) &&
                !is_square_attacked(position, static_cast<int>(Square::F8), Color::White) &&
                !is_square_attacked(position, static_cast<int>(Square::G8), Color::White)) {
                moves.add(Move(static_cast<int>(Square::E8), static_cast<int>(Square::G8),
                               MoveFlag::Castling));
            }
        }
    }

    return moves;
}

bool is_square_attacked(const Position& position, int square, Color by_color) {
    int rank = rank_of(square);
    int file = file_of(square);

    // Атакована ли клетка вражескими пешками
    for (int df : {-1, 1}) {
        int direction = (by_color == Color::White) ? 1 : -1;

        int enemy_pawn_rank = rank - direction;
        int enemy_pawn_file = file + df;

        int enemy_pawn_square = square_of(enemy_pawn_rank, enemy_pawn_file);

        if (is_valid_square(enemy_pawn_rank, enemy_pawn_file)) {
            Piece piece = position.board[enemy_pawn_square];

            if (by_color == Color::White && piece == Piece::WP) {
                return true;
            } else if (by_color == Color::Black && piece == Piece::BP) {
                return true;
            }
        }
    }

    // Атакована ли клетка вражескими конями
    for (const Offset& o : knight_offsets) {
        int new_rank = rank + o.dr;
        int new_file = file + o.df;

        if (is_valid_square(new_rank, new_file)) {
            int square_to_check = square_of(new_rank, new_file);

            Piece piece = position.board[square_to_check];

            if (by_color == Color::White && piece == Piece::WN) {
                return true;
            } else if (by_color == Color::Black && piece == Piece::BN) {
                return true;
            }
        }
    };

    // Атакована ли пешка вражеским королем
    for (const Offset& o : king_offsets) {
        int new_rank = rank + o.dr;
        int new_file = file + o.df;

        if (is_valid_square(new_rank, new_file)) {
            int square_to_check = square_of(new_rank, new_file);

            Piece piece = position.board[square_to_check];

            if (by_color == Color::White && piece == Piece::WK) {
                return true;
            } else if (by_color == Color::Black && piece == Piece::BK) {
                return true;
            }
        }
    };

    // Атакована ли клетка вражескими слайдерами
    for (const Offset& d : rook_directions) {
        int new_rank = rank + d.dr;
        int new_file = file + d.df;

        while (true) {
            if (is_valid_square(new_rank, new_file)) {
                int square_to_check = square_of(new_rank, new_file);

                Piece piece = position.board[square_to_check];

                if ((by_color == Color::White && piece == Piece::WR) ||
                    (by_color == Color::White && piece == Piece::WQ)) {
                    return true;
                } else if ((by_color == Color::Black && piece == Piece::BR) ||
                           (by_color == Color::Black && piece == Piece::BQ)) {
                    return true;
                }

                if (is_valid_square(new_rank, new_file) && piece == Piece::None) {
                    new_rank += d.dr;
                    new_file += d.df;
                } else {
                    break;
                }

            } else {
                break;
            }
        }
    };

    for (const Offset& d : bishop_directions) {
        int new_rank = rank + d.dr;
        int new_file = file + d.df;

        while (true) {
            if (is_valid_square(new_rank, new_file)) {
                int square_to_check = square_of(new_rank, new_file);

                Piece piece = position.board[square_to_check];

                if ((by_color == Color::White && piece == Piece::WB) ||
                    (by_color == Color::White && piece == Piece::WQ)) {
                    return true;
                } else if ((by_color == Color::Black && piece == Piece::BB) ||
                           (by_color == Color::Black && piece == Piece::BQ)) {
                    return true;
                }

                if (is_valid_square(new_rank, new_file) && piece == Piece::None) {
                    new_rank += d.dr;
                    new_file += d.df;
                } else {
                    break;
                }

            } else {
                break;
            }
        }
    };

    return false;
}

MoveList generate_pseudo_legal_moves(const Position& position) {
    MoveList moves;

    for (int square = 0; square < 64; square++) {
        Piece piece = position.board[square];

        if (piece == Piece::None || color_of(piece) != position.side_to_move) {
            continue;
        }

        switch (piece) {
            case Piece::WP:
            case Piece::BP: {
                MoveList pawn_moves = generate_pawn_moves(position, square);
                for (const Move& m : pawn_moves) {
                    moves.add(m);
                }
                break;
            }
            case Piece::WN:
            case Piece::BN: {
                MoveList knight_moves = generate_knight_moves(position.board, square);
                for (const Move& m : knight_moves) {
                    moves.add(m);
                }
                break;
            }
            case Piece::WB:
            case Piece::BB: {
                MoveList bishop_moves = generate_bishop_moves(position.board, square);
                for (const Move& m : bishop_moves) {
                    moves.add(m);
                }
                break;
            }
            case Piece::WR:
            case Piece::BR: {
                MoveList rook_moves = generate_rook_moves(position.board, square);
                for (const Move& m : rook_moves) {
                    moves.add(m);
                }
                break;
            }
            case Piece::WQ:
            case Piece::BQ: {
                MoveList queen_moves = generate_queen_moves(position.board, square);
                for (const Move& m : queen_moves) {
                    moves.add(m);
                }
                break;
            }
            case Piece::WK:
            case Piece::BK: {
                MoveList king_moves = generate_king_moves(position.board, square);
                for (const Move& m : king_moves) {
                    moves.add(m);
                }
                break;
            }
            default:
                break;
        }
    }

    MoveList castling_moves = generate_castling_moves(position);
    for (const Move& m : castling_moves) {
        moves.add(m);
    }

    return moves;
}

MoveList generate_legal_moves(const Position& position) {
    MoveList pseudo_legal_moves = generate_pseudo_legal_moves(position);
    MoveList legal_moves;

    for (const Move& m : pseudo_legal_moves) {
        Position new_position = position;
        Color mover = position.side_to_move;

        make_move(new_position, m);

        int king_square =
            mover == Color::White ? new_position.white_king_square : new_position.black_king_square;

        if (!is_square_attacked(new_position, king_square, new_position.side_to_move)) {
            legal_moves.add(m);
        }
    }

    return legal_moves;
}

MoveList generate_capture_moves(const Position& position) {
    MoveList moves;

    for (int square = 0; square < 64; square++) {
        Piece piece = position.board[square];

        if (piece == Piece::None || color_of(piece) != position.side_to_move) {
            continue;
        }

        switch (piece) {
            case Piece::WP:
            case Piece::BP: {
                MoveList pawn_moves = generate_pawn_moves(position, square);
                for (const Move& m : pawn_moves) {
                    if (position.board[m.to()] != Piece::None || m.flag() == MoveFlag::EnPassant) {
                        moves.add(m);
                    }
                }
                break;
            }
            case Piece::WN:
            case Piece::BN: {
                MoveList knight_moves = generate_knight_moves(position.board, square);
                for (const Move& m : knight_moves) {
                    if (position.board[m.to()] != Piece::None) {
                        moves.add(m);
                    }
                }
                break;
            }
            case Piece::WB:
            case Piece::BB: {
                MoveList bishop_moves = generate_bishop_moves(position.board, square);
                for (const Move& m : bishop_moves) {
                    if (position.board[m.to()] != Piece::None) {
                        moves.add(m);
                    }
                }
                break;
            }
            case Piece::WR:
            case Piece::BR: {
                MoveList rook_moves = generate_rook_moves(position.board, square);
                for (const Move& m : rook_moves) {
                    if (position.board[m.to()] != Piece::None) {
                        moves.add(m);
                    }
                }
                break;
            }
            case Piece::WQ:
            case Piece::BQ: {
                MoveList queen_moves = generate_queen_moves(position.board, square);
                for (const Move& m : queen_moves) {
                    if (position.board[m.to()] != Piece::None) {
                        moves.add(m);
                    }
                }
                break;
            }
            case Piece::WK:
            case Piece::BK: {
                MoveList king_moves = generate_king_moves(position.board, square);
                for (const Move& m : king_moves) {
                    if (position.board[m.to()] != Piece::None) {
                        moves.add(m);
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return moves;
}

void MoveList::add(const Move& move) {
    moves[count] = move;
    count++;
}
int MoveList::size() const { return count; }

const Move& MoveList::operator[](int index) const { return moves[index]; }
const Move* MoveList::begin() const { return moves.data(); };
const Move* MoveList::end() const { return moves.data() + count; };
Move* MoveList::begin() { return moves.data(); };
Move* MoveList::end() { return moves.data() + count; };
