#include "movegen.h"

#include "bitboard.h"
#include "board.h"
#include "magic_constants.h"
#include "move.h"

std::array<PromotionPiece, 4> promotion_pieces = {
    PromotionPiece::Knight,
    PromotionPiece::Bishop,
    PromotionPiece::Rook,
    PromotionPiece::Queen,
};

void generate_castling_moves(const Position& position, MoveList& moves) {
    if (position.side_to_move == Color::White) {
        if (position.castling_rights & WHITE_QUEENSIDE) {
            if ((position.board[static_cast<int>(Square::B1)] == Piece::None) &&
                (position.board[static_cast<int>(Square::C1)] == Piece::None) &&
                (position.board[static_cast<int>(Square::D1)] == Piece::None) &&
                !is_square_attacked_bb(position, static_cast<int>(Square::E1), Color::Black) &&
                !is_square_attacked_bb(position, static_cast<int>(Square::D1), Color::Black) &&
                !is_square_attacked_bb(position, static_cast<int>(Square::C1), Color::Black)) {
                moves.add(Move(static_cast<int>(Square::E1), static_cast<int>(Square::C1),
                               MoveFlag::Castling));
            }
        }
        if (position.castling_rights & WHITE_KINGSIDE) {
            if ((position.board[static_cast<int>(Square::F1)] == Piece::None) &&
                (position.board[static_cast<int>(Square::G1)] == Piece::None) &&
                !is_square_attacked_bb(position, static_cast<int>(Square::E1), Color::Black) &&
                !is_square_attacked_bb(position, static_cast<int>(Square::F1), Color::Black) &&
                !is_square_attacked_bb(position, static_cast<int>(Square::G1), Color::Black)) {
                moves.add(Move(static_cast<int>(Square::E1), static_cast<int>(Square::G1),
                               MoveFlag::Castling));
            }
        }
    } else {
        if (position.castling_rights & BLACK_QUEENSIDE) {
            if ((position.board[static_cast<int>(Square::B8)] == Piece::None) &&
                (position.board[static_cast<int>(Square::C8)] == Piece::None) &&
                (position.board[static_cast<int>(Square::D8)] == Piece::None) &&
                !is_square_attacked_bb(position, static_cast<int>(Square::E8), Color::White) &&
                !is_square_attacked_bb(position, static_cast<int>(Square::D8), Color::White) &&
                !is_square_attacked_bb(position, static_cast<int>(Square::C8), Color::White)) {
                moves.add(Move(static_cast<int>(Square::E8), static_cast<int>(Square::C8),
                               MoveFlag::Castling));
            }
        }
        if (position.castling_rights & BLACK_KINGSIDE) {
            if ((position.board[static_cast<int>(Square::F8)] == Piece::None) &&
                (position.board[static_cast<int>(Square::G8)] == Piece::None) &&
                !is_square_attacked_bb(position, static_cast<int>(Square::E8), Color::White) &&
                !is_square_attacked_bb(position, static_cast<int>(Square::F8), Color::White) &&
                !is_square_attacked_bb(position, static_cast<int>(Square::G8), Color::White)) {
                moves.add(Move(static_cast<int>(Square::E8), static_cast<int>(Square::G8),
                               MoveFlag::Castling));
            }
        }
    }
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
                generate_pawn_moves_bb(position, square, moves);
                break;
            }
            case Piece::WN:
            case Piece::BN: {
                generate_knight_moves_bb(position, square, moves);
                break;
            }
            case Piece::WB:
            case Piece::BB: {
                generate_bishop_moves_bb(position, square, moves);
                break;
            }
            case Piece::WR:
            case Piece::BR: {
                generate_rook_moves_bb(position, square, moves);
                break;
            }
            case Piece::WQ:
            case Piece::BQ: {
                generate_queen_moves_bb(position, square, moves);
                break;
            }
            case Piece::WK:
            case Piece::BK: {
                generate_king_moves_bb(position, square, moves);
                break;
            }
            default:
                break;
        }
    }

    generate_castling_moves(position, moves);

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

        if (!is_square_attacked_bb(new_position, king_square, new_position.side_to_move)) {
            legal_moves.add(m);
        }
    }

    return legal_moves;
}

MoveList generate_capture_moves(const Position& position) {
    MoveList moves;
    MoveList scratch;

    for (int square = 0; square < 64; square++) {
        Piece piece = position.board[square];

        if (piece == Piece::None || color_of(piece) != position.side_to_move) {
            continue;
        }

        switch (piece) {
            case Piece::WP:
            case Piece::BP: {
                scratch.clear();
                generate_pawn_moves_bb(position, square, scratch);
                for (const Move& m : scratch) {
                    if (position.board[m.to()] != Piece::None || m.flag() == MoveFlag::EnPassant) {
                        moves.add(m);
                    }
                }
                break;
            }
            case Piece::WN:
            case Piece::BN: {
                scratch.clear();
                generate_knight_moves_bb(position, square, scratch);
                for (const Move& m : scratch) {
                    if (position.board[m.to()] != Piece::None) {
                        moves.add(m);
                    }
                }
                break;
            }
            case Piece::WB:
            case Piece::BB: {
                scratch.clear();
                generate_bishop_moves_bb(position, square, scratch);
                for (const Move& m : scratch) {
                    if (position.board[m.to()] != Piece::None) {
                        moves.add(m);
                    }
                }
                break;
            }
            case Piece::WR:
            case Piece::BR: {
                scratch.clear();
                generate_rook_moves_bb(position, square, scratch);
                for (const Move& m : scratch) {
                    if (position.board[m.to()] != Piece::None) {
                        moves.add(m);
                    }
                }
                break;
            }
            case Piece::WQ:
            case Piece::BQ: {
                scratch.clear();
                generate_queen_moves_bb(position, square, scratch);
                for (const Move& m : scratch) {
                    if (position.board[m.to()] != Piece::None) {
                        moves.add(m);
                    }
                }
                break;
            }
            case Piece::WK:
            case Piece::BK: {
                scratch.clear();
                generate_king_moves_bb(position, square, scratch);
                for (const Move& m : scratch) {
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

void generate_knight_moves_bb(const Position& position, int square, MoveList& moves) {
    Bitboard attacks = knight_attacks[square];

    // Убираем клетки, занятые своими фигурами
    attacks &= ~position.by_color[static_cast<int>(position.side_to_move)];

    while (attacks != 0) {
        int to = pop_lsb(attacks);
        moves.add(Move(square, to));
    }
}

void generate_king_moves_bb(const Position& position, int square, MoveList& moves) {
    Bitboard attacks = king_attacks[square];

    // Убираем клетки, занятые своими фигурами
    attacks &= ~position.by_color[static_cast<int>(position.side_to_move)];

    while (attacks != 0) {
        int to = pop_lsb(attacks);
        moves.add(Move(square, to));
    }
}

void generate_rook_moves_bb(const Position& position, int square, MoveList& moves) {
    Bitboard occupancy = position.by_color[0] | position.by_color[1];

    Bitboard relevant_occupancy = occupancy & rook_masks[square];

    int bits = rook_relevant_bits[square];
    int magic_index = (relevant_occupancy * rook_magics[square]) >> (64 - bits);

    Bitboard attacks = rook_attacks_table[square][magic_index];
    attacks &= ~position.by_color[static_cast<int>(position.side_to_move)];

    while (attacks != 0) {
        int to = pop_lsb(attacks);
        moves.add(Move(square, to));
    }
}

void generate_bishop_moves_bb(const Position& position, int square, MoveList& moves) {
    Bitboard occupancy = position.by_color[0] | position.by_color[1];

    Bitboard relevant_occupancy = occupancy & bishop_masks[square];

    int bits = bishop_relevant_bits[square];
    int magic_index = (relevant_occupancy * bishop_magics[square]) >> (64 - bits);

    Bitboard attacks = bishop_attacks_table[square][magic_index];
    attacks &= ~position.by_color[static_cast<int>(position.side_to_move)];

    while (attacks != 0) {
        int to = pop_lsb(attacks);
        moves.add(Move(square, to));
    }
}

void generate_queen_moves_bb(const Position& position, int square, MoveList& moves) {
    generate_rook_moves_bb(position, square, moves);
    generate_bishop_moves_bb(position, square, moves);
}

void generate_pawn_moves_bb(const Position& position, int square, MoveList& moves) {
    Bitboard attacks = pawn_attacks[static_cast<int>(position.side_to_move)][square];
    Bitboard occupancy = position.by_color[0] | position.by_color[1];

    int rank = rank_of(square);
    int file = file_of(square);

    // Убираем клетки без вражеской фигуры
    attacks &= position.by_color[static_cast<int>(opposite_color(position.side_to_move))];

    if (position.en_passant_target != -1 &&
        (pawn_attacks[static_cast<int>(position.side_to_move)][square] &
         square_bb(position.en_passant_target))) {
        moves.add(Move(square, position.en_passant_target, MoveFlag::EnPassant));
    }

    int promotion_rank = (position.side_to_move == Color::White) ? 7 : 0;

    while (attacks != 0) {
        int to = pop_lsb(attacks);

        if (rank_of(to) == promotion_rank) {
            for (PromotionPiece p : promotion_pieces) {
                moves.add(Move(square, to, MoveFlag::Promotion, p));
            }
        } else {
            moves.add(Move(square, to));
        }
    }

    int direction = (position.side_to_move == Color::White) ? 1 : -1;
    int start_rank = (direction == 1) ? 1 : 6;

    int new_rank = rank + direction;
    int to = square_of(new_rank, file);
    bool blocked = occupancy & square_bb(to);

    if (new_rank == promotion_rank && !blocked) {
        for (PromotionPiece p : promotion_pieces) {
            moves.add(Move(square, to, MoveFlag::Promotion, p));
        }
    } else if (!blocked && is_valid_square(new_rank, file)) {
        moves.add(Move(square, to));
    }

    if (rank == start_rank && !blocked) {
        int double_pawn_move_rank = new_rank + direction;
        int double_pawn_move_to = square_of(double_pawn_move_rank, file);
        blocked = occupancy & square_bb(double_pawn_move_to);

        if (!blocked) {
            moves.add(Move(square, double_pawn_move_to));
        }
    }
}

bool is_square_attacked_bb(const Position& position, int square, Color by_color) {
    Bitboard occupancy = position.by_color[0] | position.by_color[1];

    if ((knight_attacks[square] & position.by_color[static_cast<int>(by_color)] &
         position.by_piece_type[static_cast<int>(PieceType::Knight)]) != 0) {
        return true;
    }

    if ((king_attacks[square] & position.by_color[static_cast<int>(by_color)] &
         position.by_piece_type[static_cast<int>(PieceType::King)]) != 0) {
        return true;
    }

    if ((pawn_attacks[static_cast<int>(opposite_color(by_color))][square] &
         position.by_color[static_cast<int>(by_color)] &
         position.by_piece_type[static_cast<int>(PieceType::Pawn)]) != 0) {
        return true;
    }

    Bitboard relevant = occupancy & rook_masks[square];
    int bits = rook_relevant_bits[square];
    int magic_index = (relevant * rook_magics[square]) >> (64 - bits);
    Bitboard rook_attacks_here = rook_attacks_table[square][magic_index];

    if ((rook_attacks_here & position.by_color[static_cast<int>(by_color)] &
         (position.by_piece_type[static_cast<int>(PieceType::Rook)] |
          position.by_piece_type[static_cast<int>(PieceType::Queen)])) != 0) {
        return true;
    }

    relevant = occupancy & bishop_masks[square];
    bits = bishop_relevant_bits[square];
    magic_index = (relevant * bishop_magics[square]) >> (64 - bits);
    Bitboard bishop_attacks_here = bishop_attacks_table[square][magic_index];

    if ((bishop_attacks_here & position.by_color[static_cast<int>(by_color)] &
         (position.by_piece_type[static_cast<int>(PieceType::Bishop)] |
          position.by_piece_type[static_cast<int>(PieceType::Queen)])) != 0) {
        return true;
    }

    return false;
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
Move* MoveList::end() { return moves.data() + count; }
void MoveList::clear() { count = 0; };
