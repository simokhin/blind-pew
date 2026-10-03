#pragma once

#include <array>
#include <string>

/// Цвет стороны. `None` - у пустой клетки.
enum class Color { White, Black, None };

/// Возвращает противоположный цвет. Предусловие: `color != Color::None`
/// (для `None` вернёт `White`).
constexpr Color opposite_color(Color color) {
    return (color == Color::White) ? Color::Black : Color::White;
}

/// Тип фигуры без цвета. Порядок от пешки к королю соответствует возрастанию ценности и
/// используется как индекс в `Position::by_piece_type` и при переборе в `least_valuable_attacker`.
/// `None` использовать как индекс нельзя.
enum class PieceType { Pawn, Knight, Bishop, Rook, Queen, King, None };

/// Фигура на клетке. Порядок значений существенен: `color_of` считает белыми значения 1..6,
/// черными 7..12, а порядок типов внутри каждого цвета совпадает с `PieceType`.
enum class Piece { None, WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK };

/// Возвращает тип фигуры без учёта цвета; для `Piece::None` - `PieceType::None`.
constexpr PieceType piece_type_of(Piece piece) {
    if (piece == Piece::BP || piece == Piece::WP) {
        return PieceType::Pawn;
    } else if (piece == Piece::BN || piece == Piece::WN) {
        return PieceType::Knight;
    } else if (piece == Piece::BB || piece == Piece::WB) {
        return PieceType::Bishop;
    } else if (piece == Piece::BR || piece == Piece::WR) {
        return PieceType::Rook;
    } else if (piece == Piece::BQ || piece == Piece::WQ) {
        return PieceType::Queen;
    } else if (piece == Piece::BK || piece == Piece::WK) {
        return PieceType::King;
    } else {
        return PieceType::None;
    }
}

/// Возвращает цвет фигуры; для `Piece::None` - `Color::None`.
constexpr Color color_of(Piece piece) {
    int value = static_cast<int>(piece);

    if (value == 0) return Color::None;
    if (value <= 6) return Color::White;
    return Color::Black;
}

/// Клетка доски. Значения совпадают с индексами `Bitboard` и `Board`: A1 = 0 ... H8 = 63.
// clang-format off
enum class Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8
};
// clang-format on

/// Координаты клетки: `rank` - горизонталь 0..7 (0 - первая), `file` - вертикаль 0..7 (0 - A).
constexpr int rank_of(int square) { return square / 8; }
constexpr int file_of(int square) { return square % 8; }
constexpr int square_of(int rank, int file) { return rank * 8 + file; }

/// Проверяет, что клетка с координатами (`rank`, `file`) лежит на доске. Нужна при обходе соседних
/// клеток по смещениям, когда координаты могут выйти за 0..7.
constexpr bool is_valid_square(int rank, int file) {
    return rank >= 0 && rank <= 7 && file >= 0 && file <= 7;
}

/// Переводит запись клетки вида `e4` в индекс. Предусловие: ровно два символа от `a1` до `h8`;
/// проверка не выполняется.
int square_from_algebraic(const std::string& s);

/// Переводит индекс клетки в запись вида `e4`.
std::string algebraic_from_square(int square);

/// Доска как массив фигур, индексируемый номером клетки (A1 = 0 ... H8 = 63).
using Board = std::array<Piece, 64>;
