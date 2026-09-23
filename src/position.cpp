#include "position.h"

#include <stdlib.h>

// Расстановка стартовой позиции
Position make_start_position() {
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

    for (int i = 8; i <= 15; i++) {
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

    for (int i = 48; i <= 55; i++) {
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
    };

    Piece moving_piece = position.board[move.from()];

    // Переставляем фигуры
    position.board[move.from()] = Piece::None;
    position.board[move.to()] = moving_piece;

    // Учитываем взятие на проходе
    if (move.flag() == MoveFlag::EnPassant) {
        position.board[square_of(rank_of(move.from()), file_of(move.to()))] = Piece::None;
    }

    // Учитываем превращение пешки
    if (move.flag() == MoveFlag::Promotion) {
        switch (move.promotion()) {
            case PromotionPiece::Bishop:
                if (position.side_to_move == Color::White) {
                    position.board[move.to()] = Piece::WB;
                } else if (position.side_to_move == Color::Black) {
                    position.board[move.to()] = Piece::BB;
                }
                break;
            case PromotionPiece::Knight:
                if (position.side_to_move == Color::White) {
                    position.board[move.to()] = Piece::WN;
                } else if (position.side_to_move == Color::Black) {
                    position.board[move.to()] = Piece::BN;
                }
                break;
            case PromotionPiece::Queen:
                if (position.side_to_move == Color::White) {
                    position.board[move.to()] = Piece::WQ;
                } else if (position.side_to_move == Color::Black) {
                    position.board[move.to()] = Piece::BQ;
                }
                break;
            case PromotionPiece::Rook:
                if (position.side_to_move == Color::White) {
                    position.board[move.to()] = Piece::WR;
                } else if (position.side_to_move == Color::Black) {
                    position.board[move.to()] = Piece::BR;
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
                position.board[static_cast<int>(Square::H1)] = Piece::None;
                position.board[static_cast<int>(Square::F1)] = Piece::WR;
                position.castling_rights &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
                break;
            case static_cast<int>(Square::C1):
                position.board[static_cast<int>(Square::A1)] = Piece::None;
                position.board[static_cast<int>(Square::D1)] = Piece::WR;
                position.castling_rights &= ~(WHITE_KINGSIDE | WHITE_QUEENSIDE);
                break;
            case static_cast<int>(Square::G8):
                position.board[static_cast<int>(Square::H8)] = Piece::None;
                position.board[static_cast<int>(Square::F8)] = Piece::BR;
                position.castling_rights &= ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
                break;
            case static_cast<int>(Square::C8):
                position.board[static_cast<int>(Square::A8)] = Piece::None;
                position.board[static_cast<int>(Square::D8)] = Piece::BR;
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
        position.castling_rights &= ~BLACK_KINGSIDE;
    }
    if (move.to() == static_cast<int>(Square::H8)) {
        position.castling_rights &= ~BLACK_QUEENSIDE;
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

    return undo_info;
}
