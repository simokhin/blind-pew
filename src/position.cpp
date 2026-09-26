#include "position.h"

#include <stdlib.h>

#include "zobrist.h"

UndoInfo make_move(Position& position, const Move& move) {
    // Проверяем, какая фигура была взята, учитывая случай взятия на проходе
    Piece captured_piece;
    if (move.flag() == MoveFlag::EnPassant) {
        captured_piece = position.board[square_of(rank_of(move.from()), file_of(move.to()))];
    } else {
        captured_piece = position.board[move.to()];
    }

    // Сохраняем состояние позиции до того, как был сделан ход
    UndoInfo undo_info = {
        .captured_piece = captured_piece,
        .castling_rights = position.castling_rights,
        .en_passant_target = position.en_passant_target,
        .halfmove_clock = position.halfmove_clock,
        .fullmove_number = position.fullmove_number,
        .zobrist_hash = position.zobrist_hash,
    };

    Piece moving_piece = position.board[move.from()];

    // Обновляем позицию короля
    if (moving_piece == Piece::WK) {
        position.white_king_square = move.to();
    } else if (moving_piece == Piece::BK) {
        position.black_king_square = move.to();
    }

    if (captured_piece != Piece::None && move.flag() != MoveFlag::EnPassant) {
        remove_piece(position, move.to());
    }
    move_piece(position, move.from(), move.to());

    // Учитываем взятие на проходе
    if (move.flag() == MoveFlag::EnPassant) {
        remove_piece(position, square_of(rank_of(move.from()), file_of(move.to())));
    }

    // Учитываем превращение пешки
    if (move.flag() == MoveFlag::Promotion) {
        switch (move.promotion()) {
            case PromotionPiece::Bishop:
                if (position.side_to_move == Color::White) {
                    remove_piece(position, move.to());
                    put_piece(position, Piece::WB, move.to());
                } else if (position.side_to_move == Color::Black) {
                    remove_piece(position, move.to());
                    put_piece(position, Piece::BB, move.to());
                }
                break;
            case PromotionPiece::Knight:
                if (position.side_to_move == Color::White) {
                    remove_piece(position, move.to());
                    put_piece(position, Piece::WN, move.to());
                } else if (position.side_to_move == Color::Black) {
                    remove_piece(position, move.to());
                    put_piece(position, Piece::BN, move.to());
                }
                break;
            case PromotionPiece::Queen:
                if (position.side_to_move == Color::White) {
                    remove_piece(position, move.to());
                    put_piece(position, Piece::WQ, move.to());
                } else if (position.side_to_move == Color::Black) {
                    remove_piece(position, move.to());
                    put_piece(position, Piece::BQ, move.to());
                }
                break;
            case PromotionPiece::Rook:
                if (position.side_to_move == Color::White) {
                    remove_piece(position, move.to());
                    put_piece(position, Piece::WR, move.to());
                } else if (position.side_to_move == Color::Black) {
                    remove_piece(position, move.to());
                    put_piece(position, Piece::BR, move.to());
                }
                break;
            default:
                break;
        }
    }

    // Рокировка
    if (move.flag() == MoveFlag::Castling) {
        switch (move.to()) {
            case static_cast<int>(Square::G1):
                move_piece(position, static_cast<int>(Square::H1), static_cast<int>(Square::F1));
                position.castling_rights &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
                break;
            case static_cast<int>(Square::C1):
                move_piece(position, static_cast<int>(Square::A1), static_cast<int>(Square::D1));
                position.castling_rights &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
                break;
            case static_cast<int>(Square::G8):
                move_piece(position, static_cast<int>(Square::H8), static_cast<int>(Square::F8));
                position.castling_rights &= ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
                break;
            case static_cast<int>(Square::C8):
                move_piece(position, static_cast<int>(Square::A8), static_cast<int>(Square::D8));
                position.castling_rights &= ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
                break;
            default:
                break;
        }
    }

    // Снятие прав рокировки
    if (moving_piece == Piece::WK) {
        position.castling_rights &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
    }
    if (moving_piece == Piece::BK) {
        position.castling_rights &= ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
    }
    if (move.from() == static_cast<int>(Square::A1)) {
        position.castling_rights &= ~WHITE_QUEENSIDE;
    }
    if (move.from() == static_cast<int>(Square::H1)) {
        position.castling_rights &= ~WHITE_KINGSIDE;
    }
    if (move.from() == static_cast<int>(Square::A8)) {
        position.castling_rights &= ~BLACK_QUEENSIDE;
    }
    if (move.from() == static_cast<int>(Square::H8)) {
        position.castling_rights &= ~BLACK_KINGSIDE;
    }
    if (move.to() == static_cast<int>(Square::A1)) {
        position.castling_rights &= ~WHITE_QUEENSIDE;
    }
    if (move.to() == static_cast<int>(Square::H1)) {
        position.castling_rights &= ~WHITE_KINGSIDE;
    }
    if (move.to() == static_cast<int>(Square::A8)) {
        position.castling_rights &= ~BLACK_QUEENSIDE;
    }
    if (move.to() == static_cast<int>(Square::H8)) {
        position.castling_rights &= ~BLACK_KINGSIDE;
    }

    // По умолчанию сбрасываем ход en passant
    position.en_passant_target = -1;

    // Установка клетки взятия на проходе, если доступна
    if (moving_piece == Piece::WP || moving_piece == Piece::BP) {
        if (abs((rank_of(move.to()) - rank_of(move.from()))) == 2) {
            int en_passant_target =
                square_of((rank_of(move.from()) + rank_of(move.to())) / 2, file_of(move.from()));
            position.en_passant_target = en_passant_target;
        }
    }

    // Обновление счетчика полуходов
    if (moving_piece == Piece::WP || moving_piece == Piece::BP || captured_piece != Piece::None) {
        position.halfmove_clock = 0;
    } else {
        position.halfmove_clock += 1;
    }

    // Обновление счетчика ходов
    if (position.side_to_move == Color::Black) {
        position.fullmove_number += 1;
    }

    // Переключение стороны, которая ходит
    position.side_to_move = (position.side_to_move == Color::White) ? Color::Black : Color::White;

    // Обновляем хэш
    position.zobrist_hash ^= side_to_move_key;

    position.zobrist_hash ^=
        castling_keys[undo_info.castling_rights] ^ castling_keys[position.castling_rights];

    if (undo_info.en_passant_target != -1) {
        position.zobrist_hash ^= en_passant_file_keys[file_of(undo_info.en_passant_target)];
    }
    if (position.en_passant_target != -1) {
        position.zobrist_hash ^= en_passant_file_keys[file_of(position.en_passant_target)];
    }

    return undo_info;
}

void unmake_move(Position& position, const Move& move, const UndoInfo& undo) {
    position.castling_rights = undo.castling_rights;
    position.en_passant_target = undo.en_passant_target;
    position.halfmove_clock = undo.halfmove_clock;
    position.fullmove_number = undo.fullmove_number;

    position.side_to_move = (position.side_to_move == Color::White) ? Color::Black : Color::White;

    move_piece(position, move.to(), move.from());
    if (undo.captured_piece != Piece::None && move.flag() != MoveFlag::EnPassant) {
        put_piece(position, undo.captured_piece, move.to());
    }

    if (position.board[move.from()] == Piece::WK) {
        position.white_king_square = move.from();
    } else if (position.board[move.from()] == Piece::BK) {
        position.black_king_square = move.from();
    }

    if (move.flag() == MoveFlag::EnPassant) {
        put_piece(position, undo.captured_piece,
                  square_of(rank_of(move.from()), file_of(move.to())));
    }

    if (move.flag() == MoveFlag::Promotion) {
        remove_piece(position, move.from());
        put_piece(position, (position.side_to_move == Color::White) ? Piece::WP : Piece::BP,
                  move.from());
    }

    if (move.flag() == MoveFlag::Castling) {
        switch (move.to()) {
            case static_cast<int>(Square::G1):
                move_piece(position, static_cast<int>(Square::F1), static_cast<int>(Square::H1));
                break;
            case static_cast<int>(Square::C1):
                move_piece(position, static_cast<int>(Square::D1), static_cast<int>(Square::A1));
                break;
            case static_cast<int>(Square::G8):
                move_piece(position, static_cast<int>(Square::F8), static_cast<int>(Square::H8));
                break;
            case static_cast<int>(Square::C8):
                move_piece(position, static_cast<int>(Square::D8), static_cast<int>(Square::A8));
                break;
            default:
                break;
        }
    }

    position.zobrist_hash = undo.zobrist_hash;
}

int king_square_of(const Position& position, Color color) {
    return color == Color::White ? position.white_king_square : position.black_king_square;
}

void put_piece(Position& position, Piece piece, int square) {
    position.board[square] = piece;

    // Устанавливаем биты
    position.by_color[static_cast<int>(color_of(piece))] |= square_bb(square);
    position.by_piece_type[static_cast<int>(piece_type_of(piece))] |= square_bb(square);

    position.zobrist_hash ^= piece_square_keys[static_cast<int>(piece)][square];
}

void remove_piece(Position& position, int square) {
    Piece piece = position.board[square];
    position.zobrist_hash ^= piece_square_keys[static_cast<int>(piece)][square];
    position.board[square] = Piece::None;

    // Снимаем биты
    position.by_color[static_cast<int>(color_of(piece))] &= ~square_bb(square);
    position.by_piece_type[static_cast<int>(piece_type_of(piece))] &= ~square_bb(square);
}

void move_piece(Position& position, int from, int to) {
    Piece piece = position.board[from];
    position.zobrist_hash ^= piece_square_keys[static_cast<int>(piece)][from] ^
                             piece_square_keys[static_cast<int>(piece)][to];

    position.board[to] = piece;
    position.board[from] = Piece::None;

    // Снимаем и устанавливаем биты
    position.by_color[static_cast<int>(color_of(piece))] ^= square_bb(from) | square_bb(to);
    position.by_piece_type[static_cast<int>(piece_type_of(piece))] ^=
        square_bb(from) | square_bb(to);
}

int make_null_move(Position& position) {
    // Сохраняем en passant поле
    int old_en_passant_target = position.en_passant_target;

    // Если оно было установлено, вычитыем его из хэша
    if (old_en_passant_target != -1) {
        position.zobrist_hash ^= en_passant_file_keys[file_of(old_en_passant_target)];
    }

    // Обнуляем поле
    position.en_passant_target = -1;

    // Вычитаем из хэша сторону
    position.side_to_move = opposite_color(position.side_to_move);
    position.zobrist_hash ^= side_to_move_key;

    return old_en_passant_target;
}

void unmake_null_move(Position& position, int old_en_passant_target) {
    // Возвращаем параметры
    position.en_passant_target = old_en_passant_target;
    position.side_to_move = opposite_color(position.side_to_move);
    position.zobrist_hash ^= side_to_move_key;

    // Возвращаем en passant поле в hash, если оно было установлен
    if (old_en_passant_target != -1) {
        position.zobrist_hash ^= en_passant_file_keys[file_of(old_en_passant_target)];
    }
}
